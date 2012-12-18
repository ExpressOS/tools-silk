//
//  VMClass.h
//  silk
//
//  Created by Haohui Mai on 12/2/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#ifndef SILK_LIB_VMCORE_VMCLASS_H_
#define SILK_LIB_VMCORE_VMCLASS_H_

#include "silk/decil/ObjectModel.h"

#include <llvm/Type.h>
#include <llvm/DerivedTypes.h>

#include <unordered_map>

namespace silk
{
    class CompilationEngine;
    class VMField;
    class VMMethod;
    //
    // Each CIL type is represented by an instance of VMClass.
    // VMClass contains three LLVM types, which are:
    //
    //   (1) Physical type. It represents the physical layout of the CIL type.
    //
    //   (2) Normal type. The type that uses in general contexts, for example,
    //       function calls or declarations. It's a pointer to the layout type
    //       when the corresponding CIL type is an object, or it's the same as
    //       the layout type if the CIL type is a ValueType.
    //
    //   (3) Boxed type. It is only defined when the CIL type is a valuetype.
    //       It's the boxed version of the layout type.
    //
    //
    // The initialization happens in two steps. First, the compilation engine
    // creates the VMClass instance, and registers it into its type cache.
    // Second, it invokes the Layout() function to do the real work. That way
    // enables the support of recursive data structure such as linked list.
    //
    class VMClass
    {
    public:
        VMClass(CompilationEngine *engine);
        virtual ~VMClass();
        void LayoutIfNecessary();

        enum class State
        {
            kUninitialized,
            kInitialized,
        };
        State state() const { return state_; }
        
        virtual decil::INamedTypeDefinition::TypeCode type_code() const;

        
        llvm::Type *physical_type() { return physical_type_; }
        llvm::Type *normal_type()   { return normal_type_; }
        llvm::Type *boxed_type()    { return boxed_type_; }

        virtual bool IsValueType() const = 0;
        
        const std::u16string &name() const
        { return name_; }
        
        typedef std::unordered_map<std::u16string, VMField*>::const_iterator field_const_iterator;
        typedef std::unordered_map<std::u16string, VMMethod*>::const_iterator method_const_iterator;
        
        field_const_iterator field_begin() const
        { return fields_.begin(); }
        field_const_iterator field_end() const
        { return fields_.end(); }
        method_const_iterator method_begin() const
        { return methods_.begin(); }
        method_const_iterator method_end() const
        { return methods_.end(); }
        
        VMField *GetField(const std::u16string &name) const;
        VMMethod *GetMethod(const std::u16string &name) const;
        llvm::Value *static_instance() const
        { return static_instance_; }
        
    protected:
        virtual void Layout() = 0;

        State state_;
        CompilationEngine *engine_;
        
        llvm::Type *physical_type_;
        llvm::Type *normal_type_;
        llvm::Type *boxed_type_;
        
        // Value to store static fields
        llvm::Value *static_instance_;
        
        std::u16string name_;
        std::unordered_map<std::u16string, VMField*> fields_;
        std::unordered_map<std::u16string, VMMethod*> methods_;
    };
    
    class VMNamedClassBase : public VMClass
    {
    public:
        decil::INamedTypeDefinition *type_def() const
        { return type_def_; }
    protected:
        VMNamedClassBase(CompilationEngine *engine, decil::INamedTypeDefinition *type_def);
        virtual decil::INamedTypeDefinition::TypeCode type_code() const;

        void LoadVMFields();
        void LoadVMMethods();
        void LoadStaticFields();
        
        virtual bool IsValueType() const;
        bool IncludeBaseClass() const;
        
        void RefineLLVMType(llvm::StructType *type, bool include_base_class, std::function<bool(const VMField*)> filter);
        
        decil::INamedTypeDefinition *type_def_;

    };
    
    class VMPrimitiveClass : public VMNamedClassBase
    {
    public:
        static VMPrimitiveClass *CreateVMPrimitiveClass(CompilationEngine *engine,
                                                        decil::INamedTypeDefinition *type_def);
        virtual void Layout() override;
        
    private:
        VMPrimitiveClass(CompilationEngine *engine, decil::INamedTypeDefinition *type_def);
    };
    
    class VMNamedClass : public VMNamedClassBase
    {
    public:
        VMNamedClass(CompilationEngine *engine, decil::INamedTypeDefinition *type_def);
        virtual void Layout() override;
    };
    
    class VMClassEnum : public VMNamedClassBase
    {
    public:
        VMClassEnum(CompilationEngine *engine, decil::INamedTypeDefinition *type_def);
        virtual void Layout() override final;
        virtual bool IsValueType() const override final
        { return true; }
    };
    
    class VMClassPointer : public VMClass
    {
    public:
        virtual void Layout() override;
        virtual bool IsValueType() const override final { return is_void_star_type_; }

    private:
        friend class CompilationEngine;
        VMClassPointer(CompilationEngine *engine, VMClass *target_type);
        VMClass *target_type_;
        bool is_void_star_type_;
    };

    class VMClassVector : public VMClass
    {
    public:
        VMClassVector(CompilationEngine *engine, VMClass *element_type);
        virtual void Layout() override;
        virtual bool IsValueType() const override final { return false; }
        VMClass *element_type() const
        { return element_type_; }
    private:
        VMClass *element_type_;

    };
}

#endif

