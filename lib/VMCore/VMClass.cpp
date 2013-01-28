//
//  VMClass.cpp
//  silk
//
//  Created by Haohui Mai on 12/2/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#include "VMClass.h"
#include "VMMember.h"
#include "CompilationEngine.h"

#include "silk/Support/Util.h"

#include <llvm/Type.h>
#include <llvm/Module.h>

#include <iostream>

namespace silk
{
    using namespace llvm;
    using namespace decil;
    
    static bool IsInstanceFieldForClass(const VMField *f)
    {
        return !f->is_static();
    }
    
    static bool IsStaticFieldForClass(const VMField *f)
    {
        return f->is_static() && !f->is_literal();
    }
    
    VMClass::VMClass(CompilationEngine *engine)
    : state_(State::kUninitialized)
    , engine_(engine)
    , physical_type_(nullptr)
    , normal_type_(nullptr)
    , boxed_type_(nullptr)
    , static_instance_(nullptr)
    {}
    
    VMClass::~VMClass()
    {}
    
    INamedTypeDefinition::TypeCode VMClass::type_code() const
    {
        return INamedTypeDefinition::TypeCode::NotPrimitive;
    }
    
    void VMClass::LayoutIfNecessary()
    {
        if (state_ == State::kUninitialized)
            Layout();
    }

    VMField *VMClass::GetField(const std::u16string &name) const
    {
        auto it = fields_.find(name);
        return it == fields_.end() ? nullptr : it->second;
    }

    VMMethod *VMClass::GetMethod(const std::u16string &name) const
    {
        auto it = methods_.find(name);
        return it == methods_.end() ? nullptr : it->second;
    }
    
    VMClassPointer::VMClassPointer(CompilationEngine *engine, VMClass *target_type)
    : VMClass(engine)
    , target_type_(target_type)
    {
        name_ = target_type->name() + u"*";

        auto &c = engine_->module()->getContext();
        is_void_star_type_ = target_type->normal_type() == Type::getVoidTy(c);

        Type *t = is_void_star_type_ ? Type::getInt8PtrTy(c) : PointerType::getUnqual(target_type->normal_type());
        physical_type_ = normal_type_ = t;
    }
    
    void VMClassPointer::Layout()
    {
        state_ = State::kInitialized;
    }

    
    VMClassVector::VMClassVector(CompilationEngine *engine, VMClass *element_type)
    : VMClass(engine)
    , element_type_(element_type)
    {
        name_ = element_type_->name() + u"[]";
    }

    void VMClassVector::Layout()
    {
        auto base_type = engine_->host()->platform_type()->system_array()->resolved_type();
        Type * types[] =
        {
            engine_->GetVMClassForNamedType(base_type)->physical_type(),
            PointerType::getUnqual(element_type_->normal_type()),
        };
        
        auto &c = engine_->module()->getContext();
        auto ty = StructType::create(c, types);
        
        ty->setName(ToUTF8String(name_));
        
        physical_type_ = ty;
        normal_type_ = PointerType::getUnqual(ty);

        state_ = State::kInitialized;
    }

    VMNamedClassBase::VMNamedClassBase(CompilationEngine *engine, INamedTypeDefinition *type_def)
    : VMClass(engine)
    , type_def_(type_def)
    {
        name_ = type_def->name();
    }
    
    INamedTypeDefinition::TypeCode VMNamedClassBase::type_code() const
    {
        return type_def_->type_code();
    }

    
    void VMNamedClassBase::LoadVMFields()
    {
        unsigned field_off = IncludeBaseClass() ? 1 : 0;
        unsigned static_field_off = 0;
        
        for (auto it = type_def_->field_begin(), end = type_def_->field_end(); it != end; ++it)
        {
            auto f = *it;
            if (f->is_literal())
                continue;

            auto ty = engine_->GetVMClassForNamedType((*it)->field_type()->resolved_type());
            
            VMField *vm_field = nullptr;
            if (f->is_static())
            {
                vm_field = new VMField(f, ty, static_field_off++);
            }
            else
            {
                vm_field = new VMField(*it, ty, field_off++);
            }
            fields_.insert(std::make_pair((*it)->name(), vm_field));
        }
    }
    
