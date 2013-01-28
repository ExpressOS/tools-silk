//
//  Util.cpp
//  silk
//
//  Created by Haohui Mai on 12/5/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#include "silk/Support/Util.h"

#include <locale>
// #include <codecvt>

// Ideally it should be done through wstring_convert
// but gcc does not have the header yet.
//
// the following code only works if the characters
// are in ASCII.
//
namespace silk
{
    std::string ToUTF8String(const std::u16string &s)
    {
        auto l = s.length();
        std::string ret(l, '\0');
        for (size_t i = 0; i < l; ++i)
            ret[i] = s[i];
#if 0
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> utf16conv;
        std::string ret = utf16conv.to_bytes(s);
#endif
        return ret;
    }

    std::u16string ToUTF16String(const std::string &s)
    {
        auto l = s.length();
        std::u16string ret(l, '\0');
        for (size_t i = 0; i < l; ++i)
            ret[i] = s[i];

#if 0
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> utf16conv;
        std::u16string ret = utf16conv.from_bytes(s);
#endif
        return ret;
    }

}
