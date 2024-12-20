/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-21
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"

namespace hku {

class TaBop : public IndicatorImp {
    INDICATOR_IMP(TaBop)
    INDICATOR_NEED_CONTEXT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    TaBop();
    explicit TaBop(const KData&);
    virtual ~TaBop() = default;
};

}  // namespace hku