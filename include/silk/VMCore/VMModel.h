//
//  VMModel.h
//  silk
//
//  Created by Haohui Mai on 12/2/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#ifndef SILK_VMCORE_VMMODEL_H_
#define SILK_VMCORE_VMMODEL_H_

#include <string>

namespace llvm
{
    class Module;
    class Function;
}

namespace silk
{
    namespace decil
    {
        class IHost;
    }
    
    class IIntrinsic;
    class ICompilationEngine
    {
    public:
        virtual ~ICompilationEngine();
        virtual void Compile() = 0;
        virtual llvm::Module *module() = 0;
        virtual decil::IHost *host() = 0;
        virtual IIntrinsic *intrinsic() const = 0;
        virtual void set_intrinsic(IIntrinsic *intrinsic) = 0;
    };
    
    class IIntrinsic
    {
    public:
        virtual ~IIntrinsic();
        virtual llvm::Function *new_object() const = 0;
        virtual llvm::Function *new_array() const = 0;
        virtual llvm::Function *array_base_pointer() const = 0;
    };
    
    ICompilationEngine *CreateCompilationEngine(decil::IHost *host, const std::string &triple);
    IIntrinsic *CreateAOTIntrinsic(llvm::Module *module);
}

#endif
