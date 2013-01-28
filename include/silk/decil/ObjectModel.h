//
//  ObjectModel.h
//  silk
//
//  Created by Haohui Mai on 11/30/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#ifndef SILK_DECIL_OBJECT_MODEL_H_
#define SILK_DECIL_OBJECT_MODEL_H_

#include "silk/decil/Units.h"

namespace silk
{
    namespace decil
    {
        class IOperation;
        class INamedTypeDefinition;
        class AssemblyIdentity;
        class ITypeDefinition;
        class IFieldDefinition;
        class IGenericTypeParameter;
        class IFieldDefinition;
        class IMethodDefinition;
        class ITypeMemberDefinition;
        class IAssembly;

        class INamedEntity : virtual public IMetadata
        {
        public:
            virtual const std::u16string &name() = 0;
        };
        
        class IModule : virtual public IMetadata
        {
        public:
            ///
            /// Returns all of the types defined in the current module. These are always named types, in other words:
            /// INamespaceTypeDefinition or INestedTypeDefinition instances.
            ///
            virtual std::vector<INamedTypeDefinition*>::const_iterator all_types_begin() const = 0;
            virtual std::vector<INamedTypeDefinition*>::const_iterator all_types_end() const = 0;
            
        };
        
        class IAssemblyReference : virtual public IMetadata
        {
        public:
            virtual IAssembly *ResolvedAssembly() = 0;
            virtual const AssemblyIdentity &identity() const = 0;
        };
        
        class IAssembly : public IModule, public IAssemblyReference
        {
        public:
            virtual IAssembly *ResolvedAssembly() override final
            { return this; }
        };
        
        class ITypeReference : virtual public IMetadata
        {
        public:
            virtual ITypeDefinition *resolved_type() = 0;
        };
        
        class ITypeDefinition : public ITypeReference , public INamedEntity
        {
        public:
            virtual ITypeReference *base_class() const = 0;
            virtual IFieldDefinition** field_begin() = 0;
            virtual IFieldDefinition** field_end() = 0;
            virtual IMethodDefinition** method_begin() = 0;
            virtual IMethodDefinition** method_end() = 0;
            virtual IGenericTypeParameter** generic_begin() = 0;
            virtual IGenericTypeParameter** generic_end() = 0;
            
            virtual ITypeDefinition *resolved_type() override final
            { return this; }
            
        };
        
        class INamedTypeDefinition : public ITypeDefinition
        {
        public:
            enum class TypeCode
            {
                Void,
                Boolean,
                Char,
                Int8,
                UInt8,
                Int16,
                UInt16,
                Int32,
                UInt32,
                Int64,
                UInt64,
                Single,
                Double,
                String,
                IntPtr,
                UIntPtr,
                NotPrimitive,
            };

            virtual TypeCode type_code() const = 0;
            virtual uint32_t packing_size() const = 0;
            virtual uint32_t class_size() const = 0;
        };
        
        class INestedType : virtual public INamedTypeDefinition
        {
        public:
            virtual ITypeDefinition *owning_type() = 0;
        };
        
        class IPointerType : virtual public ITypeDefinition
        {
        public:
            virtual ITypeReference *target_type() = 0;
        };
        
        class IVectorType : virtual public ITypeDefinition
        {
        public:
            virtual ITypeReference *element_type() = 0;
        };
        
        class ITypeMemberReference : public INamedEntity
        {};
        
        class ITypeMemberDefinition : virtual public ITypeMemberReference
        {
        public:
            virtual INamedTypeDefinition *containing_type() = 0;
            
        };
        
        class IFieldReference : virtual public ITypeMemberReference
        {
        public:
            virtual IFieldDefinition *resolved_definition() = 0;
        };
        
        class IMethodReference : virtual public ITypeMemberReference
        {
        public:
            virtual IMethodDefinition *resolved_definition() = 0;
        };
        
        class IFieldDefinition : public ITypeMemberDefinition, public IFieldReference
        {
        public:
            virtual IFieldDefinition *resolved_definition() override final
            { return this; }
            virtual raw_istream field_mapping() const = 0;
            virtual ITypeReference *field_type() = 0;
            virtual bool is_static() const = 0;
            virtual bool is_literal() const = 0;
        };
        
        class ILocalDefinition : public IMetadata
        {
        public:
            virtual bool is_pinned() const = 0;
            virtual ITypeReference *type() = 0;
        };
        
        class IParameterDefinition : public INamedEntity
        {
        public:
            virtual ITypeReference *type() = 0;
        };
        
        class IMethodDefinition : public ITypeMemberDefinition, public IMethodReference
        {
        public:
            virtual IMethodDefinition *resolved_definition() override final
            { return this; }

            virtual bool has_this() const = 0;
            virtual bool explicit_this() const = 0;
            virtual bool is_abstract() const = 0;
            virtual bool is_pinvoke() const = 0;
            virtual ITypeReference *return_type() = 0;
            virtual IParameterDefinition **param_begin() = 0;
            virtual IParameterDefinition **param_end() = 0;
            virtual IParameterDefinition *get_param(int idx) = 0;
            virtual ILocalDefinition **local_begin() = 0;
            virtual ILocalDefinition **local_end() = 0;
            virtual IOperation **inst_begin() = 0;
            virtual IOperation **inst_end() = 0;
        };
        
        enum Opcode {
#define CIL_OPCODE(name, code) k##name = code,
#define CIL_LONG_OPCODE(name, code) k##name = 0xfe << 8 | code,
#include "Opcodes.def"
            kLongOpcodePrefix = 0xFE,
        };
        
        class ILOperand
        {
        public:
            ILOperand();

            enum Type
            {
                kNull,
                kInt,
                kIntArray,
                kFloat,
                kDouble,
                kMetadata,
                kString,
            };
            
            Type type() const { return type_; }
            int64_t GetInt() const;
            float GetFloat() const;
            double GetDouble() const;
            IMetadata *GetMetadata() const;
            const std::u16string &GetString() const;
            const std::vector<int> &GetIntArray() const;
            
            void Clear();
            void SetInt(int64_t v);
            void SetFloat(float v);
            void SetDouble(double v);
            void SetMetadata(IMetadata *v);
            void SetString(const std::u16string &v);
            void SetIntArray(const std::vector<int> &v);
        private:
            Type type_;
            int64_t int64_;
            float float4_;
            double double8_;
            IMetadata *md_;
            std::u16string str_;
            std::vector<int> int_arr_;
        };

        class IOperation
        {
        public:
            virtual ~IOperation();
            virtual Opcode opcode() const = 0;
            virtual int offset() const = 0;
            virtual const ILOperand &operand() const = 0;
        };
        
        
    }
}

#endif
