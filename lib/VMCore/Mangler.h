//
//  Mangler.h
//  silk
//
//  Created by Haohui Mai on 12/8/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#ifndef SILK_LIB_VMCORE_MANGLER_H_
#define SILK_LIB_VMCORE_MANGLER_H_

#include <string>

namespace silk
{
    namespace decil
    {
        class IMethodDefinition;
    }
    
    namespace mangler
    {
        std::u16string mangle(decil::IMethodDefinition *v);
    }
}

#endif

