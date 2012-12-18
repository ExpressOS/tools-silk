//
//  MetadataTable.h
//  silk
//
//  Created by Haohui Mai on 11/25/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#ifndef SILK_LIB_DECIL_METADATA_TABLE_H_
#define SILK_LIB_DECIL_METADATA_TABLE_H_

#include "Metadata.h"
#include "MDLoader.h"

namespace silk
{
    namespace decil
    {
        class ModuleDefinition;
        class TypeReference;
        class TypeDefinition;
        class FieldDef;
        class MethodDef;
        class ParamDef;
        class InterfaceImplementation;
        class MemberReference;
        class ConstantDefinition;
        class CustomAttribute;
        class FieldMarshal;
        class DeclSecurity;
        class ClassLayout;
        class FieldLayout;
        class StandAloneSignature;
        class EventMap;
        class Event;
        class PropertyMap;
        class PropertyDefinition;
        class MethodSemantics;
        class MethodImplementation;
        class ModuleReference;
        class TypeSpecification;
        class ImplementationMap;
        class FieldRVA;
        class AssemblyDefinition;
        class AssemblyProcessor;
        class AssemblyOperatingSystem;
        class AssemblyRef;
        class AssemblyReferenceProcessor;
        class AssemblyReferenceOperatingSystem;
        class File;
        class ExportedType;
        class ManifestResource;
        class NestedClass;
        class GenericParameter;
        class MethodSpecification;
        class GenericParameterConstraint;
        
                
        struct TypeDefOrRefTrait
        {
            static const int tags[];
            static const unsigned tag_length = 3;
            static const unsigned tag_bits = 2;
        };
        
        struct HasConstantTrait
        {
            static const int tags[];
            static const unsigned tag_length = 3;
            static const unsigned tag_bits = 2;
        };
        
        struct HasCustomAttributeTrait
        {
            static const int tags[];
            static const unsigned tag_length = 22;
            static const unsigned tag_bits = 5;
        };
        
        struct HasFieldMarshallTrait
        {
            static const int tags[];
            static const unsigned tag_length = 2;
            static const unsigned tag_bits = 1;
        };
        
        struct HasDeclSecurityTrait
        {
            static const int tags[];
            static const unsigned tag_length = 3;
            static const unsigned tag_bits = 2;
        };
        
        struct MemberRefParentTrait
        {
            static const int tags[];
            static const unsigned tag_length = 5;
            static const unsigned tag_bits = 3;
        };
        
        struct HasSemanticsTrait
        {
            static const int tags[];
            static const unsigned tag_length = 2;
            static const unsigned tag_bits = 1;
        };
        
        struct MethodDefOrRefTrait
        {
            static const int tags[];
            static const unsigned tag_length = 2;
            static const unsigned tag_bits = 1;
        };
        
        struct MemberForwardedTrait
        {
            static const int tags[];
            static const unsigned tag_length = 2;
            static const unsigned tag_bits = 1;
        };
        
        struct ImplementationTrait
        {
            static const int tags[];
            static const unsigned tag_length = 3;
            static const unsigned tag_bits = 2;
        };
        
        struct CustomAttributeTypeTrait
        {
            static const int tags[];
            static const unsigned tag_length = 5;
            static const unsigned tag_bits = 3;
        };
        
        struct ResolutionScopeTrait
        {
            static const int tags[];
            static const unsigned tag_length = 4;
            static const unsigned tag_bits = 2;
        };
        
        struct TypeOrMethodDefTrait
        {
            static const int tags[];
            static const unsigned tag_length = 2;
            static const unsigned tag_bits = 1;
        };
        
        class ModuleDefinition : public MDRowBase
        {
        public:
            static unsigned id() { return kModuleDefinition; }
            virtual void Load(MDLoader &loader);
            MDString Name;
            MDGUID Mvid;
            MDGUID EncId;
            MDGUID EncBaseId;
        };
        
        class TypeRef : public MDRowBase
        {
        public:
            static unsigned id() { return kTypeReference; }
            virtual void Load(MDLoader &loader);
            MDCodedToken<ResolutionScopeTrait> ResolutionScope;
            MDString TypeName;
            MDString TypeNamespace;
        };
        
        class TypeDefinition : public MDRowBase
        {
        public:
            enum
            {
                kPrivateAccess = 0x00000000,
                kPublicAccess = 0x00000001,
                kNestedPublicAccess = 0x00000002,
                kNestedPrivateAccess = 0x00000003,
                kNestedFamilyAccess = 0x00000004,
                kNestedAssemblyAccess = 0x00000005,
                kNestedFamilyAndAssemblyAccess = 0x00000006,
                kNestedFamilyOrAssemblyAccess = 0x00000007,
                kAccessMask = 0x0000007,
                kNestedMask = 0x00000006,
                
                kAutoLayout = 0x00000000,
                kSeqentialLayout = 0x00000008,
                kExplicitLayout = 0x00000010,
                kLayoutMask = 0x00000018,
                
                kClassSemantics = 0x00000000,
                kInterfaceSemantics = 0x00000020,
                kAbstractSemantics = 0x00000080,
                kSealedSemantics = 0x00000100,
                kSpecialNameSemantics = 0x00000400,
                kRTSpecialNameReserved = 0x00000800,
                
                kImportImplementation = 0x00001000,
                kSerializableImplementation = 0x00002000,
                kIsForeign = 0x00004000,
                
                kAnsiString = 0x00000000,
                kUnicodeString = 0x00010000,
                kAutoCharString = 0x00020000,
                kStringMask = 0x00030000,
                kHasSecurityReserved = 0x00040000,
                
                kBeforeFieldInitImplementation = 0x00100000,
                kForwarderImplementation = 0x00200000,
                kDoesNotInheritTypeParameters = 0x10000000,
            };
            
            static unsigned id() { return kTypeDefinition; }
            virtual void Load(MDLoader &loader);
            bool is_nested() const { return Flags & kNestedMask; }
            
            uint32_t Flags;
            MDString TypeName;
            MDString TypeNamespace;
            MDCodedToken<TypeDefOrRefTrait> Extends;
            MDSimpleToken<FieldDef> FieldList;
            MDSimpleToken<MethodDef> MethodList;
        };
        
        class FieldDef : public MDRowBase
        {
        public:
            enum
            {
                kFieldAccessMask = 0x0007,
                kCompilerControlled = 0x0000,
                kPrivate = 0x0001,
                kFamilyAndAssembly = 0x0002,
                kAssembly = 0x0003,
                kFamily = 0x0004,
                kFamilyOrAssembly = 0x0005,
                kPublic = 0x0006,
                kStatic = 0x0010,
                kInitOnly = 0x0020,
                kLiteral = 0x0040,
                kNotSerialized = 0x0080,
                kSpecialName = 0x0200,
                kPInvokeImplementation = 0x2000,
                kRTSpecialName = 0x0400,
                kHasFieldMarshal = 0x1000,
                kHasDefault = 0x8000,
                kHasFieldRVA = 0x0100,
            };
            
            static unsigned id() { return kField; }
            virtual void Load(MDLoader &loader);
            uint16_t Flags;
            MDString Name;
            MDBlob Signature;
        };
        
        class MethodDef : public MDRowBase
        {
        public:
            /* Method Flags */
            enum
            {
                kMemberAccessMask = 0x0007,
                kCompilerControlled = 0x0000,
                kPrivate = 0x0001,
                kFamilyAndAssembly = 0x0002,
                kAssembly = 0x0003,
                kFamily = 0x0004,
                kFamilyOrAssembly = 0x0005,
                kPublic = 0x0006,
                kStatic = 0x0010,
                kFinal = 0x0020,
                kVirtual = 0x0040,
                kHideBySignature = 0x0080,
                kVTableLayoutMask = 0x0100,
                kReuseSlot = 0x0000,
                kNewSlot = 0x0100,
                kStrict = 0x0200,
                kAbstract = 0x0400,
                kSpecialName = 0x0800,
                kPInvokeImplementation = 0x2000,
                kUnmanagedExport = 0x0008,
                kRTSpecialName = 0x1000,
                kHasSecurity = 0x4000,
                kRequireSecurityObject = 0x8000,
            };
            
            static unsigned id() { return kMethodDefinition; }
            virtual void Load(MDLoader &loader);
            uint32_t RVA;
            uint16_t ImplFlags;
            uint16_t Flags;
            MDString Name;
            MDBlob Signature;
            MDSimpleToken<ParamDef> ParamList;
        };
        
        class ParamDef : public MDRowBase
        {
        public:
            static unsigned id() { return kParameter; }
            virtual void Load(MDLoader &loader);
            uint16_t Flags;
            uint16_t Sequence;
            MDString Name;
        };
        
        class InterfaceImplementation : public MDRowBase
        {
        public:
            static unsigned id() { return kInterfaceImplementation; }
            virtual void Load(MDLoader &loader);
            MDSimpleToken<TypeDefinition> Class;
            MDCodedToken<TypeDefOrRefTrait> Interface;
        };
        
        class MemberReference : public MDRowBase
        {
        public:
            static unsigned id() { return kMemberReference; }
            virtual void Load(MDLoader &loader);
            MDCodedToken<MemberRefParentTrait> Class;
            MDString Name;
            MDBlob Signature;
        };
        
        class ConstantDefinition : public MDRowBase
        {
        public:
            static unsigned id() { return kConstant; }
            virtual void Load(MDLoader &loader);
            uint16_t Type;
            MDCodedToken<HasConstantTrait> Parent;
            MDBlob Value;
        };
        
