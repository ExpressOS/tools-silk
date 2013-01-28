//
//  BinaryObjectModel.cpp
//  silk
//
//  Created by Haohui Mai on 11/26/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#include "BinaryObjectModel.h"
#include "PEFileToObjectModel.h"
#include "SignatureConverter.h"
#include "ILReader.h"

#include "silk/decil/IHost.h"
#include "silk/Support/Util.h"

#include <iostream>
#include <algorithm>

namespace silk
{
    namespace decil
    {
        MetadataObjectModelBase::~MetadataObjectModelBase()
        {}
        
        DefinitionBase::DefinitionBase(PEFileToObjectModel *model)
        : model_(model)
        {}
        
        TypeBase::TypeBase(PEFileToObjectModel *model, const TypeDefinition *type_def,
                           const std::u16string &mangled_name)
        : DefinitionBase(model)
        , type_def_(type_def)
        , mangled_name_(mangled_name)
        , base_class_(nullptr)
        , packing_size_(0)
        , class_size_(0)
        {
            model->GetClassLayout(type_def, &packing_size_, &class_size_);
        }
        
        ITypeReference *TypeBase::base_class() const
        {
            if (base_class_)
                return base_class_;
            
            base_class_ = model_->GetTypeReferenceForToken(&type_def_->Extends);
            return base_class_;
        }
        
        Assembly::Assembly(PEFileToObjectModel *model, const AssemblyIdentity &id)
        : DefinitionBase(model)
        , id_(id)
        {}
        
        std::vector<INamedTypeDefinition*>::const_iterator Assembly::all_types_begin() const
        {
            return ++(model_->named_typedefs().begin());
        }
        
        std::vector<INamedTypeDefinition*>::const_iterator Assembly::all_types_end() const
        {
            return model_->named_typedefs().end();
        }
        
        AssemblyReference::AssemblyReference(IHost *host, const AssemblyIdentity &id)
        : host_(host)
        , id_(id)
        {
        }
        
        IAssembly *AssemblyReference::ResolvedAssembly()
        {
            return host_->LoadAssembly(id_);
        }
        
        NonGenericNamespaceType::NonGenericNamespaceType(PEFileToObjectModel *model, const TypeDefinition *e)
        : TypeBase(model, e, (std::u16string)e->TypeNamespace + u"." + (std::u16string)e->TypeName)
        {}
        
        NonGenericNamespaceTypeWithPrimitiveType::NonGenericNamespaceTypeWithPrimitiveType(PEFileToObjectModel *model, const TypeDefinition *e, INamedTypeDefinition::TypeCode type_code)
        : NonGenericNamespaceType(model, e)
        , type_code_(type_code)
        {}
                
        NonGenericNestedType::NonGenericNestedType(PEFileToObjectModel *model,
                                                   const TypeDefinition *e,
                                                   unsigned owning_type_idx)
        : TypeBase(model, e, u"")
        , owning_type_(nullptr)
        , owning_type_idx_(owning_type_idx)
        {}
        
        ITypeDefinition *NonGenericNestedType::owning_type()
        {
            if (owning_type_)
                return owning_type_;
            
            owning_type_ = model_->GetTypeDefinitionAtRow(owning_type_idx_);
            return owning_type_;
        }

        const std::u16string &NonGenericNestedType::name()
        {
            if (!mangled_name_.empty())
                return mangled_name_;
            
            mangled_name_ = owning_type()->name()
            + u"."
            + (std::u16string)type_def_->TypeNamespace + u"." + (std::u16string)type_def_->TypeName;
            return mangled_name_;
        }
        
        TypeReference::TypeReference(IAssemblyReference *assembly_ref, const std::u16string &name, const std::u16string &namespace_name)
        : type_(Scope::kAssemblyRef)
        , resolved_type_(nullptr)
        , assembly_ref_(assembly_ref)
        , name_(name)
        , namespace_name_(namespace_name)
        {}
        
        ITypeDefinition *TypeReference::resolved_type()
        {
            if (resolved_type_)
                return resolved_type_;
            
            ITypeDefinition *ret = nullptr;
            
            if (type_ == kAssemblyRef)
            {
                auto assembly = static_cast<Assembly*>(assembly_ref_->ResolvedAssembly());
                assert(assembly);

                ret = assembly->model()->ResolveNamespaceTypeDefinition(namespace_name_, name_);
                assert(ret);
            }
            else
            {
                assert (0 && "Unimplemented");
            }
            resolved_type_ = ret;
            return ret;
        }
        
        PointerType::PointerType(ITypeReference *target_type)
        : target_type_(target_type)
        {}
        
        const std::u16string &PointerType::name()
        {
            if (name_.empty())
                name_ = target_type_->resolved_type()->name() + u"*";
            
            return name_;
        }
        
        VectorType::VectorType(ITypeReference *element_type)
        : element_type_(element_type)
        {}
        
        const std::u16string &VectorType::name()
        {
            if (name_.empty())
                name_ = element_type_->resolved_type()->name() + u"[]";
            
            return name_;
        }
        
