//
//  SignatureConverter.cpp
//  silk
//
//  Created by Haohui Mai on 12/1/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#include "SignatureConverter.h"
#include "PEFileToObjectModel.h"
#include "PlatformTypes.h"
#include "Host.h"
#include "MetadataTable.h"
#include "BinaryObjectModel.h"

namespace silk
{
    namespace decil
    {
        SignatureConverter::SignatureConverter(PEFileToObjectModel *model)
        : model_(model)
        {}
        
        ITypeReference *SignatureConverter::ReadType(raw_istream &is)
        {
            uint8_t idx;
            is >> idx;
            auto host = model_->host();
            switch (idx)
            {
                case kElementTypeVoid:
                    return host->platform_type()->system_void();
                case kElementTypeBoolean:
                    return host->platform_type()->system_boolean();
                case kElementTypeChar:
                    return host->platform_type()->system_char();
                case kElementTypeI1:
                    return host->platform_type()->system_int8();
                case kElementTypeU1:
                    return host->platform_type()->system_uint8();
                case kElementTypeI2:
                    return host->platform_type()->system_int16();
                case kElementTypeU2:
                    return host->platform_type()->system_uint16();
                case kElementTypeI4:
                    return host->platform_type()->system_int32();
                case kElementTypeU4:
                    return host->platform_type()->system_uint32();
                case kElementTypeI8:
                    return host->platform_type()->system_int64();
                case kElementTypeU8:
                    return host->platform_type()->system_uint64();
                case kElementTypeR4:
                    return host->platform_type()->system_float32();
                case kElementTypeR8:
                    return host->platform_type()->system_float64();
                    
                case kElementTypeString:
                    return host->platform_type()->system_string();
                    
                case kElementTypeValueType:
                case kElementTypeClass:
                {
                    MDCodedToken<TypeDefOrRefTrait> tok;
                    tok.LoadFromInt(is.read_compressed_uint32());
                    return model_->GetTypeReferenceForToken(&tok);
                }
                    
                case kElementTypeIPointer:
                    return host->platform_type()->system_intptr();
                    
                case kElementTypeUPointer:
                    return host->platform_type()->system_uintptr();
                    
                case kElementTypeObject:
                    return host->platform_type()->system_object();
                    
                case kElementTypePointer:
                case kElementTypeByReference:
                    return new PointerType(ReadType(is));
                    
                case kElementTypeSingleDimensionArray:
                    return new VectorType(ReadType(is));
                    
                default:
                    assert (0 && "Unimplemented");
                    break;
            }
            return NULL;
        }
        
        FieldSignatureConverter::FieldSignatureConverter(PEFileToObjectModel *model, const raw_istream &signauture)
        : SignatureConverter(model)
        
        {
            auto is = signauture;
            uint8_t type;
            is >> type;
            assert(IsFieldSignature(type));
            
            resolved_type_ = ReadType(is);
        }
        
        MethodSignatureConverter::MethodSignatureConverter(PEFileToObjectModel *model, const raw_istream &signauture)
        : SignatureConverter(model)
        {
            auto is = signauture;
            is >> flags_;
            assert (IsMethodSignature(flags_));
            generic_param_count_ = IsGenericInstanceSignature(flags_) ? is.read_compressed_uint32() : 0;
            auto param_count = is.read_compressed_uint32();

            return_type_ = ReadType(is);
            param_type_.reserve(param_count);
            for (auto i = 0; i < param_count; ++i)
            {
                param_type_.push_back(ReadType(is));
            }
        }
        
        LocalVariableSignatureConverter::LocalVariableSignatureConverter(PEFileToObjectModel *model,
                                                                         const raw_istream &signature)
        : SignatureConverter(model)
        {
            auto is = signature;
            is >> flags_;
            assert (IsLocalVariableSignature(flags_));
            unsigned count = is.read_compressed_uint32();
            locals_.reserve(count);

            for (size_t i = 0; i < count; ++i)
            {
                bool is_pinned = false;
                auto next = is.peek<char>();
                if (next == kElementTypePinned)
                {
                    is_pinned = true;
                    is.skip(1);
                }
                
                locals_.push_back(new LocalDefinition(is_pinned, ReadType(is)));
            }
        }
    }
}

