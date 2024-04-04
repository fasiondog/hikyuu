/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-30
 *      Author: fasiondog
 */

#pragma once

#include "../AllocateFundsBase.h"

namespace hku {

class MultiFactorAllocaterFunds : public AllocateFundsBase {
    ALLOCATEFUNDS_IMP(MultiFactorAllocaterFunds)
    ALLOCATEFUNDS_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    MultiFactorAllocaterFunds();
    virtual ~MultiFactorAllocaterFunds();
};

}  // namespace hku