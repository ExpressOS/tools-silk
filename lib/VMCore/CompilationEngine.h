//
//  CompilationEngine.h
//  silk
//
//  Created by Haohui Mai on 12/5/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#ifndef SILK_LIB_VMCORE_COMPILATION_ENGINE_H_
#define SILK_LIB_VMCORE_COMPILATION_ENGINE_H_

#include "silk/VMCore/VMModel.h"
#include "silk/decil/IHost.h"
#include "silk/decil/ObjectModel.h"

#include <unordered_map>

namespace llvm
{
    class Value;
    class Module;
}

namespace silk
{
    class VMClass;
    class VMNamedClass;

    class CompilationEngine : public ICompilationEngine
    {
    public:
        CompilationEngine(decil::IHost *host, const std::string &triple);
        virtual void Compile() override final;
        virtual llvm::Module *module() override final
        { return module_; }
        virtual decil::IHost *host() override final
        { return host_; }
        virtual IIntrinsic *intrinsic() const override final
        { return intrinsic_; }
        virtual void set_intrinsic(IIntrinsic *intrinsic) override final
        { intrinsic_ = intrinsic; }
        
        VMClass *GetVMClassForNamedType(decil::ITypeDefinition *def);
        VMClass *GetPointerType(VMClass *target_type);
        llvm::Value *GetOrCreateString(const std::u16string &str);
        decil::INamedTypeDefinition::TypeCode NativeIntTypeCode() const;
        decil::INamedTypeDefinition::TypeCode NativeUIntTypeCode() const;

    private:
        void Layout(decil::IAssembly *assembly);
        void GenerateCode(decil::IAssembly *assembly);
        
        std::unordered_map<decil::ITypeDefinition *, VMClass *> vm_types_;
        std::unordered_map<decil::ITypeDefinition *, VMClass *> pointer_type_cache_;
        std::unordered_map<decil::ITypeDefinition *, VMClass *> vector_type_cache_;
        std::unordered_map<std::u16string, llvm::Value *> string_cache_;
        std::unordered_map<VMClass *, VMClass *> vm_pointer_type_cache_;

        decil::IHost *host_;
        llvm::Module *module_;
        IIntrinsic *intrinsic_;
    };
}

#endif
