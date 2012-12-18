//
//  ErrorHandler.h
//  silk
//
//  Created by Haohui Mai on 11/24/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#ifndef SILK_SUPPORT_ERROR_HANDLER_H_
#define SILK_SUPPORT_ERROR_HANDLER_H_

namespace silk
{
    class ErrorHandler
    {
    public:
        ErrorHandler();
        bool has_error() const { return has_error_; }
        void set_error() { has_error_ = true; }
        void clear_error() { has_error_ = false; }
        
        void __attribute__((format(printf, 2, 3))) Info(const char *fmt, ...);
        void __attribute__((format(printf, 2, 3))) Warn(const char *fmt, ...);
        void __attribute__((format(printf, 2, 3))) Error(const char *fmt, ...);
    private:
        int has_error_;        
    };
}

#endif
