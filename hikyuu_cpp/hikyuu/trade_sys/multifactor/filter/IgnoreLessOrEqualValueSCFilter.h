/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-14
 *      Author: fasiondog
 */

#pragma once

#include "../ScoresFilterBase.h"

namespace hku {

class HKU_API IgnoreLessOrEqualValueSCFilter : public ScoresFilterBase {
    SCORESFILTER_IMP(IgnoreLessOrEqualValueSCFilter)
    SCORESFILTER_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IgnoreLessOrEqualValueSCFilter();
    virtual ~IgnoreLessOrEqualValueSCFilter() override = default;
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku