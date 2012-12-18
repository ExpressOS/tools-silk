//
//  BinaryObjectModel.h
//  silk
//
//  Created by Haohui Mai on 11/26/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#ifndef SILK_LIB_DECIL_BINARY_OBJECT_MODEL_H_
#define SILK_LIB_DECIL_BINARY_OBJECT_MODEL_H_

#include "MetadataTable.h"
#include "silk/decil/ObjectModel.h"

#include <string>

namespace silk
{
    namespace decil
    {
        class IHost;
        class PEFileToObjectModel;
        class FieldDefinition;
        class MethodDefinition;
        class MethodIL;
        
        class MetadataObjectModelBase
        {
        public:
            virtual ~MetadataObjectModelBase();
        };
        
        class DefinitionBase : public MetadataObjectModelBase
        {
        public:
            PEFileToObjectModel *model()
            { return model_; }
        protected:
            DefinitionBase(PEFileToObjectModel *model);
            PEFileToObjectModel *model_;
        };
        
        class TypeBase : virtual public INamedTypeDefinition, protected DefinitionBase
        {
        public:
            friend class PEFileToObjectModel;
            TypeBase(PEFileToObjectModel *model, const TypeDefinition *type_def, const std::u16string &mangled_name);
            virtual IFieldDefinition** field_begin() override final
            { return &fields_.front(); }
            virtual IFieldDefinition** field_end() override final
            { return &fields_.back() + 1; }
            virtual IMethodDefinition** method_begin() override final
            { return &methods_.front(); }
            virtual IMethodDefinition** method_end() override final
            { return &methods_.back() + 1; }
            virtual IGenericTypeParameter** generic_begin() override final
            { return &generic_params_.front(); }
            virtual IGenericTypeParameter** generic_end() override final
            { return &generic_params_.back() + 1; }
            
            virtual const std::u16string &name() override
            { return mangled_name_; }
            
            virtual ITypeReference *base_class() const override final;
            virtual uint32_t packing_size() const override final
            { return packing_size_; }
            virtual uint32_t class_size() const override final
            { return class_size_; }


        protected:
            const TypeDefinition *type_def_;
            std::u16string mangled_name_;
            mutable ITypeReference *base_class_;
            std::vector<IFieldDefinition*> fields_;
            std::vector<IMethodDefinition*> methods_;
            std::vector<IGenericTypeParameter*> generic_params_;
            uint32_t packing_size_;
            uint32_t class_size_;
        };
        
        class SystemDefinedStructuralType : virtual public ITypeDefinition
        {
        public:
            virtual ITypeReference *base_class() const override final
            { return nullptr; }
            virtual IFieldDefinition** field_begin() override final
            { return nullptr; }
            virtual IFieldDefinition** field_end() override final
            { return nullptr; }
            virtual IMethodDefinition** method_begin() override final
            { return nullptr; }
            virtual IMethodDefinition** method_end() override final
            { return nullptr; }
            virtual IGenericTypeParameter** generic_begin() override final
            { return nullptr; }
            virtual IGenericTypeParameter** generic_end() override final
            { return nullptr; }
        };
        
        class Assembly : public IAssembly, public DefinitionBase
        {
        public:
            virtual std::vector<INamedTypeDefinition*>::const_iterator all_types_begin() const override final;
            virtual std::vector<INamedTypeDefinition*>::const_iterator all_types_end() const override final;
            Assembly(PEFileToObjectModel *model, const AssemblyIdentity &id);
            virtual const AssemblyIdentity &identity() const override
            { return id_; }
        private:
            AssemblyIdentity id_;
        };
        
        class AssemblyReference : public IAssemblyReference, public MetadataObjectModelBase
        {
        public:
            AssemblyReference(IHost *host, const AssemblyIdentity &id);
            virtual IAssembly *ResolvedAssembly() override final;
            virtual const AssemblyIdentity &identity() const override final
            { return id_; }
        private:
            IHost *host_;
            AssemblyIdentity id_;
        };
        
        class NonGenericNamespaceType : public TypeBase
        {
        public:
            NonGenericNamespaceType(PEFileToObjectModel *model, const TypeDefinition *e);
            virtual TypeCode type_code() const override
            { return TypeCode::NotPrimitive; }
        };
        
        class NonGenericNamespaceTypeWithPrimitiveType : public NonGenericNamespaceType
        {
        public:
            NonGenericNamespaceTypeWithPrimitiveType(PEFileToObjectModel *model, const TypeDefinition *e, TypeCode type_code);
            virtual TypeCode type_code() const override final
            { return type_code_; }
        private:
            const TypeCode type_code_;
        };
        
        class NonGenericNestedType : public INestedType, public TypeBase
        {
        public:
            NonGenericNestedType(PEFileToObjectModel *model, const TypeDefinition *e, unsigned owning_type_idx);
            
            virtual ITypeDefinition *owning_type() override final;
            virtual TypeCode type_code() const override final
            { return TypeCode::NotPrimitive; }
            
            const std::u16string &name() override final;

        private:
            mutable ITypeDefinition *owning_type_;
            unsigned owning_type_idx_;
        };
        
        class TypeReference : public ITypeReference
        {
        public:
            virtual ITypeDefinition *resolved_type() override final;
            TypeReference(IAssemblyReference *assembly_ref, const std::u16string &name, const std::u16string &namespace_name);
        private:
            enum Scope
            {
                kTypeRef,
                kAssemblyRef,
            };
            Scope type_;
            ITypeDefinition *resolved_type_;
            IAssemblyReference *assembly_ref_;
            const std::u16string name_;
            const std::u16string namespace_name_;
        };
        