        FieldReference::FieldReference(PEFileToObjectModel *model, const MemberReference *member_ref,
                                       ITypeReference *parent_ref)
        : model_(model)
        , name_((std::u16string)member_ref->Name)
        , parent_ref_(parent_ref)
        , resolved_def_(nullptr)
        , signauture_(member_ref->Signature.to_istream())
        {}

        IFieldDefinition *FieldReference::resolved_definition()
        {
            if (resolved_def_)
                return resolved_def_;
            
            auto type = parent_ref_->resolved_type();
            for (auto it = type->field_begin(), end = type->field_end(); it != end; ++it)
            {
                auto f = *it;
                if (f->name() == name_)
                {
                    resolved_def_ = f;
                    break;
                }
            }
            
            return resolved_def_;
        }

        MethodReference::MethodReference(PEFileToObjectModel *model, const MemberReference *member_ref,
                                       ITypeReference *parent_ref)
        : model_(model)
        , name_((std::u16string)member_ref->Name)
        , parent_ref_(parent_ref)
        , resolved_def_(nullptr)
        , signauture_(member_ref->Signature.to_istream())
        {}
        
        IMethodDefinition *MethodReference::resolved_definition()
        {
            if (resolved_def_)
                return resolved_def_;
            
            auto type = parent_ref_->resolved_type();
            MethodSignatureConverter converter(model_, signauture_);
            auto params_name = PEFileToObjectModel::MangleParams(converter.param_type());
            auto has_implicit_this = converter.has_this() && !converter.explicit_this();
            
            for (auto it = type->method_begin(), end = type->method_end(); it != end; ++it)
            {
                auto m = *it;
                if (m->name() != name_)
                    continue;
                
                // XXX: cache the result to speed up lookups
                std::vector<ITypeReference*> param_type;
                // skip the this argument when mangling the name
                auto param_start = m->param_begin();
                if (has_implicit_this)
                    ++param_start;

                std::transform(param_start, m->param_end(), std::back_inserter(param_type),
                               [](IParameterDefinition *p) { return p->type(); });
                
                auto method_signature_name = PEFileToObjectModel::MangleParams(param_type);
                if (params_name == method_signature_name)
                {
                    resolved_def_ = m;
                    break;
                }
            }
            return resolved_def_;
        }

        
        FieldDefinition::FieldDefinition(PEFileToObjectModel *model, const FieldDef *def,
                                         INamedTypeDefinition *containing_type)
        : DefinitionBase(model)
        , name_(def->Name.string)
        , containing_type_(containing_type)
        , flags_(def->Flags)
        , type_(nullptr)
        , signauture_(def->Signature.to_istream())
        {
            if (has_field_rva())
                mapping_ = model->GetFieldMapping(def);
        }
        
        const std::u16string &FieldDefinition::name()
        {
            return name_;
        }
        
        bool FieldDefinition::is_static() const
        {
            return flags_ & FieldDef::kStatic;
        }
        
        bool FieldDefinition::is_literal() const
        {
            return flags_ & FieldDef::kLiteral;
        }
        
        bool FieldDefinition::has_field_rva() const
        {
            return flags_ & FieldDef::kHasFieldRVA;
        }
        
        ITypeReference *FieldDefinition::field_type()
        {
            if (type_)
                return type_;
            
            FieldSignatureConverter converter(model_, signauture_);
            type_ = converter.resolved_type();
            return type_;
        }
        
        LocalDefinition::LocalDefinition(bool is_pinned, ITypeReference *type)
        : is_pinned_(is_pinned)
        , type_(type)
        {}
        
        MethodDefinition::MethodDefinition(PEFileToObjectModel *model, const MethodDef *def,
                                           INamedTypeDefinition *containing_type)
        : DefinitionBase(model)
        , name_(def->Name)
        , flags_(def->Flags)
        , method_def_(def)
        , containing_type_(containing_type)
        {}
        
        bool MethodDefinition::has_this() const
        { return signature_flags_ & SignatureConverter::kHasThis; }
        
        bool MethodDefinition::explicit_this() const
        { return signature_flags_ & SignatureConverter::kExplicitThis; }
        
        bool MethodDefinition::is_abstract() const
        { return flags_ & MethodDef::kAbstract; }
        bool MethodDefinition::is_pinvoke() const
        { return flags_ & MethodDef::kPInvokeImplementation; }

        
        void MethodDefinition::LoadInstructions()
        {
            if (method_il_)
            {
                ILReader il_reader(model_, this);
                il_reader.ReadIL();
                instructions_.swap(il_reader.GetInstructions());
            }
        }
        
        ParameterDefinition::ParameterDefinition(PEFileToObjectModel *model, const std::u16string &name, ITypeReference *type)
        : DefinitionBase(model)
        , name_(name)
        , type_(type)
        {}
        
        CILOperation::CILOperation(Opcode opcode, int offset, const ILOperand &operand)
        : opcode_(opcode)
        , offset_(offset)
        , operand_(operand)
        {}

    }
}
