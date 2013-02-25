//
//  OpcodeCompiler.cpp
//  silk
//
//  Created by Haohui Mai on 12/8/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#include "OpcodeCompiler.h"
#include "CompilationEngine.h"
#include "VMMember.h"
#include "VMClass.h"
#include "Mangler.h"

#include "silk/decil/ObjectModel.h"
#include "silk/decil/Units.h"
#include "silk/Support/Util.h"

#include <llvm/Module.h>
#include <llvm/DataLayout.h>

#include <iostream>

namespace silk
{
    using namespace decil;
    using namespace llvm;
    
    OpcodeCompiler::OpcodeCompiler(CompilationEngine *engine, VMMethod *method)
    : stack_(nullptr)
    , engine_(engine)
    , method_(method)
    , current_function_(method->implementation())
    , prelude_bb_(nullptr)
    , current_bb_(nullptr)
    , ctx_(engine->module()->getContext())
    , builder_(engine->module()->getContext())
    {}
    
    OpcodeCompiler::Operand::Operand()
    : value(nullptr)
    , type(nullptr)
    {}
    
    OpcodeCompiler::Operand::Operand(Value *v, VMClass *t)
    : value(v)
    , type(t)
    {}
    
    OpcodeCompiler::Operand OpcodeCompiler::Pop()
    {
        assert (stack_->size());
        Operand r = stack_->back();
        stack_->pop_back();
        return r;
    }
    
    Value *OpcodeCompiler::EnsureCorrectType(const OpcodeCompiler::Operand &op, VMClass *target_type)
    {
        if (target_type->normal_type() == op.value->getType())
            return op.value;
        
        auto desired_llvm_type = target_type->normal_type();
        auto op_type = op.value->getType();
        
        // Cast pointer to appropriate types for inheritances
        // or for strings
        if (desired_llvm_type != op_type && desired_llvm_type->isPointerTy() && op_type->isPointerTy())
        {
            return builder_.CreateBitCast(op.value, desired_llvm_type);
        }
        
        return op.value;
    }
    
    //
    // Table III.9: Signature Matching
    //
    Value *OpcodeCompiler::EnsureSignatureMatching(const OpcodeCompiler::Operand &op, VMClass *dst_clazz)
    {
        auto dst_ty = dst_clazz->normal_type();
        auto src = op.value;
        auto src_ty = src->getType();

        if (src_ty == dst_ty)
            return src;
        
        auto is_dst_unsigned = IsUnsignedIntVMClass(dst_clazz);

        if (src_ty->isFloatingPointTy())
        {
            if (dst_ty->isIntegerTy())
                return CreateFPToInt(src, dst_ty, is_dst_unsigned);
            
            if (dst_ty->isFloatingPointTy())
                return CreateFPTruncOrExt(src, dst_ty);
        }
        else if (src_ty->isIntegerTy())
        {
            if (dst_ty->isIntegerTy())
                return CreateIntTruncOrExt(src, dst_ty, is_dst_unsigned);
            
            if (dst_ty->isPointerTy())
                return CreateIntPtrOrBitCast(src, dst_ty);
        }
        else if (src_ty->isPointerTy())
        {
            if (dst_ty->isIntegerTy())
            {
                auto ptr_to_int = builder_.CreatePtrToInt(src, dst_ty);
                return ptr_to_int;
            }
        }
        return src;
    }

    void OpcodeCompiler::EnsureBinaryOperatorType(Operand &lhs, Operand &rhs)
    {
        auto lhs_ty = lhs.value->getType();
        auto rhs_ty = rhs.value->getType();
        
        if (lhs_ty == rhs_ty)
            return;
        
        auto is_lhs_unsigned = IsUnsignedIntVMClass(lhs.type);
        auto is_rhs_unsigned = IsUnsignedIntVMClass(rhs.type);
        
        if (lhs_ty->isIntegerTy() && rhs_ty->isIntegerTy())
        {
            auto lhs_int_ty = cast<IntegerType>(lhs_ty);
            auto rhs_int_ty = cast<IntegerType>(rhs_ty);
            if (lhs_int_ty->getBitWidth() > rhs_int_ty->getBitWidth())
            {
                rhs.value = CreateIntTruncOrExt(rhs.value, lhs_int_ty, is_rhs_unsigned);
            }
            else
            {
                lhs.value = CreateIntTruncOrExt(lhs.value, rhs_int_ty, is_lhs_unsigned);
            }
            return;
        }
    }
        
    Value *OpcodeCompiler::CreateIntPtrOrBitCast(Value *ptr, Type *pointer_type)
    {
        if (ptr->getType()->isIntegerTy())
        {
            return builder_.CreateIntToPtr(ptr, pointer_type);
        }
        else
        {
            return builder_.CreateBitCast(ptr, pointer_type);
        }
    }

    Value *OpcodeCompiler::CreateFPTruncOrExt(Value *src, Type *dst_ty)
    {
        auto src_ty = src->getType();
        assert (dst_ty->isFloatingPointTy() && src_ty->isFloatingPointTy());
        if (src_ty->isFloatTy() && dst_ty->isDoubleTy())
        {
            return builder_.CreateFPExt(src, dst_ty);
        }
        if (src_ty->isDoubleTy() && dst_ty->isFloatTy())
        {
            return builder_.CreateFPTrunc(src, dst_ty);
        }
        return src;
    }
    
    Value *OpcodeCompiler::CreateIntTruncOrExt(Value *src, Type *dst_type, bool is_unsigned)
    {
        assert (src->getType()->isIntegerTy() && dst_type->isIntegerTy());
        
        auto src_int_ty = cast<IntegerType>(src->getType());
        auto dst_int_ty = cast<IntegerType>(dst_type);
        
        if (src_int_ty->getBitWidth() > dst_int_ty->getBitWidth())
        {
            return builder_.CreateTrunc(src, dst_type);
        }
        else if (src_int_ty->getBitWidth() < dst_int_ty->getBitWidth())
        {
            return is_unsigned ? builder_.CreateZExt(src, dst_type) : builder_.CreateSExt(src, dst_type);
        }
        return src;
    }
    
    Value *OpcodeCompiler::CreateFPToInt(Value *src, Type *dst_type, bool is_unsigned)
    {
        assert (src->getType()->isFloatingPointTy() && dst_type->isIntegerTy());
        return is_unsigned ? builder_.CreateFPToUI(src, dst_type) : builder_.CreateFPToSI(src, dst_type);
    }
    
    Value * OpcodeCompiler::CreateArrayGEP(Value *array, Value *index, Type *element_type)
    {
        auto i8_ptr = builder_.CreateBitCast(array, builder_.getInt8PtrTy());
        auto intrinsic_base_ptr = engine_->intrinsic()->array_base_pointer();
        auto base_ptr_i8 = builder_.CreateCall(intrinsic_base_ptr, i8_ptr);
        auto base_ptr = builder_.CreateBitCast(base_ptr_i8, PointerType::getUnqual(element_type));
        auto idx = CreateIntTruncOrExt(index, builder_.getInt32Ty(), false);
        auto v = builder_.CreateGEP(base_ptr, idx);
        return v;
    }
    
