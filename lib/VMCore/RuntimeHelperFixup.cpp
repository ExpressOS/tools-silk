//
//  RuntimeHelperFixup.cpp
//  silk
//
//  Created by Haohui Mai on 12/14/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#include "VMClass.h"
#include "VMMember.h"

#include "silk/VMCore/VMModel.h"

#include <llvm/Pass.h>
#include <llvm/Module.h>
#include <llvm/Instructions.h>
#include <llvm/Constants.h>
#include <llvm/PassManager.h>
#include <llvm/IntrinsicInst.h>
#include <llvm/IRBuilder.h>

using namespace llvm;

namespace silk
{
    class RuntimeHelperFixupPass : public ModulePass
    {
    public:
        static char ID;
        RuntimeHelperFixupPass(IIntrinsic *intrinsic)
        : ModulePass(ID)
        , intrinsic_(intrinsic)
        { InitStringConstructs(); }
        
        virtual bool runOnModule(Module &M);
        
    private:
        IIntrinsic *intrinsic_;
        std::unordered_map<std::string, std::string> string_constructs_;
        void InitStringConstructs();
        void FixArrayInit(Function *F);
        void FixStringConstructor(Function *old_construct, Function *new_construct);
    };
    
    char RuntimeHelperFixupPass::ID = 0;
    
    bool RuntimeHelperFixupPass::runOnModule(Module &M)
    {
        static const char *array_helper_name = "System.Runtime.CompilerServices.RuntimeHelpers..InitializeArray.System.Array.System.RuntimeFieldHandle";
        bool changed = false;
        
        Function *array_init_func = M.getFunction(array_helper_name);
        if (array_init_func)
        {
            changed = true;
            FixArrayInit(array_init_func);
        }
        
        for (auto e : string_constructs_)
        {
            auto old_func = M.getFunction(e.first);
            auto new_func = M.getFunction(e.second);
            if (old_func && new_func)
                FixStringConstructor(old_func, new_func);
        }
        
        return changed;
    }

    void RuntimeHelperFixupPass::FixArrayInit(Function *F)
    {
        Module *M = F->getParent();
        LLVMContext &ctx = F->getContext();
        
        for (auto it = F->use_begin(), end = F->use_end(); it != end; ++it)
        {
            CallInst *CI = dyn_cast<CallInst>(*it);
            assert (CI && "Unknown usage for array init helper");
            
            Instruction *field_runtime_handle = dyn_cast<Instruction>(CI->getArgOperand(1));
            assert (field_runtime_handle);
            
            MDNode *md = field_runtime_handle->getMetadata("silk_runtime_field_handle");
            assert (md);
            
            ConstantInt *handle = dyn_cast<ConstantInt>(md->getOperand(0));
            assert(handle);
            
            auto vm_field = reinterpret_cast<VMField*>(handle->getValue().getLimitedValue());
            auto field_map = vm_field->field_mapping();
            
            auto vm_named_class = dynamic_cast<VMNamedClass*>(vm_field->type());
            auto size = vm_named_class->type_def()->class_size();

            raw_istream is(field_map.start(), size);

            std::vector<uint8_t> buf;
            buf.resize(size);
            is.read((char*)buf.data(), size);
            
            Constant *predefined_value = ConstantDataArray::get(ctx, buf);
            auto GV = new GlobalVariable(*M, predefined_value->getType(),
                                         true, GlobalValue::InternalLinkage,
                                         predefined_value, ".initdata");
            
            Value *array_ptr = CI->getArgOperand(0);
            IRBuilder<> builder(CI);
            
            auto intrinsic_array_base = intrinsic_->array_base_pointer();
            auto array_ptr_casted = builder.CreateBitCast(array_ptr, builder.getInt8PtrTy());
            auto array_base_ptr = builder.CreateCall(intrinsic_array_base, array_ptr_casted);
            builder.CreateMemCpy(array_base_ptr, GV, ConstantInt::get(Type::getInt64Ty(ctx), size), 0);
            CI->eraseFromParent();
        }
    }
    
    void RuntimeHelperFixupPass::FixStringConstructor(Function *old_construct, Function *new_construct)
    {
        for (auto it = old_construct->use_begin(), end = old_construct->use_end(); it != end; ++it)
        {
            CallInst *CI = dyn_cast<CallInst>(*it);
            assert (CI);
            
            auto num_ops = CI->getNumArgOperands();
            SmallVector<Value*, 4> ops;
            for (size_t i = 1; i < num_ops; ++i)
                ops.push_back(CI->getArgOperand(i));
            
            IRBuilder<> builder(CI);
            auto new_call = builder.CreateCall(new_construct, ops);
            
            BitCastInst *this_ptr = cast<BitCastInst>(CI->getArgOperand(0));
            auto alloc = cast<Instruction>(this_ptr->llvm::User::getOperand(0));
            this_ptr->replaceAllUsesWith(new_call);
            CI->eraseFromParent();
            this_ptr->eraseFromParent();
            alloc->eraseFromParent();
        }
    }
    
    void RuntimeHelperFixupPass::InitStringConstructs()
    {
        string_constructs_["System.String...ctor.System.Char.System.Int32"]
        = "System.String..CreateString.System.Char.System.Int32";
        string_constructs_["System.String...ctor.System.Char[]"]
        = "System.String..CreateString.System.Char[]";
        string_constructs_["System.String...ctor.System.Char[].System.Int32.System.Int32"]
        = "System.String..CreateString.System.Char[].System.Int32.System.Int32";
        string_constructs_["System.String...ctor.System.SByte*.System.Int32.System.Int32"]
        = "System.String..CreateString.System.SByte*.System.Int32.System.Int32";
    }

    Pass *CreateRuntimeHelperFixupPass(IIntrinsic *intrinsic)
    {
        return new RuntimeHelperFixupPass(intrinsic);
    }
}
