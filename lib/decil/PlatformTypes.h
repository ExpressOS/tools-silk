//
//  PlatformTypes.h
//  silk
//
//  Created by Haohui Mai on 12/1/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#ifndef SILK_LIB_DECIL_PLATFORM_TYPES_H_
#define SILK_LIB_DECIL_PLATFORM_TYPES_H_

#include "silk/decil/Units.h"

namespace silk
{
    namespace decil
    {
        class IHost;
        class IAssemblyReference;
        class PlatformType : public IPlatformType
        {
        public:
            PlatformType(IHost *host);
            virtual const AssemblyIdentity &corelib_id() const override final
            { return corelib_id_; }
            virtual ITypeReference *system_void() override final
            { return system_void_; }
            virtual ITypeReference *system_boolean() override final
            { return system_boolean_; }
            virtual ITypeReference *system_char() override final
            { return system_char_; }
            virtual ITypeReference *system_int8() override final
            { return system_int8_; }
            virtual ITypeReference *system_int16() override final
            { return system_int16_; }
            virtual ITypeReference *system_int32() override final
            { return system_int32_; }
            virtual ITypeReference *system_int64() override final
            { return system_int64_; }
            virtual ITypeReference *system_uint8() override final
            { return system_uint8_; }
            virtual ITypeReference *system_uint16() override final
            { return system_uint16_; }
            virtual ITypeReference *system_uint32() override final
            { return system_uint32_; }
            virtual ITypeReference *system_uint64() override final
            { return system_uint64_; }
            virtual ITypeReference *system_float32() override final
            { return system_float32_; }
            virtual ITypeReference *system_float64() override final
            { return system_float64_; }
            virtual ITypeReference *system_intptr() override final
            { return system_intptr_; }
            virtual ITypeReference *system_uintptr() override final
            { return system_uintptr_; }
            virtual ITypeReference *system_string() override final
            { return system_string_; }
            virtual ITypeReference *system_object() override final
            { return system_object_; }
            virtual ITypeReference *system_enum() override final
            { return system_enum_; }
            virtual ITypeReference *system_value_type() override final
            { return system_value_type_; }
            virtual ITypeReference *system_array() override final
            { return system_array_; }
            virtual ITypeReference *system_runtime_field_handle() override final
            { return system_runtime_field_handle_; }
            
        private:
            IHost *host_;
            AssemblyIdentity corelib_id_;
            IAssemblyReference *corelib_reference_;
            
            ITypeReference *system_void_;
            ITypeReference *system_boolean_;
            ITypeReference *system_char_;
            ITypeReference *system_int8_;
            ITypeReference *system_int16_;
            ITypeReference *system_int32_;
            ITypeReference *system_int64_;
            ITypeReference *system_uint8_;
            ITypeReference *system_uint16_;
            ITypeReference *system_uint32_;
            ITypeReference *system_uint64_;
            ITypeReference *system_float32_;
            ITypeReference *system_float64_;
            ITypeReference *system_intptr_;
            ITypeReference *system_uintptr_;
            ITypeReference *system_string_;
            ITypeReference *system_object_;
            ITypeReference *system_enum_;
            ITypeReference *system_value_type_;
            ITypeReference *system_array_;
            ITypeReference *system_runtime_field_handle_;
        };
    }
}


#endif