    void OpcodeCompiler::Compile()
    {
        prelude_bb_ = BasicBlock::Create(ctx_, "prelude", current_function_);
        builder_.SetInsertPoint(prelude_bb_);
        DeclareLocalVariables();
        
        OpcodeScanner scanner(engine_, prelude_bb_, method_, arguments_, block_info_);
        scanner.Scan();
        
        current_bb_ = prelude_bb_;

        
        for (auto it = method_->method_def()->inst_begin(), end = method_->method_def()->inst_end();
             it != end; ++it)
        {
            auto it2 = block_info_.find((*it)->offset());
            if (it2 != block_info_.end())
            {
                if (!current_bb_->getTerminator())
                    VisitBr((*it)->offset());
                
                current_bb_ = it2->second.bb;
                builder_.SetInsertPoint(current_bb_);
                stack_ = it2->second.stack;
            }

            CompileInstruction(*it);
        }
    }
    
    void OpcodeCompiler::DeclareLocalVariables()
    {
        for (auto it = method_->method_def()->local_begin(), end = method_->method_def()->local_end(); it != end; ++it)
        {
            auto e = *it;
            auto vm_type = engine_->GetVMClassForNamedType(e->type()->resolved_type());
            auto alloca = builder_.CreateAlloca(vm_type->normal_type());
            locals_.insert(std::make_pair(e, Operand(alloca, vm_type)));
        }
    }
    
    void OpcodeCompiler::SwitchBasicBlock(OpcodeCompiler::BlockInfo *bi)
    {
        current_bb_ = bi->bb;
        builder_.CreateBr(current_bb_);
        builder_.SetInsertPoint(current_bb_);
        stack_ = bi->stack;
    }
    
    void OpcodeCompiler::MergeCurrentStackInto(const BlockInfo *bi)
    {
        if (!stack_)
            return;
        
        if (bi->stack->empty())
        {
            bi->stack->reserve(stack_->size());
            for (auto &op : *stack_)
            {
                auto v = op.value;
                auto is_op_unsigned = IsUnsignedIntVMClass(op.type);
                if (v->getType()->isIntegerTy())
                {
                    v = CreateIntTruncOrExt(v, builder_.getInt32Ty(), is_op_unsigned);
                }
                else if (v->getType()->isPointerTy())
                {
                    v = builder_.CreateBitCast(v, builder_.getInt8PtrTy());
                }
                
                auto phi = PHINode::Create(v->getType(), 2, "", bi->bb);
                phi->addIncoming(v, current_bb_);
                bi->stack->push_back(Operand(phi, op.type));
            }
        }
        else
        {
            assert(stack_->size() <= bi->stack->size());
            for (size_t i = 0; i < stack_->size(); ++i)
            {
                Operand &r = bi->stack->at(i);
                Operand op = stack_->at(i);
                auto is_op_unsigned = IsUnsignedIntVMClass(op.type);
                assert (isa<PHINode>(r.value));
                auto phi = cast<PHINode>(r.value);
                auto v = stack_->at(i).value;
//                phi = EnsurePHINodeHasCorrectType(r, op);

                if (isa<Constant>(v) && cast<Constant>(v)->isNullValue())
                    v = Constant::getNullValue(r.value->getType());

                if (v->getType()->isIntegerTy())
                {
                    v = CreateIntTruncOrExt(v, builder_.getInt32Ty(), is_op_unsigned);
                }
                else if (v->getType()->isPointerTy())
                {
                    v = builder_.CreateBitCast(v, builder_.getInt8PtrTy());
                }
                
                phi->addIncoming(v, current_bb_);

            }
        }
    }
    
