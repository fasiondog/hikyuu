/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-13
 *      Author: fasiondog
 */

#pragma once

#include "../ScoresFilterBase.h"

namespace hku {

class HKU_API MinAmountPercentSCFilter : public ScoresFilterBase {
    SCORESFILTER_IMP(MinAmountPercentSCFilter)
    SCORESFILTER_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    MinAmountPercentSCFilter();
    MinAmountPercentSCFilter(double min_amount_percent_limit);
    virtual ~MinAmountPercentSCFilter() = default;

    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku