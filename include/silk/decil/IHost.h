//
//  Host.h
//  silk
//
//  Created by Haohui Mai on 11/24/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#ifndef SILK_DECIL_HOST_H_
#define SILK_DECIL_HOST_H_

#include "silk/Support/ErrorHandler.h"
#include <string>

namespace silk
{
    namespace decil
    {
        class IAssembly;
        class IPlatformType;
        class AssemblyIdentity;

        class IHost
        {
        public:
            virtual ~IHost();
            virtual IAssembly *LoadAssembly(const std::string &file) = 0;
            virtual IAssembly *LoadAssembly(const AssemblyIdentity &id) = 0;
            virtual IPlatformType *platform_type() = 0;
            
            virtual void AddClassPath(const std::string &path) = 0;

            ErrorHandler &error_handler() { return error_handler_; }
            virtual IAssembly *get_assembly(int idx) = 0;
            virtual size_t assembly_size() const = 0;
        protected:
            ErrorHandler error_handler_;
        };
        
        IHost *CreateDefaultHost(void);
    }
}

#endif
