//
//  Util.cpp
//  silk
//
//  Created by Haohui Mai on 12/5/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#include "silk/Support/Util.h"

#include <locale>
#include <codecvt>

namespace silk
{
    std::string ToUTF8String(const std::u16string &s)
    {
        if (s.empty())
            return "";
        
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> utf16conv;
        std::string ret = utf16conv.to_bytes(s);
        return ret;
    }

    std::u16string ToUTF16String(const std::string &s)
    {
        if (s.empty())
            return u"";
        
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> utf16conv;
        std::u16string ret = utf16conv.from_bytes(s);
        return ret;
    }

}
