//
//  Util.h
//  silk
//
//  Created by Haohui Mai on 12/5/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#ifndef SILK_SUPPORT_UTIL_H_
#define SILK_SUPPORT_UTIL_H_

#include <string>

namespace silk
{
    std::string ToUTF8String(const std::u16string &s);
    std::u16string ToUTF16String(const std::string &s);
}



#endif
