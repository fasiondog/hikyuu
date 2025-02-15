/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-07
 *      Author: fasiondog
 */

#pragma once

#include "../AllocateFundsBase.h"

namespace hku {

class FixedWeightListAllocateFunds : public AllocateFundsBase {
    ALLOCATEFUNDS_IMP(FixedWeightListAllocateFunds)
    ALLOCATEFUNDS_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedWeightListAllocateFunds();
    explicit FixedWeightListAllocateFunds(const PriceList& weights);
    virtual ~FixedWeightListAllocateFunds();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */
