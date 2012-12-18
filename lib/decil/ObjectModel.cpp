//
//  ObjectModel.cpp
//  silk
//
//  Created by Haohui Mai on 12/7/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#include "silk/decil/ObjectModel.h"

#include <cassert>

namespace silk
{
    namespace decil
    {
        IOperation::~IOperation()
        {}
        
        ILOperand::ILOperand()
        : type_(kNull)
        {}
     
        int64_t ILOperand::GetInt() const
        { assert (type_ == kInt); return int64_; }
        float ILOperand::GetFloat() const
        { assert (type_ == kFloat); return float4_; }
        double ILOperand::GetDouble() const
        { assert (type_ == kDouble); return double8_; }
        IMetadata *ILOperand::GetMetadata() const
        { assert (type_ == kMetadata); return md_; }
        const std::u16string &ILOperand::GetString() const
        { assert (type_ == kString); return str_; }
        const std::vector<int> &ILOperand::GetIntArray() const
        { assert (type_ == kIntArray); return int_arr_; }
        
        void ILOperand::Clear()
        { type_ = kNull; }
        void ILOperand::SetInt(int64_t v)
        { type_ = kInt; int64_ = v; }
        void ILOperand::SetFloat(float v)
        { type_ = kFloat; float4_ = v; }
        void ILOperand::SetDouble(double v)
        { type_ = kDouble; double8_ = v; }
        void ILOperand::SetMetadata(IMetadata *v)
        { type_ = kMetadata; md_ = v; }
        void ILOperand::SetString(const std::u16string &v)
        { type_ = kString; str_ = v; }
        void ILOperand::SetIntArray(const std::vector<int> &v)
        { type_ = kIntArray; int_arr_ = v; }
    }
}
