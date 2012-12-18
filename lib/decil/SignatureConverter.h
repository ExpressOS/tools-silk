//
//  SignatureConverter.h
//  silk
//
//  Created by Haohui Mai on 12/1/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#ifndef SILK_LIB_DECIL_SIGNATURE_CONVERTER_H_
#define SILK_LIB_DECIL_SIGNATURE_CONVERTER_H_

#include "silk/Support/raw_istream.h"

#include <vector>

namespace silk
{
    namespace decil
    {
        class PEFileToObjectModel;
        class ITypeReference;
        class FieldDef;
        class ILocalDefinition;
        
        class SignatureConverter
        {
        public:
            static bool IsFieldSignature(uint8_t flag)
            { return (flag & kField) == kField; }

            static bool IsMethodSignature(uint8_t flag)
            { return (flag & kCallingConventionMask) <= kVarArgCall; }
            
            static bool IsLocalVariableSignature(uint8_t flag)
            { return (flag & kLocalVar) == kLocalVar; }
            
            static bool IsGenericInstanceSignature(uint8_t flag)
            { return (flag & kGenericInstance) == kGenericInstance; }

            enum
            {
                kDefaultCall = 0x00,
                kCCall = 0x01,
                kStdCall = 0x02,
                kThisCall = 0x03,
                kFastCall = 0x04,
                kVarArgCall = 0x05,
                kField = 0x06,
                kLocalVar = 0x07,
                kProperty = 0x08,
                kGenericInstance = 0x0A,
                kMax = 0x0C,
                kCallingConventionMask = 0x0F,
                kHasThis = 0x20,
                kExplicitThis = 0x40,
                kGeneric = 0x10,
            };
            
        protected:
            enum
            {
                kElementTypeEnd                        = 0x00,
                kElementTypeVoid                       = 0x01,
                kElementTypeBoolean                    = 0x02,
                kElementTypeChar                       = 0x03,
                kElementTypeI1                         = 0x04,
                kElementTypeU1                         = 0x05,
                kElementTypeI2                         = 0x06,
                kElementTypeU2                         = 0x07,
                kElementTypeI4                         = 0x08,
                kElementTypeU4                         = 0x09,
                kElementTypeI8                         = 0x0A,
                kElementTypeU8                         = 0x0B,
                kElementTypeR4                         = 0x0C,
                kElementTypeR8                         = 0x0D,
                kElementTypeString                     = 0x0E,
                kElementTypePointer                    = 0x0F,
                kElementTypeByReference                = 0x10,
                kElementTypeValueType                  = 0x11,
                kElementTypeClass                      = 0x12,
                kElementTypeVar                        = 0x13,
                kElementTypeArray                      = 0x14,
                kElementTypeGenericInstantiation       = 0x15,
                kElementTypeTypedByReference           = 0x16,
                kElementTypeIPointer                   = 0x18,
                kElementTypeUPointer                   = 0x19,
                kElementTypeFunctionPointer            = 0x1B,
                kElementTypeObject                     = 0x1C,
                kElementTypeSingleDimensionArray       = 0x1D,
                kElementTypeMethodVar                  = 0x1E,
                kElementTypeCustomModifierRequired     = 0x1F,
                kElementTypeCustomModifierOptional     = 0x20,
                kElementTypeInternal                   = 0x21,
                kElementTypeModifier                   = 0x40,
                kElementTypeSentinel                   = 0x41,
                kElementTypePinned                     = 0x45,
                kElementTypeType                       = 0x50,
                kElementTypeCustomAttributeBoxed       = 0x51,
                kElementTypeCustomAttributeField       = 0x53,
                kElementTypeCustomAttributeProperty    = 0x54,
                kElementTypeCustomAttributeEnum        = 0x55,
            };
                        
            SignatureConverter(PEFileToObjectModel *model);
            ITypeReference *ReadType(raw_istream &is);
            
            PEFileToObjectModel *model_;
            raw_istream blob_;
            

        };
        
        class FieldSignatureConverter : private SignatureConverter
        {
        public:
            FieldSignatureConverter(PEFileToObjectModel *model, const raw_istream &signature);
            ITypeReference *resolved_type()
            { return resolved_type_; }
            
        private:
            ITypeReference *resolved_type_;
        };
        
        class MethodSignatureConverter : private SignatureConverter
        {
        public:
            MethodSignatureConverter(PEFileToObjectModel *model, const raw_istream &signature);
            ITypeReference *return_type()
            { return return_type_; }
            
            uint8_t flags() const
            { return flags_; }
            
            bool has_this() const
            { return flags_ & kHasThis; }
            
            bool explicit_this() const
            { return flags_ & kExplicitThis; }

            const std::vector<ITypeReference*> &param_type() const
            { return param_type_; }
        private:
            uint8_t flags_;
            unsigned generic_param_count_;
            
            ITypeReference *return_type_;
            std::vector<ITypeReference*> param_type_;
        };
        
        class LocalVariableSignatureConverter : private SignatureConverter
        {
        public:
            LocalVariableSignatureConverter(PEFileToObjectModel *model, const raw_istream &signature);
            std::vector<ILocalDefinition*> &locals()
            { return locals_; }
            
        private:
            uint8_t flags_;
            std::vector<ILocalDefinition*> locals_;
        };
    }
}

#endif
