//
//  ILReader.cpp
//  silk
//
//  Created by Haohui Mai on 12/7/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#include "ILReader.h"
#include "PEFileReader.h"
#include "BinaryObjectModel.h"
#include "PEFileToObjectModel.h"

#include "silk/Support/ErrorHandler.h"

#include <llvm/ADT/OwningPtr.h>

namespace silk
{
    namespace decil
    {
        ILReader::ILReader(PEFileToObjectModel *model, MethodDefinition *method)
        : model_(model)
        , method_(method)
        {}
        
        void ILReader::ReadIL()
        {
            PopulateCILInstructions();
        }
        
        void ILReader::PopulateCILInstructions()
        {
            raw_istream is = method_->method_il()->EncodedILMemoryBlock;
            char i8;
            int16_t i16;
            int32_t i32;
            int64_t i64;
            float float4;
            double double8;
            MDToken token;
            
            while (!is.eof())
            {
                ILOperand operand;
                int offset = (int)is.tellg();
                auto opcode = GetOpcode(is);

                switch (opcode)
                {
                    case kNop:
                    case kBreak:
                        break;

                    case kLdarg_0:
                    case kLdarg_1:
                    case kLdarg_2:
                    case kLdarg_3:
                        operand.SetMetadata(GetParameter((int)opcode - kLdarg_0));
                        break;
                        
                    case kLdloc_0:
                    case kLdloc_1:
                    case kLdloc_2:
                    case kLdloc_3:
                        operand.SetMetadata(GetLocal((int)opcode - kLdloc_0));
                        break;
                        
                    case kStloc_0:
                    case kStloc_1:
                    case kStloc_2:
                    case kStloc_3:
                        operand.SetMetadata(GetLocal((int)opcode - kStloc_0));
                        break;
                        
                    case kLdarg_s:
                    case kLdarga_s:
                    case kStarg_s:
                        is >> i8;
                        operand.SetMetadata(GetParameter(i8));
                        break;
                    case kLdloc_s:
                    case kLdloca_s:
                    case kStloc_s:
                        is >> i8;
                        operand.SetMetadata(GetLocal(i8));
                        break;
                    case kLdnull:
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
                        operand.SetInt((int)opcode - kLdc_i4_0);
                        break;
                    case kLdc_i4_s:
                        is >> i8;
                        operand.SetInt(i8);
                        break;
                    case kLdc_i4:
                        is >> i32;
                        operand.SetInt(i32);
                        break;
                    case kLdc_i8:
                        is >> i64;
                        operand.SetInt(i64);
                        break;
                    case kLdc_r4:
                        is >> float4;
                        operand.SetFloat(float4);
                        break;
                    case kLdc_r8:
                        is >> double8;
                        operand.SetDouble(double8);
                        break;
                    case kDup:
                    case kPop:
                        break;
                    case kJmp:
                        is >> i32;
                        operand.SetInt(i32);
                        break;
                    case kCall:
                        is >> token;
                        operand.SetMetadata(GetMethod(&token));
                        break;
//                    case kCalli:
//                        value = this.GetFunctionPointerType(memReader.ReadUInt32());
//                        break;
                    case kRet:
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
                        is >> i8;
                        operand.SetInt(i8 + 2 + offset);
                        break;
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
                        is >> i32;
                        operand.SetInt(i32 + 5 + offset);
                        break;
                    case kSwitch:
                    {
                        is >> i32;
                        auto num_targets = i32;
                        std::vector<int> targets;
                        targets.reserve(num_targets);
                        auto off = (int)offset + num_targets * 4 + 5;
                        for (auto i = 0; i < num_targets; ++i)
                        {
                            is >> i32;
                            targets.push_back(i32 + off);
                        }
                        operand.SetIntArray(targets);
                    }
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
                    case kStind_ref:
                    case kStind_i1:
                    case kStind_i2:
                    case kStind_i4:
                    case kStind_i8:
                    case kStind_r4:
                    case kStind_r8:
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
                    case kNeg:
                    case kNot:
                    case kConv_i1:
                    case kConv_i2:
                    case kConv_i4:
                    case kConv_i8:
                    case kConv_r4:
                    case kConv_r8:
                    case kConv_u4:
                    case kConv_u8:
                        break;
                    case kCallvirt:
                        is >> token;
                        operand.SetMetadata(GetMethod(&token));
                        break;
                        
                    case kCpobj:
                    case kLdobj:
                        is >> token;
                        operand.SetMetadata(GetType(&token));
                        break;
                    case kLdstr:
                        is >> token;
                        operand.SetString(GetUserStringForToken(&token));
                        break;
                    case kNewobj:
                        is >> token;
                        operand.SetMetadata(GetMethod(&token));
                        break;
                    case kCastclass:
                    case kIsinst:
                        is >> token;
                        operand.SetMetadata(GetType(&token));
                        break;
                    case kConv_r_un:
                        break;
                    case kUnbox:
                        is >> token;
                        operand.SetMetadata(GetType(&token));
                        break;
                    case kThrow:
                        break;
                    case kLdfld:
                    case kLdflda:
                    case kStfld:
                        is >> token;
                        operand.SetMetadata(GetField(&token));
                        break;
                    case kLdsfld:
                    case kLdsflda:
                    case kStsfld:
                        is >> token;
                        operand.SetMetadata(GetField(&token));
                        break;
                    case kStobj:
                        is >> token;
                        operand.SetMetadata(GetType(&token));
                        break;
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
                        break;
                    case kBox:
                        is >> token;
                        operand.SetMetadata(GetType(&token));
                        break;
                    case kNewarr:
                        is >> token;
                        operand.SetMetadata(GetType(&token));
                        break;
                    case kLdlen:
                        break;
                    case kLdelema:
                        is >> token;
                        operand.SetMetadata(GetType(&token));
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
                    case kStelem_i:
                    case kStelem_i1:
                    case kStelem_i2:
                    case kStelem_i4:
                    case kStelem_i8:
                    case kStelem_r4:
                    case kStelem_r8:
                    case kStelem_ref:
                        break;
                    case kLdelem:
                    case kStelem:
                    case kUnbox_any:
                        is >> token;
                        operand.SetMetadata(GetType(&token));
                        break;
                    case kConv_ovf_i1:
                    case kConv_ovf_u1:
                    case kConv_ovf_i2:
                    case kConv_ovf_u2:
                    case kConv_ovf_i4:
                    case kConv_ovf_u4:
                    case kConv_ovf_i8:
                    case kConv_ovf_u8:
                        break;
                    case kRefanyval:
                        is >> token;
                        operand.SetMetadata(GetType(&token));
                        break;
                    case kCkfinite:
                        break;
                    case kMkrefany:
                        is >> token;
                        operand.SetMetadata(GetType(&token));
                        break;
                    case kLdtoken:
                        is >> token;
                        operand.SetMetadata(GetRuntimeHandleFromToken(&token));
                        break;
                        
                    case kConv_u2:
                    case kConv_u1:
                    case kConv_i:
                    case kConv_ovf_i:
                    case kConv_ovf_u:
                    case kAdd_ovf:
                    case kAdd_ovf_un:
                    case kMul_ovf:
                    case kMul_ovf_un:
                    case kSub_ovf:
                    case kSub_ovf_un:
                    case kEndfinally:
                        break;
                    case kLeave:
                        is >> i32;
                        operand.SetInt(offset + 5 + i32);
                        break;
                    case kLeave_s:
                        is >> i8;
                        operand.SetInt(offset + 2 + i8);
                        break;
                    case kStind_i:
                    case kConv_u:
                    case kArglist:
                    case kCeq:
                    case kCgt:
                    case kCgt_un:
                    case kClt:
                    case kClt_un:
                        break;
                    case kLdftn:
                    case kLdvirtftn:
                        is >> token;
                        operand.SetMetadata(GetMethod(&token));
                        break;
                    case kLdarg:
                    case kLdarga:
                    case kStarg:
                        is >> i16;
                        operand.SetMetadata(GetParameter(i16));
                        break;
                    case kLdloc:
                    case kLdloca:
                    case kStloc:
                        is >> i16;
                        operand.SetInt(i16);
                        break;
                    case kLocalloc:
                        break;
                    case kEndfilter:
                        break;
                    case kUnaligned_:
                        is >> i8;
                        operand.SetInt(i8);
                        break;
                    case kVolatile_:
                    case kTail_:
                        break;
                    case kInitobj:
                    case kConstrained_:
                        is >> token;
                        operand.SetMetadata(GetType(&token));
                        break;
                    case kCpblk:
                    case kInitblk:
                        break;
                    case kNo_:
                        is >> i8;
                        operand.SetInt(i8);
                        break;
                    case kRethrow:
                        break;
                    case kSizeof:
                        is >> token;
                        operand.SetMetadata(GetType(&token));
                        break;
                    case kRefanytype:
                    case kReadonly_:
                        break;
                    default:
                        assert (0 && "Unimplemented opcode");
                        break;
                }
                auto inst = new CILOperation(opcode, offset, operand);
                instructions_.push_back(inst);
            }
        }
        
        Opcode ILReader::GetOpcode(raw_istream &is)
        {
            uint8_t b0, b1;
            is >> b0;
            
            if (b0 == kLongOpcodePrefix) {
                is >> b1;
                return (Opcode)((b0 << 8) | b1);
            } else {
                return (Opcode)b0;
            }
        }
        
        IMetadata *ILReader::GetLocal(int idx)
        {
            return method_->get_local(idx);
        }
        
        IMetadata *ILReader::GetParameter(int idx)
        {
            return method_->get_param(idx);
        }
        
        IMetadata *ILReader::GetMethod(const MDToken *tok)
        {
            auto method = model_->GetMethodReferenceForToken(tok);
            assert (method);
            return method;
        }
        
        IMetadata *ILReader::GetType(const MDToken *tok)
        {
            auto type = model_->GetTypeReferenceForToken(tok);
            assert (type);
            return type;
        }
        
        IMetadata *ILReader::GetField(const MDToken *tok)
        {
            auto field = model_->GetFieldReferenceForToken(tok);
            assert (field);
            return field;
        }
        
        IMetadata *ILReader::GetRuntimeHandleFromToken(const MDToken *tok)
        {
            auto type = tok->id();
            if (type == kField || type == kMemberReference)
            {
                return model_->GetFieldReferenceForToken(tok);
            }
            else
            {
                assert (0 && "unimplemented");
            }
            return nullptr;
        }
        
        std::u16string ILReader::GetUserStringForToken(const MDToken *tok)
        {
            ErrorHandler eh;
            raw_istream dummy;
            MDLoader loader(model_->file(), dummy, eh);
            MDString str;
            loader.LoadUserString(&str, *tok);
            return (std::u16string)str;
        }
    }
}

