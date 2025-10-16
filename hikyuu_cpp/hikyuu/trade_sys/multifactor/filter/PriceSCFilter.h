/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-13
 *      Author: fasiondog
 */

#pragma once

#include "../ScoresFilterBase.h"

namespace hku {

class HKU_API PriceSCFilter : public ScoresFilterBase {
    SCORESFILTER_IMP(PriceSCFilter)
    SCORESFILTER_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    PriceSCFilter();
    virtual ~PriceSCFilter() override = default;
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku