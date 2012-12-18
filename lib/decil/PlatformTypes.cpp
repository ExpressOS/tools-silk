//
//  PlatformTypes.cpp
//  silk
//
//  Created by Haohui Mai on 11/30/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#include "PlatformTypes.h"
#include "BinaryObjectModel.h"

#include "silk/decil/Units.h"
#include "silk/decil/ObjectModel.h"

namespace silk
{
    namespace decil
    {
#define INIT_TYPEREF(name, type_name) \
system_ ## name ## _ = new TypeReference(corelib_reference_, type_name, u"System");

        PlatformType::PlatformType(IHost *host)
        : host_(host)
        , corelib_id_(u"mscorlib", u"en-US", 3, 5, 0, 0)
        {
            corelib_reference_ = new AssemblyReference(host, corelib_id());
            INIT_TYPEREF(void, u"Void")
            INIT_TYPEREF(boolean, u"Boolean")
            INIT_TYPEREF(char, u"Char")
            INIT_TYPEREF(int8, u"SByte")
            INIT_TYPEREF(int16, u"Int16")
            INIT_TYPEREF(int32, u"Int32")
            INIT_TYPEREF(int64, u"Int64")
            INIT_TYPEREF(uint8, u"Byte")
            INIT_TYPEREF(uint16, u"UInt16")
            INIT_TYPEREF(uint32, u"UInt32")
            INIT_TYPEREF(uint64, u"UInt64")
            INIT_TYPEREF(float32, u"Single")
            INIT_TYPEREF(float64, u"Double")
            INIT_TYPEREF(intptr, u"IntPtr")
            INIT_TYPEREF(uintptr, u"UIntPtr")
            INIT_TYPEREF(string, u"String")
            INIT_TYPEREF(object, u"Object")
            INIT_TYPEREF(enum, u"Enum")
            INIT_TYPEREF(value_type, u"ValueType")
            INIT_TYPEREF(array, u"Array")
            INIT_TYPEREF(runtime_field_handle, u"RuntimeFieldHandle")
        }
    }
}

