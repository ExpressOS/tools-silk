//
//  PEFileToObjectModel.h
//  silk
//
//  Created by Haohui Mai on 11/26/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#ifndef SILK_LIB_DECIL_PEFILE_TO_OBJECT_MODEL_H_
#define SILK_LIB_DECIL_PEFILE_TO_OBJECT_MODEL_H_

#include "BinaryObjectModel.h"
#include "MetadataTable.h"

#include <functional>

namespace silk
{
    namespace decil
    {
        class PEFileReader;
        class MethodDefinition;
        class Host;
        
        class PEFileToObjectModel
        {
        public:
            PEFileToObjectModel(Host *host, PEFileReader *file);
            Assembly *containing_assembly() const { return containing_assembly_; }
            const std::vector<INamedTypeDefinition*> &named_typedefs() const
            { return named_typedefs_; }
            Host *host()
            { return host_; }
            PEFileReader *file()
            { return file_; }
            
            static std::u16string MangleParams(const std::vector<ITypeReference*> &params);
            INamedTypeDefinition *GetTypeDefinitionAtRow(size_t idx);
            ITypeReference *GetTypeReferenceForToken(const MDTokenBase *tok);
            IMethodReference *GetMethodReferenceForToken(const MDTokenBase *tok);
            IFieldReference *GetFieldReferenceForToken(const MDTokenBase *tok);
            INamedTypeDefinition *ResolveNamespaceTypeDefinition(const std::u16string &namespace_name, const std::u16string &name);
            void LoadMethodDefinition(MethodDefinition *method, const MethodDef *def);
            raw_istream GetFieldMapping(const FieldDef *field_def);
            void GetClassLayout(const TypeDefinition *type_def, uint32_t *packing_size, uint32_t * class_size);
        private:
            Host *host_;
            Assembly *containing_assembly_;
            PEFileReader *file_;
            
            std::vector<AssemblyReference*> assembly_references_;
            std::vector<INamedTypeDefinition*> named_typedefs_;
            std::vector<ITypeReference*> type_refs_;
            std::vector<FieldDefinition*> fields_;
            std::vector<MethodDefinition*> methods_;
            std::vector<ITypeMemberReference*> member_refs_;
            
            void LoadAssemblyReferences();
            void LoadTypeDefinitions();
            void LoadTypeBaseMembers(TypeBase *type);
            void EnsureParentTypeLoaded(int idx, std::function<int(const decil::TypeDefinition&)> func);
            
            unsigned FindParentOfNestedClassByRowId(size_t idx);
            
            AssemblyReference *GetAssemblyReferenceAtRow(size_t idx);
            ITypeReference *GetTypeReferenceAtRow(size_t idx);
            FieldDefinition *GetFieldDefAtRow(size_t idx);
            MethodDefinition *GetMethodDefAtRow(size_t idx);
            ITypeMemberReference *GetMemberReferenceAtRow(size_t idx);
            
            INamedTypeDefinition::TypeCode GetTypeCodeForTypeDefAtRow(size_t idx) const;
            
            MethodDefinition *CreateMethodDefinitionAtRow(size_t idx, TypeBase *containing_type);
            FieldDefinition *CreateFieldDefinitionAtRow(size_t idx, TypeBase *containing_type);

        };
    }
}

#endif
