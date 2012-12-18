//
//  Units.h
//  silk
//
//  Created by Haohui Mai on 11/25/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#ifndef SILK_DECIL_UNITS_H_
#define SILK_DECIL_UNITS_H_

#include "silk/Support/raw_istream.h"

#include <string>
#include <vector>

namespace silk
{
    namespace decil
    {
        class ITypeReference;
        
        class IMetadata
        {
        public:
            virtual ~IMetadata();
        };
        
        class AssemblyIdentity
        {
        public:
            AssemblyIdentity(const std::u16string &name, const std::u16string &culture,
                             uint16_t major_version, uint16_t minor_version,
                             uint16_t build_number, uint16_t revision_number);
            
            bool operator==(const AssemblyIdentity &rhs) const;
            bool operator!=(const AssemblyIdentity &rhs) const
            { return !(*this == rhs); }
            size_t hash() const;
            const std::u16string &name() const
            { return name_; }
        private:
            std::u16string name_;
            std::u16string culture_;
            uint32_t version_;
            uint32_t build_;
        };
        
        class IPlatformType : public IMetadata
        {
        public:
            virtual const AssemblyIdentity &corelib_id() const = 0;
            virtual ITypeReference *system_void() = 0;
            virtual ITypeReference *system_boolean() = 0;
            virtual ITypeReference *system_char() = 0;
            virtual ITypeReference *system_int8() = 0;
            virtual ITypeReference *system_int16() = 0;
            virtual ITypeReference *system_int32() = 0;
            virtual ITypeReference *system_int64() = 0;
            virtual ITypeReference *system_uint8() = 0;
            virtual ITypeReference *system_uint16() = 0;
            virtual ITypeReference *system_uint32() = 0;
            virtual ITypeReference *system_uint64() = 0;
            virtual ITypeReference *system_float32() = 0;
            virtual ITypeReference *system_float64() = 0;
            virtual ITypeReference *system_intptr() = 0;
            virtual ITypeReference *system_uintptr() = 0;
            virtual ITypeReference *system_string() = 0;
            virtual ITypeReference *system_object() = 0;
            virtual ITypeReference *system_enum() = 0;
            virtual ITypeReference *system_value_type() = 0;
            virtual ITypeReference *system_array() = 0;
            virtual ITypeReference *system_runtime_field_handle() = 0;
        };
    }
}

namespace std
{
    template <>
    struct hash<silk::decil::AssemblyIdentity>
    {
        size_t operator()(const silk::decil::AssemblyIdentity &v) const { return v.hash(); }
    };
}
#endif
