//
//  CompilationEngine.cpp
//  silk
//
//  Created by Haohui Mai on 12/2/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#include "CompilationEngine.h"
#include "VMClass.h"
#include "VMMember.h"
#include "OpcodeCompiler.h"

#include "silk/VMCore/VMModel.h"
#include "silk/decil/ObjectModel.h"
#include "silk/Support/Util.h"

#include <llvm/LLVMContext.h>
#include <llvm/Module.h>

#include <iostream>

namespace silk
{
    using namespace llvm;
    using namespace decil;
    
    ICompilationEngine::~ICompilationEngine()
    {}
    
    ICompilationEngine *CreateCompilationEngine(IHost *host, const std::string &triple)
    {
        return new CompilationEngine(host, triple);
    }
    
    CompilationEngine::CompilationEngine(IHost *host, const std::string &triple)
    : host_(host)
    , module_(new Module("", getGlobalContext()))
    , intrinsic_(nullptr)
    {
        module_->setTargetTriple(triple);
        
        if (triple == "armv7-elf-linux")
        {
            module_->setDataLayout("e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:32:64-v128:32:128-a0:0:32-n32-S32");
        }
        else if (triple == "x86_64-apple-macosx10.7.3")
        {
            module_->setDataLayout("e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64");
            
        }
        else if (triple == "i386-apple-macosx10.8.0")
        {
            module_->setDataLayout("e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:128:128-n8:16:32-S128");
        }
        else if (triple == "i386-unknown-linux-gnu")
        {
            module_->setDataLayout("e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S128");
        }
    }
    
    void CompilationEngine::Compile()
    {
        // The set of loaded assemblies might change during resolution
        // Thus here it gets the size every time.
        for (size_t i = 0; i < host_->assembly_size(); ++i)
        {
            auto assembly = host_->get_assembly(i);
            Layout(assembly);
        }
        
        for (size_t i = 0; i < host_->assembly_size(); ++i)
        {
            auto assembly = host_->get_assembly(i);
            GenerateCode(assembly);
        }
    }
    
    void CompilationEngine::Layout(IAssembly *assembly)
    {
        for (auto it = assembly->all_types_begin(), end = assembly->all_types_end(); it != end; ++it)
        {
            GetVMClassForNamedType(*it);
        }
    }
    
    void CompilationEngine::GenerateCode(IAssembly *assembly)
    {
        for (auto it = assembly->all_types_begin(), end = assembly->all_types_end(); it != end; ++it)
        {
            auto vm_class = GetVMClassForNamedType(*it);
            for (auto it2 = vm_class->method_begin(), end2 = vm_class->method_end(); it2 != end2; ++it2)
            {
                auto m = it2->second;
                if (m->method_def()->inst_begin() == m->method_def()->inst_end())
                    continue;
                
                OpcodeCompiler compiler(this, m);
                compiler.Compile();
            }
        }
    }
    
    VMClass *CompilationEngine::GetVMClassForNamedType(ITypeDefinition *def)
    {
        auto it = vm_types_.find(def);
        if (it != vm_types_.end())
        {
            return it->second;
        }
        
        assert (def);
        VMClass *ret = nullptr;
        
        if (auto named_type = dynamic_cast<INamedTypeDefinition*>(def))
        {
            auto tc = named_type->type_code();
            if (named_type->base_class() &&
                named_type->base_class()->resolved_type() == host_->platform_type()->system_enum()->resolved_type())
            {
                ret = new VMClassEnum(this, named_type);
            }
            else if (tc == INamedTypeDefinition::TypeCode::NotPrimitive)
            {
                ret = new VMNamedClass(this, named_type);
            }
            else
            {
                ret = VMPrimitiveClass::CreateVMPrimitiveClass(this, named_type);
            }
        }
        else if (auto pointer_type = dynamic_cast<IPointerType*>(def))
        {
            auto target_type = pointer_type->target_type()->resolved_type();
            assert (target_type);
            
            auto it = pointer_type_cache_.find(target_type);
            if (it == pointer_type_cache_.end())
            {
                auto vm_target_type = GetVMClassForNamedType(target_type);
                ret = GetPointerType(vm_target_type);
                pointer_type_cache_.insert(std::make_pair(target_type, ret));
            }
            else
            {
                ret = it->second;
            }
        }
        else if (auto vector_type = dynamic_cast<IVectorType*>(def))
        {
            auto element_type = vector_type->element_type()->resolved_type();
            assert (element_type);
            
            auto it = vector_type_cache_.find(element_type);
            
            if (it == vector_type_cache_.end())
            {
                auto vm_element_type = GetVMClassForNamedType(element_type);
                assert (vm_element_type->normal_type());
                ret = new VMClassVector(this, vm_element_type);
                vector_type_cache_.insert(std::make_pair(element_type, ret));
            }
            else
            {
                ret = it->second;
            }
        }
        else
        {
            assert (0 && "Unimplemented");
        }
        
        vm_types_.insert(std::make_pair(def, ret));

        ret->LayoutIfNecessary();
        
        assert(ret->normal_type());
        return ret;
    }
    
    VMClass *CompilationEngine::GetPointerType(VMClass *target_type)
    {
        auto it = vm_pointer_type_cache_.find(target_type);
        if (it != vm_pointer_type_cache_.end())
            return it->second;
        
        auto result_type = new VMClassPointer(this, target_type);
        vm_pointer_type_cache_.insert(std::make_pair(target_type, result_type));
        return result_type;
    }
    
    Value *CompilationEngine::GetOrCreateString(const std::u16string &str)
    {
        auto it = string_cache_.find(str);
        if (it != string_cache_.end())
            return it->second;
        
        auto vm_str_type = GetVMClassForNamedType(host_->platform_type()->system_string()->resolved_type());
        auto str_ty = cast<StructType>(vm_str_type->physical_type());

        LLVMContext & c = module_->getContext();
        Type * i16_ty = Type::getInt16Ty(c);
        Type * i32_ty = Type::getInt32Ty(c);
        ArrayType * payload_ty = ArrayType::get(i16_ty, str.length());
        
        SmallVector<Type *, 4> elements(str_ty->element_begin(), str_ty->element_end());
        elements.back() = payload_ty;
        StructType * const_str_ty = StructType::create(elements);
        
        SmallVector<Constant *, 4> init_struct;
        init_struct.push_back(Constant::getNullValue(elements[0]));  // Object struct
        init_struct.push_back(ConstantInt::get(i32_ty, str.length()));    // length
        auto payload = ArrayRef<uint16_t>(reinterpret_cast<const uint16_t*>(str.c_str()), str.length());
        init_struct.push_back(ConstantDataArray::get(c, payload));
        
        auto initializer = ConstantStruct::get(const_str_ty, init_struct);
        
        auto v = new GlobalVariable(*module_, const_str_ty, true, GlobalValue::InternalLinkage, initializer, ".str");
        string_cache_.insert(std::make_pair(str, v));
        return v;
    }
    
    INamedTypeDefinition::TypeCode CompilationEngine::NativeIntTypeCode() const
    {
        return INamedTypeDefinition::TypeCode::Int32;
    }
    
    INamedTypeDefinition::TypeCode CompilationEngine::NativeUIntTypeCode() const
    {
        return INamedTypeDefinition::TypeCode::UInt32;
    }
}
