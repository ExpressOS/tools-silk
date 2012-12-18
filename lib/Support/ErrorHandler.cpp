//
//  ErrorHandler.cpp
//  silk
//
//  Created by Haohui Mai on 9/19/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#include "silk/Support/ErrorHandler.h"

#include <cstdarg>
#include <cstdio>

namespace silk {
    ErrorHandler::ErrorHandler()
    : has_error_(0)
    {}
    
    static void PrintMessage(const char *type, const char *fmt, va_list args)
    {
        fprintf(stderr, "\t%s: ", type);
        vfprintf(stderr, fmt, args);
        fprintf(stderr, "\n");
    }

    void ErrorHandler::Info(const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        PrintMessage("info", fmt, args);
        va_end(args);
    }
    
    void ErrorHandler::Warn(const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        PrintMessage("warning", fmt, args);
        va_end(args);
    }
    
    void ErrorHandler::Error(const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        PrintMessage("error", fmt, args);
        va_end(args);
        set_error();
    }
}

