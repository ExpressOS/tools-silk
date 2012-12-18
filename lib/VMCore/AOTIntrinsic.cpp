//
//  AOTIntrinsic.cpp
//  silk
//
//  Created by Haohui Mai on 12/11/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#include "silk/VMCore/VMModel.h"

#include <llvm/Module.h>
#include <llvm/DerivedTypes.h>

namespace silk
{
    using namespace llvm;
    
    class AOTIntrinsic : public IIntrinsic
    {
    public:
        AOTIntrinsic(Module *module);
        virtual Function *new_object() const override final
        { return new_object_; }
        virtual Function *new_array() const override final
        { return new_array_; }
        virtual Function *array_base_pointer() const override final
        { return array_base_pointer_; }

    private:
        Module *module_;
        Function *new_object_;
        Function *new_array_;
        Function *array_base_pointer_;
    };
    
    IIntrinsic::~IIntrinsic()
    {}
    
    AOTIntrinsic::AOTIntrinsic(Module *module)
    : module_(module)
    {
        auto &c = module->getContext();
        Type *new_object_params[] = { Type::getInt32Ty(c) };
        new_object_ = Function::Create(FunctionType::get(Type::getInt8PtrTy(c), new_object_params, false),
                                       GlobalValue::ExternalLinkage, "__silk_rt_new_object", module);
        
        Type *new_array_params[] = { Type::getInt32Ty(c), Type::getInt32Ty(c) };
        new_array_ = Function::Create(FunctionType::get(Type::getInt8PtrTy(c), new_array_params, false),
                                     GlobalValue::ExternalLinkage, "__silk_rt_new_array", module);

        Type *array_base_ptr_params[] = { Type::getInt8PtrTy(c) };
        array_base_pointer_ = Function::Create(FunctionType::get(Type::getInt8PtrTy(c), array_base_ptr_params, false),
                                       GlobalValue::ExternalLinkage, "__silk_rt_array_base_ptr", module);

    }
    
    IIntrinsic *CreateAOTIntrinsic(Module *m)
    { return new AOTIntrinsic(m); }
}
