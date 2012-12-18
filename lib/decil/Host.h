//
//  Host.h
//  silk
//
//  Created by Haohui Mai on 11/25/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#ifndef SILK_LIB_DECIL_HOST_H_
#define SILK_LIB_DECIL_HOST_H_

#include "silk/decil/IHost.h"
#include "silk/decil/Units.h"

#include <llvm/ADT/OwningPtr.h>
#include <llvm/Support/MemoryBuffer.h>

#include <list>
#include <string>
#include <unordered_map>

namespace silk
{
    namespace decil
    {
        class PEFileReader;
        class PlatformType;
        
        class Host : public IHost
        {
        public:
            Host();
            ~Host();
            IAssembly *LoadAssembly(const std::string &file) override final;
            virtual IAssembly *LoadAssembly(const AssemblyIdentity &id) override final;

            virtual void AddClassPath(const std::string &path) override final;

            virtual IPlatformType *platform_type() override final;
            
            virtual IAssembly *get_assembly(int idx) override final;
            virtual size_t assembly_size() const override final;

        private:
            IAssembly *LoadAssemblyFromPath(const std::string &path);
            IAssembly *LookupAssembly(const AssemblyIdentity &id) const;
            
            std::list<std::string> class_paths_;
            std::unordered_map<AssemblyIdentity, IAssembly*> assemblies_;
            std::vector<IAssembly*> loaded_assemblies_;
            PlatformType *platform_type_;
        };
    }
}
#endif

