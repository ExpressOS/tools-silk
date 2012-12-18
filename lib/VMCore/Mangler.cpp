//
//  Mangler.cpp
//  silk
//
//  Created by Haohui Mai on 12/8/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#include "Mangler.h"

#include "silk/decil/ObjectModel.h"

namespace silk
{
    using namespace decil;
    namespace mangler
    {
        std::u16string mangle(decil::IMethodDefinition *v)
        {
            std::u16string s = v->name();
            auto param_start = v->param_begin();
            auto has_implicit_this = v->has_this() && !v->explicit_this();
            if (has_implicit_this)
                ++param_start;
            for (auto it = param_start, end = v->param_end(); it != end; ++it)
            {
                s += u"." + (*it)->type()->resolved_type()->name();
            }
            return s;
        }

    }
}

