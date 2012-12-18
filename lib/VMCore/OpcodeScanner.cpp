//
//  OpcodeScanner.cpp
//  silk
//
//  Created by Haohui Mai on 12/8/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#include "OpcodeCompiler.h"
#include "CompilationEngine.h"
#include "VMClass.h"
#include "VMMember.h"

#include "silk/decil/ObjectModel.h"
#include "silk/Support/Util.h"

#include <llvm/Function.h>

#include <iostream>

namespace silk
{
    using namespace decil;
    using namespace llvm;
    
    OpcodeScanner::OpcodeScanner(CompilationEngine *engine,
                                 llvm::BasicBlock *prelude, VMMethod *method,
                                 std::unordered_map<decil::IParameterDefinition *, llvm::Value*> &arguments,
                                 std::unordered_map<int, OpcodeCompiler::BlockInfo> &block_info)
    : engine_(engine)
    , prelude_(prelude)
    , method_(method)
    , arguments_(arguments)
    , block_info_(block_info)
    {}
    
    void OpcodeScanner::Scan()
    {
        bool is_next_inst_a_new_bb = true;
        for (auto it = method_->method_def()->inst_begin(), end = method_->method_def()->inst_end(); it != end; ++it)
            ScanInstruction(*it, &is_next_inst_a_new_bb);
        
        auto it = block_info_.find(0);
        if (it != block_info_.end())
            it->second.bb->setName("entry");
    }

    void OpcodeScanner::ScanInstruction(IOperation *op, bool *is_next_inst_a_new_bb)
    {
        if (*is_next_inst_a_new_bb)
        {
            RecordStartOfBasicBlock(op->offset());
        }
        
        *is_next_inst_a_new_bb = false;
        
        switch (op->opcode())
        {
            case kLdarga_s:
            case kLdarga:
            case kStarg_s:
            case kStarg:
                CopyArgumentIntoMemory(dynamic_cast<IParameterDefinition*>(op->operand().GetMetadata()));
                break;
                
            case kRet:
                *is_next_inst_a_new_bb = true;
                break;
                
            case kBr_s:
            case kBrfalse_s:
            case kBrtrue_s:
            case kBeq_s:
            case kBge_s:
            case kBgt_s:
            case kBle_s:
            case kBlt_s:
            case kBne_un_s:
            case kBge_un_s:
            case kBgt_un_s:
            case kBle_un_s:
            case kBlt_un_s:
            case kBr:
            case kBrfalse:
            case kBrtrue:
            case kBeq:
            case kBge:
            case kBgt:
            case kBle:
            case kBlt:
            case kBne_un:
            case kBge_un:
            case kBgt_un:
            case kBle_un:
            case kBlt_un:
            case kLeave_s:
            case kLeave:
                RecordStartOfBasicBlock((int)op->operand().GetInt());
                *is_next_inst_a_new_bb = true;
                break;

            case kSwitch:
                *is_next_inst_a_new_bb = true;
                for (auto e : op->operand().GetIntArray())
                    RecordStartOfBasicBlock(e);
                break;
                
            default:
                break;
        }
    }
    
    void OpcodeScanner::CopyArgumentIntoMemory(IParameterDefinition *param)
    {
        if (arguments_[param])
            return;
        
        auto vm_type = engine_->GetVMClassForNamedType(param->type()->resolved_type());
        
        auto ptr = new AllocaInst(vm_type->physical_type(), nullptr, "", prelude_);
        arguments_[param] = ptr;
        
        auto f = prelude_->getParent();
        int i = 0;
        Function::arg_iterator AI = f->arg_begin();
        for (auto it = method_->method_def()->param_begin(), end = method_->method_def()->param_end();
             it != end; ++it, ++i, ++AI)
        {
            if (*it == param)
                break;
        }
        
        new StoreInst((Value*)AI, ptr, prelude_);
    }
    
    void OpcodeScanner::RecordStartOfBasicBlock(int pos)
    {
        auto it = block_info_.find(pos);
        if (it != block_info_.end())
            return;
        
        auto f = prelude_->getParent();
        OpcodeCompiler::BlockInfo info;
        info.bb = BasicBlock::Create(f->getContext(), "", f);
        info.stack = new std::vector<OpcodeCompiler::Operand>();
        block_info_.insert(std::make_pair(pos, info));
    }
}

