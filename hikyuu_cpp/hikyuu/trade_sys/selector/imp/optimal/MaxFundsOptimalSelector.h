/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-22
 *      Author: fasiondog
 */

#pragma once
#ifndef HKU_TRADE_SYS_MAX_FUNDS_OPTIMAL_SELECTOR_H_
#define HKU_TRADE_SYS_MAX_FUNDS_OPTIMAL_SELECTOR_H_

#include "OptimalSelectorBase.h"

namespace hku {

class MaxFundsOptimalSelector : public OptimalSelectorBase {
    OPTIMAL_SELECTOR_IMP(MaxFundsOptimalSelector)
    OPTIMAL_SELECTOR_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    MaxFundsOptimalSelector();
    virtual ~MaxFundsOptimalSelector();
};

}  // namespace hku

#endif /* HKU_TRADE_SYS_MAX_FUNDS_OPTIMAL_SELECTOR_H_ */