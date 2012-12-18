//
//  VMMember.h
//  silk
//
//  Created by Haohui Mai on 12/5/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#ifndef SILK_LIB_VMCORE_VMMEMBER_H_
#define SILK_LIB_VMCORE_VMMEMBER_H_

#include "silk/decil/ObjectModel.h"

#include <string>
#include <unordered_map>

namespace llvm
{
    class Function;
}

namespace silk
{
    class VMClass;
    
    class VMMember
    {
    public:
        const std::u16string &name() const
        { return name_; }
    protected:
        VMMember(const std::u16string &name);
        virtual ~VMMember();
        std::u16string name_;
    };
    
    class VMField : public VMMember
    {
    public:
        VMClass *type() const
        { return type_; }
        unsigned offset() const
        { return offset_; }
        VMField(decil::IFieldDefinition *def, VMClass *type, unsigned offset);
        
        bool is_static() const
        { return def_->is_static(); }
        bool is_literal() const
        { return def_->is_literal(); }
        raw_istream field_mapping() const
        { return def_->field_mapping(); }
    private:
        decil::IFieldDefinition *def_;
        VMClass *type_;
        unsigned offset_;
    };
    
    class ParamInfo
    {
    public:
        const std::u16string &name() const
        { return name_; }
        VMClass *type() const
        { return type_; }
        decil::IParameterDefinition *param_def() const
        { return def_; }
        ParamInfo(const std::u16string &name, VMClass *type, decil::IParameterDefinition *def);
    private:
        std::u16string name_;
        decil::IParameterDefinition *def_;
        VMClass *type_;
    };

    //
    // Class to represent a CIL method.
    // If the method is an instance method, it includes the `this`
    // parameter as the parameter 0.
    //
    class VMMethod : public VMMember
    {
    public:
        friend class VMNamedClassBase;
        VMMethod(decil::IMethodDefinition *def);

        llvm::Function *implementation() const
        { return implementation_; }
        
        const std::u16string &mangled_name();
        
        VMClass *return_type()
        { return return_type_; }
        
        decil::IMethodDefinition *method_def() const
        { return def_; }
        
        const ParamInfo &get_param(int idx) const
        { return params_.at(idx); }
        
        std::vector<ParamInfo>::const_iterator param_begin() const
        { return params_.begin(); }

        std::vector<ParamInfo>::const_iterator param_end() const
        { return params_.end(); }
        
        bool has_implicit_this() const
        { return has_implicit_this_; }
        
    private:
        llvm::Function *implementation_;
        decil::IMethodDefinition *def_;
        std::vector<ParamInfo> params_;
        VMClass *return_type_;
        std::u16string mangled_name_;
        bool has_implicit_this_;
    };
}

#endif

