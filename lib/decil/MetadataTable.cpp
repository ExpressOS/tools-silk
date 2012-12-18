//
//  MetadataTable.cpp
//  silk
//
//  Created by Haohui Mai on 11/25/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#include "MetadataTable.h"
#include "PEFileReader.h"

namespace silk
{
    namespace decil
    {
        const int ResolutionScopeTrait::tags[] =
        {
            kModuleDefinition,
            kModuleReference,
            kAssemblyReference,
            kTypeReference,
        };
        
        const int TypeDefOrRefTrait::tags[] =
        {
            kTypeDefinition,
            kTypeReference,
            kTypeSpecification,
        };
        
        const int HasConstantTrait::tags[] =
        {
            kField,
            kParameter,
            kProperty,
        };
        
        const int HasCustomAttributeTrait::tags[] =
        {
            kMethodDefinition,
            kField,
            kTypeReference,
            kTypeDefinition,
            kParameter,
            kInterfaceImplementation,
            kMemberReference,
            kModuleDefinition,
            /* kPermission */
            kInvalidTableReference,
            kProperty,
            kEvent,
            kStandAloneSignature,
            kModuleReference,
            kTypeSpecification,
            kAssemblyDefinition,
            kAssemblyReference,
            kFile,
            kExportedType,
            kManifestResource,
            kGenericParameter,
            kGenericParameterConstraint,
            kMethodSpecification,
        };
        
        const int HasFieldMarshallTrait::tags[] =
        {
            kField,
            kParameter,
        };
        
        const int HasDeclSecurityTrait::tags[] =
        {
            kTypeDefinition,
            kMethodDefinition,
            kAssemblyDefinition,
        };
        
        const int MemberRefParentTrait::tags[] =
        {
            kTypeDefinition,
            kTypeReference,
            kModuleReference,
            kMethodDefinition,
            kTypeSpecification,
        };
        
        const int HasSemanticsTrait::tags[] =
        {
            kEvent,
            kProperty,
        };
        
        const int MethodDefOrRefTrait::tags[] =
        {
            kMethodDefinition,
            kMemberReference,
        };
        
        const int MemberForwardedTrait::tags[] =
        {
            kField,
            kMethodDefinition,
        };
        
        const int ImplementationTrait::tags[] =
        {
            kFile,
            kAssemblyReference,
            kExportedType,
        };
        
        const int CustomAttributeTypeTrait::tags[] =
        {
            kInvalidTableReference,
            kInvalidTableReference,
            kMethodDefinition,
            kMemberReference,
            kInvalidTableReference,
        };
        
        const int TypeOrMethodDefTrait::tags[] =
        {
            kTypeDefinition,
            kMethodDefinition,
        };
        
        void AssemblyDefinition::Load(MDLoader &loader)
        {
            loader.stream() >> HashAlgId >> MajorVersion >> MinorVersion
            >> BuildNumber >> RevisionNumber >> Flags;
            loader.Load(&PublicKey).Load(&Name).Load(&Culture);
        }
        
        void AssemblyRef::Load(MDLoader &loader)
        {
            loader.stream() >> MajorVersion >> MinorVersion
            >> BuildNumber >> RevisionNumber >> Flags;
            loader.Load(&PublicKeyOrToken).Load(&Name).Load(&Culture)
            .Load(&HashValue);
        }
        
        void ClassLayout::Load(MDLoader &loader)
        {
            loader.stream() >> PackingSize >> ClassSize;
            loader.Load(&Parent);
        }
        
        void ConstantDefinition::Load(MDLoader &loader)
        {
            loader.stream() >> Type;
            loader.Load(&Parent).Load(&Value);
        }
        
        void CustomAttribute::Load(MDLoader &loader)
        {
            loader.Load(&Parent).Load(&Type).Load(&Value);
        }
        
        
        void DeclSecurity::Load(MDLoader &loader)
        {
            loader.stream() >> Action;
            loader.Load(&Parent).Load(&PermissionSet);
        }
        
        void FieldDef::Load(MDLoader &loader)
        {
            loader.stream() >> Flags;
            loader.Load(&Name).Load(&Signature);
        }
        
        void FieldRVA::Load(MDLoader &loader)
        {
            loader.stream() >> RVA;
            loader.Load(&Field);
        }
        
        void GenericParameter::Load(MDLoader &loader)
        {
            loader.stream() >> Number >> Flags;
            loader.Load(&Owner).Load(&Name);
        }
        
        void ImplementationMap::Load(MDLoader &loader)
        {
            loader.stream() >> MappingFlags;
            loader.Load(&MemberForwarded).Load(&ImportName).Load(&ImportScope);
        }
        
        void InterfaceImplementation::Load(MDLoader &loader)
        {
            loader.Load(&Class).Load(&Interface);
        }
        
        void MemberReference::Load(MDLoader &loader)
        {
            loader.Load(&Class).Load(&Name).Load(&Signature);
        }
        
        void MethodDef::Load(MDLoader &loader)
        {
            loader.stream() >> RVA >> ImplFlags >> Flags;
            loader.Load(&Name).Load(&Signature).Load(&ParamList);
        }
        
        void MethodImplementation::Load(MDLoader &loader)
        {
            loader.Load(&Class).Load(&MethodBody).Load(&MethodDeclaration);
        }
        
        void MethodSemantics::Load(MDLoader &loader)
        {
            loader.stream() >> Semantics;
            loader.Load(&Method).Load(&Association);
        }
        
        void ModuleDefinition::Load(MDLoader &loader)
        {
            loader.stream().skip(2);
            loader.Load(&Name).Load(&Mvid).Load(&EncId).Load(&EncBaseId);
        }
        
        void ModuleReference::Load(MDLoader &loader)
        {
            loader.Load(&Name);
        }
        
        void NestedClass::Load(MDLoader &loader)
        {
            loader.Load(&NestedClass).Load(&EnclosingClass);
        }
        
        void ParamDef::Load(MDLoader &loader)
        {
            loader.stream() >> Flags >> Sequence;
            loader.Load(&Name);
        }
        
        void PropertyDefinition::Load(MDLoader &loader)
        {
            loader.stream() >> Flags;
            loader.Load(&Name).Load(&Type);
        }
        
        void PropertyMap::Load(MDLoader &loader)
        {
            loader.Load(&Parent).Load(&PropertyList);
        }
        
        void StandAloneSignature::Load(MDLoader &loader)
        {
            loader.Load(&Signature);
        }
        
        void TypeDefinition::Load(MDLoader &loader)
        {
            loader.stream() >> Flags;
            loader.Load(&TypeName).Load(&TypeNamespace).Load(&Extends)
            .Load(&FieldList).Load(&MethodList);
        }
        
        void TypeRef::Load(MDLoader &loader)
        {
            loader.Load(&ResolutionScope).Load(&TypeName).Load(&TypeNamespace);
        }
        
        void TypeSpecification::Load(MDLoader &loader)
        {
            loader.Load(&Signature);
        }
    }
}

