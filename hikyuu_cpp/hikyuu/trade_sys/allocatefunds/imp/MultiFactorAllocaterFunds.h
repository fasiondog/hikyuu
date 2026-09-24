/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-30
 *      Author: fasiondog
 */

#pragma once

#include "../AllocateFundsBase.h"

namespace hku {

/*
 * Create an asset allocation algorithm instance with the MultiFactor score weights, i.e. the scores
 * returned by SE are used as the weights directly.
 */
class MultiFactorAllocaterFunds : public AllocateFundsBase {
    ALLOCATEFUNDS_IMP(MultiFactorAllocaterFunds)
    ALLOCATEFUNDS_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    MultiFactorAllocaterFunds();
    virtual ~MultiFactorAllocaterFunds();
};

}  // namespace hku