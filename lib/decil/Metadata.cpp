//
//  Metadata.cpp
//  silk
//
//  Created by Haohui Mai on 11/25/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#include "Metadata.h"

namespace silk
{
    namespace decil
    {
        MDRowBase::MDRowBase()
        : row_index_(0)
        {}

        MDRowBase::~MDRowBase()
        {}
        
        MDTableBase::MDTableBase()
        : rows_(0)
        , sorted_(false)
        {}
        
        MDTableBase::~MDTableBase()
        {}
    }
}