        class PointerType : public SystemDefinedStructuralType, public IPointerType
        {
        public:
            PointerType(ITypeReference *target_type);
            virtual ITypeReference *target_type() override final
            { return target_type_; }
            const std::u16string &name() override final;

        private:
            ITypeReference *target_type_;
            mutable std::u16string name_;
        };
        
        class VectorType : public SystemDefinedStructuralType, public IVectorType
        {
        public:
            VectorType(ITypeReference *element_type);
            virtual ITypeReference *element_type() override final
            { return element_type_; }
            const std::u16string &name() override final;

        private:
            ITypeReference *element_type_;
            mutable std::u16string name_;
        };
        
        class FieldReference : public IFieldReference
        {
        public:
            FieldReference(PEFileToObjectModel *model, const MemberReference *member_ref, ITypeReference *parent_ref);
            virtual IFieldDefinition *resolved_definition() override final;
            virtual const std::u16string &name() override final
            { return name_; }
            
        private:
            PEFileToObjectModel *model_;
            std::u16string name_;
            ITypeReference *parent_ref_;
            IFieldDefinition *resolved_def_;
            raw_istream signauture_;
        };
        
        class MethodReference : public IMethodReference
        {
        public:
            MethodReference(PEFileToObjectModel *model, const MemberReference *member_ref, ITypeReference *parent_ref);
            virtual IMethodDefinition *resolved_definition() override final;
            virtual const std::u16string &name() override final
            { return name_; }
            
        private:
            PEFileToObjectModel *model_;
            std::u16string name_;
            ITypeReference *parent_ref_;
            IMethodDefinition *resolved_def_;
            raw_istream signauture_;

        };
        
        class FieldDefinition : public IFieldDefinition, public DefinitionBase
        {
        public:
            FieldDefinition(PEFileToObjectModel *model, const FieldDef *def, INamedTypeDefinition *containing_type);
            virtual INamedTypeDefinition *containing_type() override final
            { return containing_type_; }
            const std::u16string &name() override final;
            virtual bool is_static() const override final;
            virtual bool is_literal() const override final;
            bool has_field_rva() const;
            virtual ITypeReference *field_type() override final;
            virtual raw_istream field_mapping() const override final
            { return mapping_; }
            
        private:
            std::u16string name_;
            INamedTypeDefinition *containing_type_;
            uint16_t flags_;
            ITypeReference *type_;
            raw_istream signauture_;
            raw_istream mapping_;
        };
        
        class LocalDefinition : public ILocalDefinition
        {
        public:
            virtual bool is_pinned() const override final
            { return is_pinned_; }
            virtual ITypeReference *type() override final
            { return type_; }
            LocalDefinition(bool is_pinned, ITypeReference *type);
        private:
            bool is_pinned_;
            ITypeReference *type_;
        };
        
        class MethodDefinition : public IMethodDefinition, public DefinitionBase
        {
        public:
            friend class PEFileToObjectModel;
            MethodDefinition(PEFileToObjectModel *model, const MethodDef *def, INamedTypeDefinition *containing_type);
            virtual const std::u16string &name() override final
            { return name_; }
            
            virtual bool has_this() const override final;
            virtual bool explicit_this() const override final;
            virtual bool is_abstract() const override final;
            virtual bool is_pinvoke() const override final;

            virtual INamedTypeDefinition *containing_type() override final
            { return containing_type_; }

            virtual ITypeReference *return_type() override final
            { return return_type_; }
            virtual IParameterDefinition **param_begin() override final
            { return &params_[0]; }
            virtual IParameterDefinition **param_end() override final
            { return &params_.back() + 1; }
            virtual IParameterDefinition *get_param(int idx) override final
            { return params_[idx]; }
            
            virtual ILocalDefinition **local_begin() override final
            { return &locals_[0]; }
            virtual ILocalDefinition **local_end() override final
            { return &locals_.back() + 1; }
            ILocalDefinition *get_local(int idx) const
            { return locals_[idx]; }

            virtual IOperation **inst_begin() override final
            { return &instructions_[0]; }
            virtual IOperation **inst_end() override final
            { return &instructions_.back() + 1; }
            
            const MethodIL *method_il() const
            { return method_il_; }
            const MethodDef *method_def() const
            { return method_def_; }
            void LoadInstructions();
        private:
            std::u16string name_;
            uint8_t signature_flags_;
            uint16_t flags_;
            const MethodDef *method_def_;
            INamedTypeDefinition *containing_type_;
            ITypeReference *return_type_;
            MethodIL *method_il_;
            std::vector<IParameterDefinition*> params_;
            std::vector<ILocalDefinition*> locals_;
            std::vector<IOperation*> instructions_;
        };
        
        class ParameterDefinition : public IParameterDefinition, public DefinitionBase
        {
        public:
            virtual const std::u16string &name() override final
            { return name_; }
            virtual ITypeReference *type() override final
            { return type_; }
            ParameterDefinition(PEFileToObjectModel *model, const std::u16string &name, ITypeReference *type);
        private:
            std::u16string name_;
            ITypeReference *type_;
        };
        
        class CILOperation : public IOperation
        {
        public:
            virtual Opcode opcode() const override final
            { return opcode_; }
            virtual int offset() const override final
            { return offset_; }
            virtual const ILOperand &operand() const override final
            { return operand_; }
            CILOperation(Opcode opcode, int offset, const ILOperand &operand);
            
        private:
            Opcode opcode_;
            int offset_;
            ILOperand operand_;
        };
    }
}

#endif