    void OpcodeCompiler::CompileInstruction(IOperation *op)
    {
        switch (op->opcode())
        {
            case kNop:
            case kBreak:
                break;
                
            case kLdarg_0:
            case kLdarg_1:
            case kLdarg_2:
            case kLdarg_3:
            case kLdarg_s:
            case kLdarg:
                VisitLdarg(dynamic_cast<IParameterDefinition*>(op->operand().GetMetadata()));
                break;

            case kLdloc_0:
            case kLdloc_1:
            case kLdloc_2:
            case kLdloc_3:
            case kLdloc_s:
            case kLdloc:
                VisitLdloc(dynamic_cast<ILocalDefinition*>(op->operand().GetMetadata()));
                break;

            case kLdloca_s:
            case kLdloca:
                VisitLdloca(dynamic_cast<ILocalDefinition*>(op->operand().GetMetadata()));
                break;

            case kStloc_0:
            case kStloc_1:
            case kStloc_2:
            case kStloc_3:
            case kStloc_s:
            case kStloc:
                VisitStloc(dynamic_cast<ILocalDefinition*>(op->operand().GetMetadata()));
                break;
                

            case kLdarga_s:
            case kLdarga:
                VisitLdarga(dynamic_cast<IParameterDefinition*>(op->operand().GetMetadata()));
                break;
                
            case kStarg_s:
            case kStarg:
                VisitStarg(dynamic_cast<IParameterDefinition*>(op->operand().GetMetadata()));
                break;

            case kLdnull:
                VisitLdnull();
                break;
                
            case kLdc_i4_m1:
            case kLdc_i4_0:
            case kLdc_i4_1:
            case kLdc_i4_2:
            case kLdc_i4_3:
            case kLdc_i4_4:
            case kLdc_i4_5:
            case kLdc_i4_6:
            case kLdc_i4_7:
            case kLdc_i4_8:
            case kLdc_i4_s:
            case kLdc_i4:
                VisitLdcI4((int)op->operand().GetInt());
                break;
            case kLdc_i8:
                VisitLdcI8(op->operand().GetInt());
                break;
            case kLdc_r4:
                VisitLdcR4(op->operand().GetFloat());
                break;
            case kLdc_r8:
                VisitLdcR8(op->operand().GetDouble());
                break;
            case kDup:
                VisitDup();
                break;
            case kPop:
                VisitPop();
                break;
//            case kJmp:
//                is >> i32;
//                operand.SetInt(i32);
//                break;
            case kCall:
                VisitCall(dynamic_cast<IMethodReference*>(op->operand().GetMetadata()));
                break;
//                //                    case kCalli:
//                //                        value = this.GetFunctionPointerType(memReader.ReadUInt32());
//                //                        break;
            case kRet:
                VisitRet();
                break;
                
            case kBr_s:
            case kBr:
                VisitBr((int)op->operand().GetInt());
                break;
                
            case kBrfalse_s:
                VisitBrTF(op->offset() + 2, (int)op->operand().GetInt(), false);
                break;

            case kBrtrue_s:
                VisitBrTF(op->offset() + 2, (int)op->operand().GetInt(), true);
                break;
                
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
                VisitCompareAndBranch(op->opcode(), op->offset() + 2, (int)op->operand().GetInt());
                break;

            case kBrfalse:
                VisitBrTF(op->offset() + 5, (int)op->operand().GetInt(), false);
                break;
                
            case kBrtrue:
                VisitBrTF(op->offset() + 5, (int)op->operand().GetInt(), true);
                break;

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
                VisitCompareAndBranch(op->opcode(), op->offset() + 5, (int)op->operand().GetInt());
                break;

            case kSwitch:
                VisitSwitch(op->operand().GetIntArray(), op->offset());
                break;
            case kLdind_i1:
            case kLdind_u1:
            case kLdind_i2:
            case kLdind_u2:
            case kLdind_i4:
            case kLdind_u4:
            case kLdind_i8:
            case kLdind_i:
            case kLdind_r4:
            case kLdind_r8:
            case kLdind_ref:
                VisitLdind(op->opcode());
                break;
                
            case kStind_ref:
            case kStind_i1:
            case kStind_i2:
            case kStind_i4:
            case kStind_i8:
            case kStind_r4:
            case kStind_r8:
            case kStind_i:
                VisitStind(op->opcode());
                break;
                
            case kAdd:
            case kSub:
            case kMul:
            case kDiv:
            case kDiv_un:
            case kRem:
            case kRem_un:
            case kAnd:
            case kOr:
            case kXor:
            case kShl:
            case kShr:
            case kShr_un:
                VisitBinaryOperator(op->opcode());
                break;
            case kNeg:
                VisitNeg();
                break;
            case kNot:
                VisitNot();
                break;
                
            case kConv_i1:
            case kConv_i2:
            case kConv_i4:
            case kConv_i8:
            case kConv_r4:
            case kConv_r8:
            case kConv_r_un:
            case kConv_u1:
            case kConv_u2:
            case kConv_u4:
            case kConv_u8:
            case kConv_i:
            case kConv_u:
            case kConv_ovf_i1_un:
            case kConv_ovf_i2_un:
            case kConv_ovf_i4_un:
            case kConv_ovf_i8_un:
            case kConv_ovf_u1_un:
            case kConv_ovf_u2_un:
            case kConv_ovf_u4_un:
            case kConv_ovf_u8_un:
            case kConv_ovf_i_un:
            case kConv_ovf_u_un:
            case kConv_ovf_i1:
            case kConv_ovf_u1:
            case kConv_ovf_i2:
            case kConv_ovf_u2:
            case kConv_ovf_i4:
            case kConv_ovf_u4:
            case kConv_ovf_i8:
            case kConv_ovf_u8:
            case kConv_ovf_i:
            case kConv_ovf_u:
                VisitConversion(op->opcode());
                break;
                
            // XXX: Implement virtual calls
            case kCallvirt:
                VisitCall(dynamic_cast<IMethodReference*>(op->operand().GetMetadata()));
                break;
//
//            case kCpobj:
            case kLdobj:
                VisitLdobj(dynamic_cast<ITypeReference*>(op->operand().GetMetadata()));
                break;
            case kLdstr:
                VisitLdstr(op->operand().GetString());
                break;
            case kNewobj:
                VisitNewObj(dynamic_cast<IMethodReference*>(op->operand().GetMetadata()));
                break;
            case kCastclass:
                VisitCastClass(dynamic_cast<ITypeReference*>(op->operand().GetMetadata()));
                break;
            case kIsinst:
                VisitIsInst(dynamic_cast<ITypeReference*>(op->operand().GetMetadata()));
                break;
//            case kUnbox:
//                is >> token;
//                operand.SetMetadata(GetType(token));
//                break;
//            case kThrow:
//                break;
            case kLdfld:
            case kLdsfld:
                VisitLoadField(dynamic_cast<IFieldReference*>(op->operand().GetMetadata()));
                break;
            case kLdflda:
            case kLdsflda:
                VisitLoadFieldAddress(dynamic_cast<IFieldReference*>(op->operand().GetMetadata()));
                break;
            case kStfld:
            case kStsfld:
                VisitStoreField(dynamic_cast<IFieldReference*>(op->operand().GetMetadata()));
                break;
            case kStobj:
                VisitStobj(dynamic_cast<ITypeReference*>(op->operand().GetMetadata()));
                break;

            case kBox:
                VisitBox(dynamic_cast<ITypeReference*>(op->operand().GetMetadata()));
                break;

            case kNewarr:
                VisitNewarr(dynamic_cast<ITypeReference*>(op->operand().GetMetadata()));
                break;
            case kLdlen:
                VisitLdlen();
                break;
            case kLdelema:
                VisitLdelema(dynamic_cast<ITypeReference*>(op->operand().GetMetadata()));
                break;
            case kLdelem_i1:
            case kLdelem_u1:
            case kLdelem_i2:
            case kLdelem_u2:
            case kLdelem_i4:
            case kLdelem_u4:
            case kLdelem_i8:
            case kLdelem_i:
            case kLdelem_r4:
            case kLdelem_r8:
            case kLdelem_ref:
                VisitLdelem(op->opcode());
                break;
            case kStelem_i:
            case kStelem_i1:
            case kStelem_i2:
            case kStelem_i4:
            case kStelem_i8:
            case kStelem_r4:
            case kStelem_r8:
            case kStelem_ref:
                VisitStelem(op->opcode());
                break;
            case kLdelem:
                VisitLdelem(dynamic_cast<ITypeReference*>(op->operand().GetMetadata()));
                break;
            case kStelem:
                VisitStelem(dynamic_cast<ITypeReference*>(op->operand().GetMetadata()));
                break;

            case kUnbox_any:
                VisitUnboxAny(dynamic_cast<ITypeReference*>(op->operand().GetMetadata()));
                break;
//            case kRefanyval:
//                is >> token;
//                operand.SetMetadata(GetType(token));
//                break;
//            case kCkfinite:
//                break;
//            case kMkrefany:
//                is >> token;
//                operand.SetMetadata(GetType(token));
//                break;
            case kLdtoken:
                VisitLdtoken(op->operand().GetMetadata());
                break;
//            case kAdd_ovf:
//            case kAdd_ovf_un:
//            case kMul_ovf:
//            case kMul_ovf_un:
//            case kSub_ovf:
//            case kSub_ovf_un:
//            case kEndfinally:
//                break;
            case kLeave:
            case kLeave_s:
                VisitLeave((int)op->operand().GetInt());
                break;
//            case kArglist:
            case kCeq:
            case kCgt:
            case kCgt_un:
            case kClt:
            case kClt_un:
                VisitCompare(op->opcode());
                break;
//            case kLdftn:
//            case kLdvirtftn:
//                is >> token;
//                operand.SetMetadata(GetMethod(token));
//                break;
//            case kLocalloc:
//                break;
//            case kEndfilter:
//                break;
//            case kUnaligned_:
//                is >> i8;
//                operand.SetInt(i8);
//                break;
//            case kVolatile_:
//            case kTail_:
//                break;
            case kInitobj:
                VisitInitObj(dynamic_cast<ITypeReference*>(op->operand().GetMetadata()));
                break;
//            case kConstrained_:
//                is >> token;
//                operand.SetMetadata(GetType(token));
//                break;
//            case kCpblk:
//            case kInitblk:
//                break;
//            case kNo_:
//                is >> i8;
//                operand.SetInt(i8);
//                break;
//            case kRethrow:
//                break;
            case kSizeof:
                VisitSizeof(dynamic_cast<ITypeReference*>(op->operand().GetMetadata()));
                break;
//            case kRefanytype:
//            case kReadonly_:
//                break;
            default:
                assert (0 && "Unimplemented opcode");
                break;
            }
    }
    
    void OpcodeCompiler::VisitLdarg(IParameterDefinition *param)
    {
        Operand r;
        auto it = arguments_.find(param);
        if (it == arguments_.end())
        {
            auto AI = current_function_->arg_begin();
            for (auto it2 = method_->method_def()->param_begin(), end2 = method_->method_def()->param_end();
                 it2 != end2; ++it2, ++AI)
            {
                if (*it2 == param)
                    break;
            }
            r.value = AI;
        }
        else
        {
            r.value = builder_.CreateLoad(GetArgumentAddress(param));
        }
        
        auto vm_type_it = std::find_if(method_->param_begin(), method_->param_end(),
                               [&](const ParamInfo &p) { return p.param_def() == param; });
        
        assert (vm_type_it != method_->param_end());
        r.type = vm_type_it->type();

        Push(r);
    }
    
    /* Arguments */    
    void OpcodeCompiler::VisitLdarga(IParameterDefinition *param)
    {
        auto vm_class = engine_->GetVMClassForNamedType(param->type()->resolved_type());
        Operand r(GetArgumentAddress(param), engine_->GetPointerType(vm_class));
        Push(r);
    }
    
    void OpcodeCompiler::VisitStarg(IParameterDefinition *param)
    {
        Operand r = Pop();
        builder_.CreateStore(r.value, GetArgumentAddress(param));
    }
    
    llvm::Value *OpcodeCompiler::GetArgumentAddress(IParameterDefinition *param)
    {
        return arguments_[param];
    }
    
    void OpcodeCompiler::VisitLdloc(ILocalDefinition *loc)
    {
        auto v = builder_.CreateLoad(locals_[loc].value);
        Push(Operand(v, locals_[loc].type));
    }
    
    void OpcodeCompiler::VisitLdloca(ILocalDefinition *loc)
    {
        Operand r;
        auto ty = engine_->GetPointerType(locals_[loc].type);
        Push(Operand(locals_[loc].value, ty));
    }
    
    void OpcodeCompiler::VisitStloc(ILocalDefinition *loc)
    {
        Operand r = Pop();
//        Value * v = fixTypeForAssignment(r.m_value, m_locals.at(loc).m_value);
        auto addr = locals_[loc];

        auto v = EnsureCorrectType(r, addr.type);
        v = EnsureSignatureMatching(Operand(v, addr.type), addr.type);

        builder_.CreateStore(v, addr.value);
    }

    void OpcodeCompiler::VisitLdcI4(int val)
    {
        auto ty = GetPrimitiveType(decil::INamedTypeDefinition::TypeCode::Int32);
        Push(Operand(builder_.getInt32((uint32_t)val), ty));
    }
    
    void OpcodeCompiler::VisitLdcI8(int64_t val)
    {
        auto ty = GetPrimitiveType(decil::INamedTypeDefinition::TypeCode::Int64);
        Push(Operand(builder_.getInt64((uint64_t)val), ty));
    }

    void OpcodeCompiler::VisitLdcR4(float val)
    {
        auto ty = GetPrimitiveType(decil::INamedTypeDefinition::TypeCode::Single);
        Push(Operand(ConstantFP::get(builder_.getFloatTy(), val), ty));
    }

    void OpcodeCompiler::VisitLdcR8(double val)
    {
        auto ty = GetPrimitiveType(decil::INamedTypeDefinition::TypeCode::Double);
        Push(Operand(ConstantFP::get(builder_.getDoubleTy(), val), ty));
    }
    
    void OpcodeCompiler::VisitDup()
    {
        auto r = Pop();
        Push(r);
        Push(r);
    }
    
    void OpcodeCompiler::VisitPop()
    {
        Pop();
    }

    void OpcodeCompiler::VisitCall(IMethodReference *method_ref)
    {
        auto method_def = method_ref->resolved_definition();
        auto vm_class = engine_->GetVMClassForNamedType(method_def->containing_type());
        auto callee = vm_class->GetMethod(mangler::mangle(method_def));
        assert (callee);

        auto f = callee->implementation();
        auto num_params = f->getFunctionType()->getNumParams();
        std::vector<Value *> args;
        args.reserve(num_params);
        
        for (unsigned i = 0; i < num_params; ++i)
        {
            auto r = Pop();
            auto v = EnsureCorrectType(r, callee->get_param(num_params - 1 - i).type());
            v = EnsureSignatureMatching(Operand(v, callee->get_param(num_params - 1 - i).type()),
                                    callee->get_param(num_params - 1 - i).type());
            args.push_back(v);
        }
        
        std::vector<Value *> real_args(args.rbegin(), args.rend());
        auto v = builder_.CreateCall(f, real_args);
        
        if (!f->getReturnType()->isVoidTy())
        {
            Push(Operand(v, callee->return_type()));
        }
    }
    
    void OpcodeCompiler::VisitRet()
    {
        if (current_function_->getReturnType()->isVoidTy())
        {
            builder_.CreateRetVoid();
        }
        else
        {
            Operand r = Pop();
            auto ret_type = method_->return_type();
            auto v = EnsureCorrectType(r, ret_type);
            v = EnsureSignatureMatching(Operand(v, ret_type), ret_type);
            builder_.CreateRet(v);
        }
    }
    
    void OpcodeCompiler::VisitBr(int pos)
    {
        auto block_info = &block_info_[pos];
        MergeCurrentStackInto(block_info);
        builder_.CreateBr(block_info->bb);
    }
    
    void OpcodeCompiler::VisitBrTF(int next_pos, int pos, bool branch_on_true)
    {
        Operand r;
        r.value = Constant::getNullValue(Peek().value->getType());
        r.type = Peek().type;
        Push(r);
        VisitCompareAndBranch(branch_on_true ? kBne_un : kBeq, next_pos, pos);
    }
    
    void OpcodeCompiler::VisitCompareAndBranch(Opcode opcode, int next_pos, int branch_pos)
    {
        VisitCompare(opcode);
        auto r = Pop();
        auto cond = builder_.CreateICmpNE(r.value, Constant::getNullValue(r.value->getType()));
        auto &true_block = block_info_[branch_pos];
        auto &false_block = block_info_[next_pos];
        
        MergeCurrentStackInto(&true_block);
        MergeCurrentStackInto(&false_block);
        BranchInst::Create(true_block.bb, false_block.bb, cond, current_bb_);
    }
    
    void OpcodeCompiler::VisitSwitch(const std::vector<int> &targets, int offset)
    {
        Operand value = Pop();
        int next_off = (int)(offset + 5 + 4 * targets.size());
        auto next_block = block_info_[next_off].bb;
        auto SI = builder_.CreateSwitch(value.value, next_block);
        for (unsigned i = 0; i < targets.size(); ++i)
        {
            auto target = block_info_[targets[i]].bb;
            SI->addCase(builder_.getInt32(i), target);
        }
    }
    
    void OpcodeCompiler::VisitLeave(int pos)
    {
        std::cout << "WARNING: Treating leave as br\n";
        VisitBr(pos);
    }

    void OpcodeCompiler::VisitNewObj(IMethodReference *ctor_ref)
    {
        // FIXME: Special handling for string?
        auto method_def = ctor_ref->resolved_definition();
        auto vm_class = engine_->GetVMClassForNamedType(method_def->containing_type());
        auto ctor = vm_class->GetMethod(mangler::mangle(method_def));
        assert (ctor);
        
        // Allocate object
        auto intrinsic = engine_->intrinsic();        
        Value *thiz = nullptr;
        if (vm_class->IsValueType())
        {
            // XXX: Move to the prelude block?
            thiz = builder_.CreateAlloca(vm_class->physical_type());
        }
        else
        {
            DataLayout TD(engine_->module());
            int size = (int)TD.getTypeStoreSize(vm_class->physical_type());
            auto alloc = builder_.CreateCall(intrinsic->new_object(), builder_.getInt32(size));
            thiz = builder_.CreateBitCast(alloc, PointerType::getUnqual(vm_class->physical_type()));
        }

        // Call constructor

        auto f = ctor->implementation();
        std::vector<Operand> args;

        auto num_params = f->getFunctionType()->getNumParams();
        args.reserve(num_params);
        
        for (unsigned i = 0; i < num_params - 1; ++i)
        {
            auto r = Pop();
            args.push_back(r);
        }

        args.push_back(Operand(thiz, vm_class));
        std::for_each(args.rbegin(), args.rend(), [&](const Operand &op) { Push(op); });
        VisitCall(method_def);
        if (vm_class->IsValueType())
        {
            auto value = builder_.CreateLoad(thiz);
            Push(Operand(value, vm_class));
        }
        else
        {
            Push(Operand(thiz, vm_class));
        }
    }
    
    void OpcodeCompiler::VisitBinaryOperator(Opcode opcode)
    {
        Operand rhs = Pop();
        Operand lhs = Pop();
        bool is_float = lhs.value->getType()->isFloatingPointTy();
        auto op = (Instruction::BinaryOps)ToLLVMBinaryOperator(opcode, is_float);
        
        
        if (lhs.value->getType()->isPointerTy() && rhs.value->getType()->isIntegerTy()
            && (op == Instruction::Add || op == Instruction::Sub))
        {
            auto native_int_ty = GetPrimitiveType(engine_->NativeIntTypeCode());
            auto ptr_type = lhs.value->getType();
            lhs.value = builder_.CreatePtrToInt(lhs.value, native_int_ty->normal_type());
            EnsureBinaryOperatorType(lhs, rhs);
            auto v = builder_.CreateBinOp(op, lhs.value, rhs.value);
            auto v1 = builder_.CreateIntToPtr(v, ptr_type);
            Push(Operand(v1, lhs.type));
        }
        else
        {
            EnsureBinaryOperatorType(lhs, rhs);
            auto v = builder_.CreateBinOp(op, lhs.value, rhs.value);
            Push(Operand(v, lhs.type));
        }
    }
    
    void OpcodeCompiler::VisitNeg()
    {
        Operand v = Pop();
        
        Operand r;
        r.type = v.type;
        if (v.value->getType()->isFloatingPointTy())
        {
            r.value = builder_.CreateFNeg(v.value);
        }
        else
        {
            r.value = builder_.CreateNeg(v.value);
        }
        Push(r);
    }
    
    void OpcodeCompiler::VisitNot()
    {
        Operand v = Pop();
        v.value = builder_.CreateNot(v.value);
        Push(v);
    }

    
    void OpcodeCompiler::VisitConversion(Opcode opcode)
    {
        Operand r = Pop();
        auto src = r.value;
        auto src_ty = src->getType();
        auto tc = ToPrimitiveTypeCode(opcode);
        auto vm_class = GetPrimitiveType(tc);
        auto dst_type = vm_class->normal_type();
        auto is_dst_unsigned = IsConversionToUnsigned(opcode);
        
        Value *new_v = nullptr;
        
        if (tc == INamedTypeDefinition::TypeCode::Single
            || tc == INamedTypeDefinition::TypeCode::Double)
        {
            if (src_ty->isFloatingPointTy())
            {
                new_v = CreateFPTruncOrExt(src, dst_type);
            }
            else
            {
                new_v = opcode == kConv_r_un
                ? builder_.CreateUIToFP(src, dst_type)
                : builder_.CreateSIToFP(src, dst_type);
            }
        }
        else
        {
            // Convert to int
            if (src_ty->isFloatingPointTy())
            {
                new_v = CreateFPToInt(src, dst_type, is_dst_unsigned);
            }
            else if (src_ty->isPointerTy())
            {
                auto v1 = builder_.CreatePtrToInt(src, dst_type);
                new_v = CreateIntTruncOrExt(v1, dst_type, is_dst_unsigned);
            }
            else
            {
                new_v = CreateIntTruncOrExt(src, dst_type, is_dst_unsigned);
            }
        }

        Push(Operand(new_v, vm_class));
    }
    
    void OpcodeCompiler::VisitLdobj(ITypeReference *type_ref)
    {
        auto vm_class = engine_->GetVMClassForNamedType(type_ref->resolved_type());
        auto ptr_type = vm_class->IsValueType()
        ? PointerType::getUnqual(vm_class->normal_type())
        : vm_class->normal_type();
        
        auto src = Pop();
        auto ptr = CreateIntPtrOrBitCast(src.value, ptr_type);
        auto v = builder_.CreateLoad(ptr);
        Push(Operand(v, vm_class));
    }

    void OpcodeCompiler::VisitStobj(ITypeReference *type_ref)
    {
        Operand src = Pop();
        Operand dest = Pop();
        
        auto vm_class = engine_->GetVMClassForNamedType(type_ref->resolved_type());
        auto ptr_type = vm_class->IsValueType()
        ? PointerType::getUnqual(vm_class->normal_type())
        : vm_class->normal_type();
        auto ptr = CreateIntPtrOrBitCast(dest.value, ptr_type);
        
        auto v = EnsureCorrectType(src, vm_class);
        v = EnsureSignatureMatching(Operand(v, vm_class), vm_class);
        builder_.CreateStore(v, ptr);
    }

    void OpcodeCompiler::VisitLdstr(const std::u16string &str)
    {
        Value *v = engine_->GetOrCreateString(str);
        auto type = GetPrimitiveType(INamedTypeDefinition::TypeCode::String);
        Push(Operand(v, type));
    }
    
    void OpcodeCompiler::VisitCompare(decil::Opcode opcode)
    {
        Operand rhs = Pop();
        Operand lhs = Pop();
        bool is_float = lhs.value->getType()->isFloatingPointTy();
        auto op = (CmpInst::Predicate)ToLLVMComparePredicate(opcode, is_float);

        Value *v = nullptr;

        if (is_float)
        {
            v = builder_.CreateFCmp(op, lhs.value, rhs.value);
        }
        else if (lhs.value->getType()->isPointerTy() && rhs.value->getType()->isPointerTy())
        {
            auto v1 = builder_.CreateBitCast(rhs.value, lhs.value->getType());
            v = builder_.CreateICmp(op, lhs.value, v1);
        }
        else
        {
            EnsureBinaryOperatorType(lhs, rhs);
            v = builder_.CreateICmp(op, lhs.value, rhs.value);
        }

        auto vm_type = GetPrimitiveType(decil::INamedTypeDefinition::TypeCode::Boolean);
        Push(Operand(v, vm_type));
    }
    
    void OpcodeCompiler::VisitLdind(Opcode opcode)
    {
        Operand addr = Pop();
        auto tc = ToPrimitiveTypeCode(opcode);
        auto vm_class = GetPrimitiveType(tc);
        auto ptr = CreateIntPtrOrBitCast(addr.value, PointerType::getUnqual(vm_class->normal_type()));
        auto v = builder_.CreateLoad(ptr);
        Push(Operand(v, vm_class));
    }
    
    void OpcodeCompiler::VisitStind(Opcode opcode)
    {
        Operand value = Pop();
        Operand addr = Pop();
        auto tc = ToPrimitiveTypeCode(opcode);
        auto vm_class = GetPrimitiveType(tc);
        auto ptr = CreateIntPtrOrBitCast(addr.value, PointerType::getUnqual(vm_class->normal_type()));
        
        auto v = EnsureCorrectType(value, vm_class);
        v = EnsureSignatureMatching(Operand(v, vm_class), vm_class);

        builder_.CreateStore(v, ptr);
    }
    
    void OpcodeCompiler::VisitLdnull()
    {
        Operand r;
        auto ty = GetPrimitiveType(INamedTypeDefinition::TypeCode::IntPtr);
        r.value = Constant::getNullValue(ty->normal_type());
        r.type = ty;
        Push(r);
    }
    
    void OpcodeCompiler::VisitLdelema(decil::ITypeReference *type_ref)
    {
        auto index = Pop();
        auto array = Pop();

        auto vm_class = engine_->GetVMClassForNamedType(type_ref->resolved_type());

        Operand r;
        r.type = vm_class;
        r.value = CreateArrayGEP(array.value, index.value, vm_class->normal_type());
        Push(r);
    }

    void OpcodeCompiler::VisitLdelem(decil::ITypeReference *type_ref)
    {
        VisitLdelema(type_ref);
        Operand addr = Pop();

        auto vm_class = engine_->GetVMClassForNamedType(type_ref->resolved_type());
        Operand r;
        r.type = vm_class;
        r.value = builder_.CreateLoad(addr.value);
        Push(r);
    }

    void OpcodeCompiler::VisitStelem(decil::ITypeReference *type_ref)
    {
        Operand value = Pop();
        VisitLdelema(type_ref);
        Operand addr = Pop();
        
        auto vm_class = engine_->GetVMClassForNamedType(type_ref->resolved_type());
        Operand r;
        r.type = vm_class;
        auto v = EnsureCorrectType(value, vm_class);
        v = EnsureSignatureMatching(Operand(v, vm_class), vm_class);
        builder_.CreateStore(v, addr.value);
    }

    void OpcodeCompiler::VisitLdelem(decil::Opcode opcode)
    {
        auto tc = ToPrimitiveTypeCode(opcode);
        auto ty = dynamic_cast<VMPrimitiveClass*>(GetPrimitiveType(tc));
        VisitLdelema(ty->type_def());
        Operand addr = Pop();
        auto v = builder_.CreateLoad(addr.value);
        Push(Operand(v, ty));
    }
    
    void OpcodeCompiler::VisitStelem(decil::Opcode opcode)
    {
        Operand value = Pop();
        auto tc = ToPrimitiveTypeCode(opcode);
        auto ty = dynamic_cast<VMPrimitiveClass*>(GetPrimitiveType(tc));
        VisitLdelema(ty->type_def());
        Operand addr = Pop();
        
        auto v = EnsureCorrectType(value, ty);
        v = EnsureSignatureMatching(Operand(v, ty), ty);
        builder_.CreateStore(v, addr.value);
    }

    void OpcodeCompiler::VisitNewarr(ITypeReference *type_ref)
    {
        DataLayout TD(engine_->module());
        auto intrinsic_new_array = engine_->intrinsic()->new_array();
        auto vm_class = engine_->GetVMClassForNamedType(type_ref->resolved_type());
        auto vm_array_class = new VMClassVector(engine_, vm_class);
        vm_array_class->LayoutIfNecessary();
        
        size_t elem_size = TD.getTypeStoreSize(vm_class->normal_type());
        Operand array_size = Pop();
        auto array_size_v = EnsureSignatureMatching(array_size,
                                                    GetPrimitiveType(decil::INamedTypeDefinition::TypeCode::Int32));
        Operand r;
        r.type = vm_array_class;
        auto arr_ptr = builder_.CreateCall2(intrinsic_new_array, array_size_v, builder_.getInt32((int)elem_size));
        r.value = builder_.CreateBitCast(arr_ptr, vm_array_class->normal_type());
        
        Push(r);
    }
    
    void OpcodeCompiler::VisitLdlen()
    {
        Operand v = Pop();
        auto arr_type_ref = engine_->host()->platform_type()->system_array();
        auto arr_type = engine_->GetVMClassForNamedType(arr_type_ref->resolved_type());
        auto arr_len_method = arr_type->GetMethod(u"get_Length");
        assert (arr_len_method);
        auto arr_ptr = builder_.CreateBitCast(v.value, arr_type->normal_type());
        Push(Operand(arr_ptr, arr_type));
        VisitCall(arr_len_method->method_def());
    }
    
    void OpcodeCompiler::VisitLoadField(IFieldReference *field_ref)
    {
        VisitLoadFieldAddress(field_ref);
        auto field_def = field_ref->resolved_definition();
        auto vm_class = engine_->GetVMClassForNamedType(field_def->containing_type());
        auto vm_field = vm_class->GetField(field_def->name());

        Operand addr = Pop();
        auto v = builder_.CreateLoad(addr.value);
        Push(Operand(v, vm_field->type()));
    }
    
    void OpcodeCompiler::VisitLoadFieldAddress(IFieldReference *field_ref)
    {
        auto field_def = field_ref->resolved_definition();
        auto vm_field_class = engine_->GetVMClassForNamedType(field_def->containing_type());
        auto vm_field = vm_field_class->GetField(field_def->name());
        auto vm_class = engine_->GetVMClassForNamedType(field_def->containing_type());
        
        Value *inst = nullptr;
        if (vm_field->is_static())
        {
            inst = vm_field_class->static_instance();
        }
        else
        {
            Operand addr = Pop();
            auto ptr_ty = vm_field_class->IsValueType()
            ? PointerType::getUnqual(vm_field_class->boxed_type())
            : vm_field_class->normal_type();
            
            // addr could be a reference to a value type
            // (but might be an instance of llvm::PointerType)
            if (vm_class->IsValueType() && !addr.value->getType()->isPointerTy())
            {
                // instance of value type, do a store load to get the address
                auto tmp = builder_.CreateAlloca(addr.value->getType());
                builder_.CreateStore(addr.value, tmp);
                inst = builder_.CreateBitCast(tmp, ptr_ty);
            }
            else
            {
                // a reference type, or a pointer for the value type
                inst = builder_.CreateBitCast(addr.value, ptr_ty);
            }
        }
        auto gep = builder_.CreateStructGEP(inst, vm_field->offset());
        Push(Operand(gep, engine_->GetPointerType(vm_field->type())));

    }
    
    void OpcodeCompiler::VisitStoreField(IFieldReference *field_ref)
    {
        Operand r = Pop();
        VisitLoadFieldAddress(field_ref);
        auto field_def = field_ref->resolved_definition();
        auto vm_class = engine_->GetVMClassForNamedType(field_def->containing_type());
        auto vm_field = vm_class->GetField(field_def->name());
        auto vm_field_ty = vm_field->type();
        
        Operand addr = Pop();
        auto field_ptr_type = PointerType::getUnqual(vm_field_ty->normal_type());
        
        auto ptr = CreateIntPtrOrBitCast(addr.value, field_ptr_type);        
        auto v = EnsureCorrectType(r, vm_field_ty);
        v = EnsureSignatureMatching(Operand(v, vm_field_ty), vm_field_ty);
        builder_.CreateStore(v, ptr);
    }
    
    void OpcodeCompiler::VisitInitObj(ITypeReference *type_ref)
    {
        auto vm_class = engine_->GetVMClassForNamedType(type_ref->resolved_type());
        Operand dest = Pop();
        auto ptr = builder_.CreateBitCast(dest.value, PointerType::getUnqual(vm_class->normal_type()));
        auto zero = Constant::getNullValue(vm_class->normal_type());
        builder_.CreateStore(zero, ptr);
    }
    
    void OpcodeCompiler::VisitSizeof(ITypeReference *type_ref)
    {
        auto vm_class = engine_->GetVMClassForNamedType(type_ref->resolved_type());

        DataLayout TD(engine_->module());
        auto size = TD.getTypeStoreSize(vm_class->physical_type());
        
        auto int32ty = GetPrimitiveType(INamedTypeDefinition::TypeCode::Int32);
        Push(Operand(builder_.getInt32((int)size), int32ty));
    }
    
    void OpcodeCompiler::VisitBox(ITypeReference *type_ref)
    {
        auto vm_class = engine_->GetVMClassForNamedType(type_ref->resolved_type());
        if (!vm_class->IsValueType())
            return;
        
        Operand val = Pop();

        DataLayout TD(engine_->module());
        auto intrinsic = engine_->intrinsic();
        
        int size = (int)TD.getTypeStoreSize(vm_class->boxed_type());
        auto mem = builder_.CreateCall(intrinsic->new_object(), builder_.getInt32(size));
        auto ptr = builder_.CreateBitCast(mem, PointerType::getUnqual(val.value->getType()));
        
        
//        Value * v = fixTypeForAssignment(val.m_value, ptr);
        builder_.CreateStore(val.value, ptr);
        auto boxed_ptr = builder_.CreateBitCast(ptr, PointerType::getUnqual(vm_class->boxed_type()));
        Push(Operand(boxed_ptr, vm_class));
    }
    
    void OpcodeCompiler::VisitLdtoken(IMetadata *md)
    {
        if (auto field_ref = dynamic_cast<IFieldReference*>(md))
        {
            auto field_def = field_ref->resolved_definition();
            auto parent_class = engine_->GetVMClassForNamedType(field_def->containing_type());
            auto vm_field = parent_class->GetField(field_def->name());
            auto runtime_handle = builder_.getInt64(reinterpret_cast<uint64_t>(vm_field));
            
            auto field_handle_ty = engine_->host()->platform_type()->system_runtime_field_handle()->resolved_type();
            auto field_handle_vm_class = engine_->GetVMClassForNamedType(field_handle_ty);
            auto alloca = builder_.CreateAlloca(field_handle_vm_class->normal_type());
            auto handle_ptr = builder_.CreateStructGEP(alloca, 0);
            builder_.CreateStore(runtime_handle, handle_ptr);

            Operand r;

            r.type = field_handle_vm_class;
            auto LI = builder_.CreateLoad(alloca);
            r.value = LI;
            // Annotate the load instruction for later fixups
            Value *args[] = { runtime_handle };
            MDNode *md = MDNode::get(LI->getContext(), args);
            LI->setMetadata("silk_runtime_field_handle", md);
            Push(r);
        }
        else
        {
            assert (0 && "Unimplemented");
        }
    }

    void OpcodeCompiler::VisitUnboxAny(ITypeReference *type_ref)
    {
        VisitCastClass(type_ref);
        auto vm_class = engine_->GetVMClassForNamedType(type_ref->resolved_type());
        if (!vm_class->IsValueType())
            return;
        
        Operand val = Pop();
        //Value * ptr = builder_.CreateStructGEP(val.value, 1);
        
        auto v = builder_.CreateLoad(val.value);
        Push(Operand(v, vm_class));
    }
    
    void OpcodeCompiler::VisitCastClass(ITypeReference *type_ref)
    {
        auto vm_class = engine_->GetVMClassForNamedType(type_ref->resolved_type());
        auto obj = Pop();

        // FIXME: Actually check the type
        auto target_ty = vm_class->IsValueType()
        ? PointerType::getUnqual(vm_class->normal_type())
        : vm_class->normal_type();
        
        auto v = builder_.CreateBitCast(obj.value, target_ty);
        Push(Operand(v, vm_class));
    }
    
    void OpcodeCompiler::VisitIsInst(ITypeReference *)
    {
        // FIXME: actually check the type
    }
    
    bool OpcodeCompiler::IsUnsignedIntVMClass(const VMClass *clazz) const
    {
        switch (clazz->type_code())
        {
            case decil::INamedTypeDefinition::TypeCode::UInt8:
            case decil::INamedTypeDefinition::TypeCode::UInt16:
            case decil::INamedTypeDefinition::TypeCode::UInt32:
            case decil::INamedTypeDefinition::TypeCode::UInt64:
            case decil::INamedTypeDefinition::TypeCode::Char:
                return true;
                
            default:
                return false;
        }
    }
    
    bool OpcodeCompiler::IsConversionToUnsigned(Opcode opcode)
    {
        switch (opcode) {
            case kConv_r_un:
            case kConv_u1:
            case kConv_u2:
            case kConv_u4:
            case kConv_u8:
            case kConv_u:
            case kConv_ovf_u1_un:
            case kConv_ovf_u2_un:
            case kConv_ovf_u4_un:
            case kConv_ovf_u8_un:
            case kConv_ovf_u_un:
            case kConv_ovf_u1:
            case kConv_ovf_u2:
            case kConv_ovf_u4:
            case kConv_ovf_u8:
            case kConv_ovf_u:
                return true;
                
            default:
                return false;
        }
    }

    int OpcodeCompiler::ToLLVMBinaryOperator(Opcode opcode, bool is_float)
    {
        switch (opcode) {
            case kAdd:
                return is_float ? Instruction::FAdd : Instruction::Add;
            case kSub:
                return is_float ? Instruction::FSub : Instruction::Sub;
                
            // TODO: handle overflow correctly
            case kMul_ovf:
            case kMul_ovf_un:
            case kMul:
                return is_float ? Instruction::FMul : Instruction::Mul;
            case kDiv:
                return is_float ? Instruction::FDiv : Instruction::SDiv;
            case kDiv_un:
                return Instruction::UDiv;
            case kRem:
                return is_float ? Instruction::FRem : Instruction::SRem;
            case kRem_un:
                return Instruction::URem;
            case kAnd:
                return Instruction::And;
            case kOr:
                return Instruction::Or;
            case kXor:
                return Instruction::Xor;
            case kShl:
                return Instruction::Shl;
            case kShr:
                return Instruction::AShr;
            case kShr_un:
                return Instruction::LShr;
                
            case kAdd_ovf:
            case kAdd_ovf_un:
            case kSub_ovf:
            case kSub_ovf_un:
                assert (0 && "unimplemented");
                
            default:
                return 0;
        }
        // Unreachable                                                                                                                                                                  
        return 0;
    }
    
    int OpcodeCompiler::ToLLVMComparePredicate(Opcode opcode, bool is_float)
    {
        switch (opcode)
        {
            case kBge_s:
            case kBge:
                return is_float ? CmpInst::FCMP_OGE : CmpInst::ICMP_SGE;
            case kBle_s:
            case kBle:
                return is_float ? CmpInst::FCMP_OLE : CmpInst::ICMP_SLE;
            case kBne_un_s:
            case kBne_un:
                return is_float ? CmpInst::FCMP_UNE : CmpInst::ICMP_NE;
            case kBge_un_s:
            case kBge_un:
                return is_float ? CmpInst::FCMP_UGE : CmpInst::ICMP_UGE;
            case kBle_un_s:
            case kBle_un:
                return is_float ? CmpInst::FCMP_ULE : CmpInst::ICMP_ULE;
            case kBeq_s:
            case kBeq:
            case kCeq:
                return is_float ? CmpInst::FCMP_OEQ : CmpInst::ICMP_EQ;
            case kBgt_s:
            case kBgt:
            case kCgt:
                return is_float ? CmpInst::FCMP_OGT : CmpInst::ICMP_SGT;
            case kBgt_un_s:
            case kBgt_un:
            case kCgt_un:
                return is_float ? CmpInst::FCMP_UGT : CmpInst::ICMP_UGT;
            case kBlt_s:
            case kBlt:
            case kClt:
                return is_float ? CmpInst::FCMP_OLT : CmpInst::ICMP_SLT;
            case kBlt_un_s:
            case kBlt_un:
            case kClt_un:
                return is_float ? CmpInst::FCMP_ULT : CmpInst::ICMP_ULT;
            default:
                return 0;
        }
    }
    
    INamedTypeDefinition::TypeCode OpcodeCompiler::ToPrimitiveTypeCode(Opcode opcode)
    {
        switch (opcode)
        {
            case kLdind_i1:
            case kStind_i1:
            case kConv_i1:
            case kConv_ovf_i1_un:
            case kConv_ovf_i1:
            case kLdelem_i1:
            case kStelem_i1:
                return INamedTypeDefinition::TypeCode::Int8;
            case kLdind_u1:
            case kConv_u1:
            case kConv_ovf_u1_un:
            case kConv_ovf_u1:
            case kLdelem_u1:
                return INamedTypeDefinition::TypeCode::UInt8;
            case kLdind_i2:
            case kStind_i2:
            case kConv_i2:
            case kConv_ovf_i2_un:
            case kConv_ovf_i2:
            case kLdelem_i2:
            case kStelem_i2:
                return INamedTypeDefinition::TypeCode::Int16;
            case kLdind_u2:
            case kConv_u2:
            case kConv_ovf_u2_un:
            case kConv_ovf_u2:
            case kLdelem_u2:
                return INamedTypeDefinition::TypeCode::UInt16;
            case kLdind_i4:
            case kStind_i4:
            case kConv_i4:
            case kConv_ovf_i4_un:
            case kConv_ovf_i4:                    
            case kLdelem_i4:
            case kStelem_i4:
                return INamedTypeDefinition::TypeCode::Int32;
            case kLdind_u4:
            case kConv_u4:
            case kConv_ovf_u4_un:
            case kConv_ovf_u4:                    
            case kLdelem_u4:
                return INamedTypeDefinition::TypeCode::UInt32;
            case kLdind_i8:
            case kStind_i8:
            case kConv_i8:
            case kConv_ovf_i8_un:
            case kConv_ovf_i8:                    
            case kLdelem_i8:
            case kStelem_i8:
                return INamedTypeDefinition::TypeCode::Int64;
            case kConv_u8:
            case kConv_ovf_u8_un:
            case kConv_ovf_u8:                    
                return INamedTypeDefinition::TypeCode::UInt64;
            case kLdind_i:
            case kStind_i:
            case kConv_i:
            case kConv_ovf_i_un:
            case kConv_ovf_i:
            case kLdelem_i:
            case kStelem_i:
                return engine_->NativeIntTypeCode();
            case kConv_u:
            case kConv_ovf_u:
            case kConv_ovf_u_un:                    
                return engine_->NativeUIntTypeCode();
                
            case kLdind_r4:
            case kStind_r4:
            case kConv_r4:
            case kLdelem_r4:
            case kStelem_r4:
                return INamedTypeDefinition::TypeCode::Single;
            case kLdind_r8:
            case kStind_r8:
            case kConv_r8:
            // XXX: Is F == Double?
            case kConv_r_un:
            case kLdelem_r8:
            case kStelem_r8:
                return INamedTypeDefinition::TypeCode::Double;
            case kLdind_ref:
            case kStind_ref:
            case kLdelem_ref:
            case kStelem_ref:
                return INamedTypeDefinition::TypeCode::IntPtr;

            default:
                return INamedTypeDefinition::TypeCode::NotPrimitive;
        }
    }

    VMClass *OpcodeCompiler::GetPrimitiveType(INamedTypeDefinition::TypeCode tc)
    {
#define PRIMITIVE_CASE(t, name) \
case decil::INamedTypeDefinition::TypeCode::t: \
ref = engine_->host()->platform_type()->name()->resolved_type(); \
break;

        ITypeReference *ref = nullptr;
        switch (tc)
        {
                PRIMITIVE_CASE(Void, system_void)
                PRIMITIVE_CASE(Boolean, system_boolean)
                PRIMITIVE_CASE(Char, system_char)
                PRIMITIVE_CASE(Int8, system_int8)
                PRIMITIVE_CASE(UInt8, system_uint8)
                PRIMITIVE_CASE(Int16, system_int16)
                PRIMITIVE_CASE(UInt16, system_uint16)
                PRIMITIVE_CASE(Int32, system_int32)
                PRIMITIVE_CASE(UInt32, system_uint32)
                PRIMITIVE_CASE(Int64, system_int64)
                PRIMITIVE_CASE(UInt64, system_uint64)
                PRIMITIVE_CASE(Single, system_float32)
                PRIMITIVE_CASE(Double, system_float64)
                PRIMITIVE_CASE(String, system_string)
                PRIMITIVE_CASE(IntPtr, system_intptr)
                PRIMITIVE_CASE(UIntPtr, system_uintptr)
            case INamedTypeDefinition::TypeCode::NotPrimitive:
                assert (0 && "Unreachable");
        }
        return engine_->GetVMClassForNamedType(ref->resolved_type());
#undef PRIMITIVE_CASE
    }
    
}