        class CustomAttribute : public MDRowBase
        {
        public:
            static unsigned id() { return kCustomAttribute; }
            virtual void Load(MDLoader &loader);
            MDCodedToken<HasCustomAttributeTrait> Parent;
            MDCodedToken<CustomAttributeTypeTrait> Type;
            MDBlob Value;
        };
        
        class DeclSecurity : public MDRowBase
        {
        public:
            static unsigned id() { return kDeclSecurity; }
            virtual void Load(MDLoader &loader);
            uint16_t Action;
            MDCodedToken<HasDeclSecurityTrait> Parent;
            MDBlob PermissionSet;
        };
        
        class ClassLayout : public MDRowBase
        {
        public:
            static unsigned id() { return kClassLayout; }
            virtual void Load(MDLoader &loader);
            uint16_t PackingSize;
            uint32_t ClassSize;
            MDSimpleToken<TypeDefinition> Parent;
        };
        
        class StandAloneSignature : public MDRowBase
        {
        public:
            static unsigned id() { return kStandAloneSignature; }
            virtual void Load(MDLoader &loader);
            MDBlob Signature;
        };
        
        class PropertyMap : public MDRowBase
        {
        public:
            static unsigned id() { return kPropertyMap; }
            virtual void Load(MDLoader &loader);
            MDSimpleToken<TypeDefinition> Parent;
            MDSimpleToken<PropertyDefinition> PropertyList;
        };
        
        class PropertyDefinition : public MDRowBase
        {
        public:
            static unsigned id() { return kProperty; }
            virtual void Load(MDLoader &loader);
            uint16_t Flags;
            MDString Name;
            MDBlob Type;
        };
        
        class MethodSemantics : public MDRowBase
        {
        public:
            static unsigned id() { return kMethodSemantics; }
            virtual void Load(MDLoader &loader);
            uint16_t Semantics;
            MDSimpleToken<MethodDef> Method;
            MDCodedToken<HasSemanticsTrait> Association;
        };
        
        class MethodImplementation : public MDRowBase
        {
        public:
            static unsigned id() { return kMethodImplementation; }
            virtual void Load(MDLoader &loader);
            MDSimpleToken<TypeDefinition> Class;
            MDCodedToken<MethodDefOrRefTrait> MethodBody;
            MDCodedToken<MethodDefOrRefTrait> MethodDeclaration;
        };
        
        class ModuleReference : public MDRowBase
        {
        public:
            static unsigned id() { return kModuleReference; }
            virtual void Load(MDLoader &loader);
            MDString Name;
        };
        
        
        class TypeSpecification : public MDRowBase
        {
        public:
            static unsigned id() { return kTypeSpecification; }
            virtual void Load(MDLoader &loader);
            MDBlob Signature;
        };
        
        class ImplementationMap : public MDRowBase
        {
        public:
            static unsigned id() { return kImplementationMap; }
            virtual void Load(MDLoader &loader);
            uint16_t MappingFlags;
            MDCodedToken<MemberForwardedTrait> MemberForwarded;
            MDString ImportName;
            MDSimpleToken<ModuleReference> ImportScope;
        };
        
        class FieldRVA : public MDRowBase
        {
        public:
            static unsigned id() { return kFieldRVA; }
            virtual void Load(MDLoader &loader);
            uint32_t RVA;
            MDSimpleToken<TypeDefinition> Field;
        };
        
        class AssemblyDefinition : public MDRowBase
        {
        public:
            static unsigned id() { return kAssemblyDefinition; }
            virtual void Load(MDLoader &loader);
            uint32_t HashAlgId;
            uint16_t MajorVersion;
            uint16_t MinorVersion;
            uint16_t BuildNumber;
            uint16_t RevisionNumber;
            uint32_t Flags;
            MDBlob PublicKey;
            MDString Name;
            MDString Culture;
        };
        
        class AssemblyRef : public MDRowBase
        {
        public:
            static unsigned id() { return kAssemblyReference; }
            virtual void Load(MDLoader &loader);
            uint16_t MajorVersion;
            uint16_t MinorVersion;
            uint16_t BuildNumber;
            uint16_t RevisionNumber;
            uint32_t Flags;
            MDBlob PublicKeyOrToken;
            MDString Name;
            MDString Culture;
            MDBlob HashValue;
        };
        
        class NestedClass : public MDRowBase
        {
        public:
            static unsigned id() { return kNestedClass; }
            virtual void Load(MDLoader &loader);
            MDSimpleToken<TypeDefinition> NestedClass;
            MDSimpleToken<TypeDefinition> EnclosingClass;
        };
        
        class GenericParameter : public MDRowBase
        {
        public:
            static unsigned id() { return kGenericParameter; }
            virtual void Load(MDLoader &loader);
            uint16_t Number;
            uint16_t Flags;
            MDCodedToken<TypeOrMethodDefTrait> Owner;
            MDString Name;
        };
        
    }
}

#endif