    void VMNamedClassBase::LoadVMMethods()
    {
        for (auto it = type_def_->method_begin(), end = type_def_->method_end(); it != end; ++it)
        {
            auto method = *it;
            auto vm_method = new VMMethod(method);

            auto has_implicit_this = method->has_this() && !method->explicit_this();
            auto return_ty = engine_->GetVMClassForNamedType(method->return_type()->resolved_type());
            
            vm_method->has_implicit_this_ = has_implicit_this;
            vm_method->return_type_ = return_ty;

            std::vector<Type*> llvm_params;
            for (auto it2 = method->param_begin(), end2 = method->param_end(); it2 != end2; ++it2)
            {
                auto p = *it2;
                VMClass *vm_type = nullptr;
                
                //
                // Param 0 is the this argument
                // For valuetype, the this parameter should be a managed pointer
                //
                if (has_implicit_this && IsValueType() && it2 == method->param_begin())
                {
                    vm_type = engine_->GetPointerType(this);
                }
                else
                {
                    vm_type = engine_->GetVMClassForNamedType(p->type()->resolved_type());
                }
                
                ParamInfo param_info(p->name(), vm_type, p);
                vm_method->params_.push_back(param_info);
                llvm_params.push_back(param_info.type()->normal_type());
            }
            
            auto func_ty = FunctionType::get(vm_method->return_type()->normal_type(), llvm_params, false);
            
            std::string func_name;
            if (method->is_pinvoke())
            {
                func_name = ToUTF8String(method->name());
            }
            else
            {
                func_name = ToUTF8String(name() + u"." +u"." + vm_method->mangled_name());
            }
            
            vm_method->implementation_ = Function::Create(func_ty, GlobalValue::ExternalLinkage,
                                                          func_name, engine_->module());

            methods_.insert(std::make_pair(vm_method->mangled_name(), vm_method));
        }
    }
    
    void VMNamedClassBase::LoadStaticFields()
    {
        auto &c = engine_->module()->getContext();
        auto static_ty = StructType::create(c);
        RefineLLVMType(static_ty, false, IsStaticFieldForClass);
        
        if (static_ty->getNumElements())
        {
            static_instance_ = new GlobalVariable(*engine_->module(), static_ty, false,
                                                  GlobalValue::InternalLinkage,
                                                  Constant::getNullValue(static_ty),
                                                  ToUTF8String(u"static_" + name_));
        }
    }
    
    void VMNamedClassBase::RefineLLVMType(StructType *type, bool include_base_class, std::function<bool(const VMField*)> filter)
    {
        std::vector<VMField*> fields;
        for (auto p : fields_)
        {
            if (filter(p.second))
                fields.push_back(p.second);
        }
        
        std::sort(fields.begin(), fields.end(),
                  [](VMField *lhs, VMField *rhs) { return lhs->offset() < rhs->offset(); });
        
        std::vector<Type*> fields_type;

        if (include_base_class)
        {
            auto base_class = type_def_->base_class();
            auto base_type = base_class->resolved_type();
            fields_type.push_back(engine_->GetVMClassForNamedType(base_type)->physical_type());
        }

        std::transform(fields.begin(), fields.end(), std::back_inserter(fields_type),
                       [](VMField *f) { return f->type()->normal_type(); });
        
        type->setBody(fields_type);
    }
    
    bool VMNamedClassBase::IsValueType() const
    {
        auto base_class = type_def_->base_class();
        bool is_valuetype = base_class;
        if (is_valuetype)
        {
            auto &name = base_class->resolved_type()->name();
            is_valuetype = name == u"System.ValueType" || name == u"System.Enum";
        }
        return is_valuetype;
    }
    
    bool VMNamedClassBase::IncludeBaseClass() const
    {
        auto base_class = type_def_->base_class();
        bool ret = base_class;
        if (base_class)
        {
            auto &name = base_class->resolved_type()->name();
            ret = !(name == u"System.ValueType" || name == u"System.Enum");
        }
        return ret;
    }
    
    VMNamedClass::VMNamedClass(CompilationEngine *engine, INamedTypeDefinition *type_def)
    : VMNamedClassBase(engine, type_def)
    {}
    
    void VMNamedClass::Layout()
    {
        // Create type holders to handle recursive data structures
        auto &c = engine_->module()->getContext();

        physical_type_ = StructType::create(c, ToUTF8String(name_));
        normal_type_ = IsValueType() ? physical_type_ : PointerType::getUnqual(physical_type_);
        boxed_type_ = IsValueType() ? physical_type_ : nullptr;
        
        LoadVMFields();
        RefineLLVMType(cast<StructType>(physical_type_), IncludeBaseClass(), IsInstanceFieldForClass);
        LoadStaticFields();
        
        LoadVMMethods();
        state_ = State::kInitialized;
    }
    
