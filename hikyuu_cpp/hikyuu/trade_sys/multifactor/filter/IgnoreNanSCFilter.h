/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-14
 *      Author: fasiondog
 */

#pragma once

#include "../ScoresFilterBase.h"

namespace hku {

class HKU_API IgnoreNanSCFilter : public ScoresFilterBase {
    SCORESFILTER_IMP(IgnoreNanSCFilter)
    SCORESFILTER_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IgnoreNanSCFilter();
    virtual ~IgnoreNanSCFilter() override = default;
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku