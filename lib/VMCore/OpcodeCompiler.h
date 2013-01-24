//
//  OpcodeCompiler.h
//  silk
//
//  Created by Haohui Mai on 12/8/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#ifndef SILK_LIB_OPCODE_COMPILER_H_
#define SILK_LIB_OPCODE_COMPILER_H_

#include "silk/decil/ObjectModel.h"

#include <llvm/IRBuilder.h>

#include <vector>
#include <unordered_map>

namespace silk
{
    
    class CompilationEngine;
    class VMMethod;
    class VMClass;
    class OpcodeScanner;
    class OpcodeCompiler
    {
    public:
        OpcodeCompiler(CompilationEngine *engine, VMMethod *method);
        void Compile();
        
        struct Operand
        {
            llvm::Value *value;
            VMClass *type;
            Operand();
            Operand(llvm::Value *v, VMClass *type);
        };
        
        struct BlockInfo
        {
            llvm::BasicBlock *bb;
            std::vector<Operand> *stack;
        };
        
    private:
        void DeclareLocalVariables();
        void CompileInstruction(decil::IOperation *op);
        VMClass *GetPrimitiveType(decil::INamedTypeDefinition::TypeCode tc);
        bool IsUnsignedIntVMClass(const VMClass *clazz) const;
        
        void Push(const Operand &op) { stack_->push_back(op); }
        Operand Pop();
        Operand &Peek() { return stack_->back(); }
        void SwitchBasicBlock(BlockInfo *bi);
        void MergeCurrentStackInto(const BlockInfo *bi);

        llvm::Value *EnsureCorrectType(const Operand &v, VMClass *clazz);
        llvm::Value *EnsureSignatureMatching(const Operand &v, VMClass *dst_clazz);
        void EnsureBinaryOperatorType(Operand &lhs, Operand &rhs);
        llvm::Value *CreateIntPtrOrBitCast(llvm::Value *ptr, llvm::Type *pointer_type);
        llvm::Value *CreateFPTruncOrExt(llvm::Value *src, llvm::Type *target_type);
        llvm::Value *CreateFPToInt(llvm::Value *src, llvm::Type *dst_type, bool is_unsigned);
        llvm::Value *CreateIntTruncOrExt(llvm::Value *src, llvm::Type *dst_type, bool is_unsigned);
        llvm::Value *CreateArrayGEP(llvm::Value *array, llvm::Value *index, llvm::Type *element_type);

        llvm::Value *GetArgumentAddress(decil::IParameterDefinition *param);
        static int ToLLVMBinaryOperator(decil::Opcode opcode, bool is_float);
        static int ToLLVMComparePredicate(decil::Opcode opcode, bool is_float);
        decil::INamedTypeDefinition::TypeCode ToPrimitiveTypeCode(decil::Opcode opcode);
        
        void VisitLdarg(decil::IParameterDefinition *param);
        void VisitLdarga(decil::IParameterDefinition *param);
        void VisitStarg(decil::IParameterDefinition *param);
        void VisitLdloc(decil::ILocalDefinition *loc);
        void VisitStloc(decil::ILocalDefinition *loc);
        void VisitLdloca(decil::ILocalDefinition *loc);
        void VisitLdcI4(int val);
        void VisitLdcI8(int64_t val);
        void VisitLdcR4(float v);
        void VisitLdcR8(double v);
        void VisitDup();
        void VisitPop();
        void VisitCall(decil::IMethodReference *method_ref);
        void VisitRet();
        void VisitBr(int pos);
        void VisitBrTF(int next_pos, int branch_pos, bool branch_on_true);
        void VisitCompareAndBranch(decil::Opcode opcode, int next_pos, int branch_pos);
        void VisitSwitch(const std::vector<int> &branches, int op_offset);
        void VisitLeave(int pos);
        void VisitNewObj(decil::IMethodReference *ctor_ref);
        void VisitBinaryOperator(decil::Opcode opcode);
        void VisitNeg();
        void VisitNot();
        void VisitConversion(decil::Opcode opcode);

        void VisitLdobj(decil::ITypeReference *type_ref);
        void VisitStobj(decil::ITypeReference *type_ref);

        void VisitLdstr(const std::u16string &str);
        void VisitCompare(decil::Opcode opcode);
        void VisitLdind(decil::Opcode opcode);
        void VisitStind(decil::Opcode opcode);
        
        void VisitLdnull();
        void VisitLdelema(decil::ITypeReference *type_ref);
        void VisitLdelem(decil::ITypeReference *type_ref);
        void VisitStelem(decil::ITypeReference *type_ref);
        void VisitLdelem(decil::Opcode opcode);
        void VisitStelem(decil::Opcode opcode);
        void VisitNewarr(decil::ITypeReference *type_ref);
        void VisitLdlen();
        void VisitLoadField(decil::IFieldReference *field_ref);
        void VisitLoadFieldAddress(decil::IFieldReference *field_ref);
        void VisitStoreField(decil::IFieldReference *field_ref);
        
        void VisitInitObj(decil::ITypeReference *type_ref);
        void VisitSizeof(decil::ITypeReference *type_ref);
        void VisitBox(decil::ITypeReference *type_ref);
        void VisitUnboxAny(decil::ITypeReference *type_ref);
        void VisitLdtoken(decil::IMetadata *md);
        void VisitCastClass(decil::ITypeReference *type_ref);
        void VisitIsInst(decil::ITypeReference *type_ref);
        
        std::vector<Operand> *stack_;
        std::unordered_map<decil::ILocalDefinition*, Operand> locals_;
        std::unordered_map<decil::IParameterDefinition *, llvm::Value*> arguments_;
        std::unordered_map<int, OpcodeCompiler::BlockInfo> block_info_;
        CompilationEngine *engine_;
        VMMethod *method_;
        llvm::Function *current_function_;
        llvm::BasicBlock *prelude_bb_;
        llvm::BasicBlock *current_bb_;
        llvm::LLVMContext &ctx_;
        llvm::IRBuilder<> builder_;
    };
    
    //
    // Scan through the instruction stream to infer the control flow graph
    // and to figure out which arguments to be copied into memory, in order to
    // support ldarga / starg.
    //
    
    class OpcodeScanner
    {
    public:
        OpcodeScanner(CompilationEngine *engine, llvm::BasicBlock *prelude, VMMethod *method,
                      std::unordered_map<decil::IParameterDefinition *, llvm::Value*> &arguments,
                      std::unordered_map<int, OpcodeCompiler::BlockInfo> &block_info);
        void Scan();
        
    private:
        void ScanInstruction(decil::IOperation *op, bool *next_inst_as_bb);
        void CopyArgumentIntoMemory(decil::IParameterDefinition *loc);
        void RecordStartOfBasicBlock(int pos);
        CompilationEngine *engine_;
        llvm::BasicBlock *prelude_;
        VMMethod *method_;
        std::unordered_map<decil::IParameterDefinition *, llvm::Value*> &arguments_;
        std::unordered_map<int, OpcodeCompiler::BlockInfo> &block_info_;
    };
}
#endif

