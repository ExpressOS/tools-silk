//
//  VMMember.cpp
//  silk
//
//  Created by Haohui Mai on 12/5/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#include "VMMember.h"
#include "VMClass.h"

#include <algorithm>

namespace silk
{
    using namespace decil;
    
    VMMember::~VMMember()
    {}
    
    VMMember::VMMember(const std::u16string &name)
    : name_(name)
    {}
    
    VMField::VMField(IFieldDefinition *def, VMClass *type, unsigned offset)
    : VMMember(def->name())
    , def_(def)
    , type_(type)
    , offset_(offset)
    {}

    VMMethod::VMMethod(IMethodDefinition *def)
    : VMMember(def->name())
    , implementation_(nullptr)
    , def_(def)
    , return_type_(nullptr)
    {}
   
    const std::u16string &VMMethod::mangled_name()
    {
        // XXX: Skiping the this argument when mangling the name of the method.
        // That way the it matches the mangle schemes of IMethodReference.
        if (mangled_name_.empty())
        {
            mangled_name_ = name_;
            auto it = has_implicit_this() ? ++params_.begin() : params_.begin();
            std::for_each(it, params_.end(),
                          [&](const ParamInfo &p) { mangled_name_ += u"." + p.type()->name(); });
        }
        return mangled_name_;
    }
    
    ParamInfo::ParamInfo(const std::u16string &name, VMClass *type, IParameterDefinition *def)
    : name_(name)
    , def_(def)
    , type_(type)
    {}
    
   
}