    VMClassEnum::VMClassEnum(CompilationEngine *engine, INamedTypeDefinition *type_def)
    : VMNamedClassBase(engine, type_def)
    {}

    void VMClassEnum::Layout()
    {
        auto &c = engine_->module()->getContext();
        auto int_ty = Type::getInt32Ty(c);
        physical_type_ = normal_type_ = boxed_type_ = int_ty;
        LoadVMMethods();
        state_ = State::kInitialized;
    }
    
    VMPrimitiveClass::VMPrimitiveClass(CompilationEngine *engine, INamedTypeDefinition *type_def)
    : VMNamedClassBase(engine, type_def)
    {}
    
    VMPrimitiveClass *VMPrimitiveClass::CreateVMPrimitiveClass(CompilationEngine *engine, INamedTypeDefinition *type_def)
    {
        using decil::INamedTypeDefinition;
        
        struct TypeMap
        {
            INamedTypeDefinition::TypeCode tc_;
            Type *physical_type_;
            TypeMap(INamedTypeDefinition::TypeCode tc, Type *physical_type)
            : tc_(tc)
            , physical_type_(physical_type)
            {}
        };
        
        static std::vector<TypeMap> maps;
        
        if (!maps.size())
        {
            auto &c = engine->module()->getContext();
            // Initialize the type map
            // Notice that we don't handle the string type here. Defer it until the layout stage.
            maps.push_back(TypeMap(INamedTypeDefinition::TypeCode::Void, Type::getVoidTy(c)));
            maps.push_back(TypeMap(INamedTypeDefinition::TypeCode::Boolean, Type::getInt1Ty(c)));
            maps.push_back(TypeMap(INamedTypeDefinition::TypeCode::Char, Type::getInt16Ty(c)));
            maps.push_back(TypeMap(INamedTypeDefinition::TypeCode::Int8, Type::getInt8Ty(c)));
            maps.push_back(TypeMap(INamedTypeDefinition::TypeCode::UInt8, Type::getInt8Ty(c)));
            maps.push_back(TypeMap(INamedTypeDefinition::TypeCode::Int16, Type::getInt16Ty(c)));
            maps.push_back(TypeMap(INamedTypeDefinition::TypeCode::UInt16, Type::getInt16Ty(c)));
            maps.push_back(TypeMap(INamedTypeDefinition::TypeCode::Int32, Type::getInt32Ty(c)));
            maps.push_back(TypeMap(INamedTypeDefinition::TypeCode::UInt32, Type::getInt32Ty(c)));
            maps.push_back(TypeMap(INamedTypeDefinition::TypeCode::Int64, Type::getInt64Ty(c)));
            maps.push_back(TypeMap(INamedTypeDefinition::TypeCode::UInt64, Type::getInt64Ty(c)));
            maps.push_back(TypeMap(INamedTypeDefinition::TypeCode::Single, Type::getFloatTy(c)));
            maps.push_back(TypeMap(INamedTypeDefinition::TypeCode::Double, Type::getDoubleTy(c)));
            maps.push_back(TypeMap(INamedTypeDefinition::TypeCode::IntPtr, Type::getInt8PtrTy(c)));
            maps.push_back(TypeMap(INamedTypeDefinition::TypeCode::UIntPtr, Type::getInt8PtrTy(c)));
        }
        
        auto r = new VMPrimitiveClass(engine, type_def);
        for (auto & e : maps)
        {
            if (e.tc_ == type_def->type_code())
            {
                r->physical_type_ = e.physical_type_;
                r->normal_type_ = e.physical_type_;
                break;
            }
        }
                
        assert (type_def->type_code() == INamedTypeDefinition::TypeCode::String || r->normal_type_);
        return r;
    }
    
    void VMPrimitiveClass::Layout()
    {
        auto &c = engine_->module()->getContext();
        if (type_def_->type_code() == INamedTypeDefinition::TypeCode::String)
        {
            physical_type_ = StructType::create(c, ToUTF8String(name_));
            normal_type_ = PointerType::getUnqual(physical_type_);
            
            LoadVMFields();
            RefineLLVMType(cast<StructType>(physical_type_), IncludeBaseClass(), IsInstanceFieldForClass);
        }
        else
        {
            LoadVMFields();
            boxed_type_ = StructType::create(c, ToUTF8String(name_));
            RefineLLVMType(cast<StructType>(boxed_type_), IncludeBaseClass(), IsInstanceFieldForClass);
        }
        
        LoadStaticFields();

        LoadVMMethods();
        state_ = State::kInitialized;
    }
}

