//
//  Units.cpp
//  silk
//
//  Created by Haohui Mai on 11/26/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#include "silk/decil/ObjectModel.h"

namespace silk
{
    namespace decil
    {
        // FIXME: the culture field should be case-insensitive.
        AssemblyIdentity::AssemblyIdentity(const std::u16string &name, const std::u16string &culture,
                                           uint16_t major_version, uint16_t minor_version,
                                           uint16_t build_number, uint16_t revision_number)
        : name_(name)
        , culture_(culture)
        , version_((major_version << 16) + minor_version)
        , build_((build_number << 16) + revision_number)
        {
            std::transform(culture_.begin(), culture_.end(), culture_.begin(), ::tolower);
        }
        
        bool AssemblyIdentity::operator==(const AssemblyIdentity &rhs) const
        {
            return name_ == rhs.name_ && culture_ == rhs.culture_
            && version_ == rhs.version_ && build_ == rhs.build_;
        }
        
        size_t AssemblyIdentity::hash() const
        {
            std::hash<std::u16string> str_hash;
            return str_hash(name_) ^ str_hash(culture_) ^ version_ ^ build_;
        }
        
        IMetadata::~IMetadata()
        {}
    }
}

