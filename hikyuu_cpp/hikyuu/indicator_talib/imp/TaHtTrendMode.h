/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-21
 *      Author: fasiondog
 */
#pragma once

#include "hikyuu/indicator/Indicator.h"

namespace hku {

class TaHtTrendMode : public IndicatorImp {
    INDICATOR_IMP(TaHtTrendMode)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    TaHtTrendMode();
    virtual ~TaHtTrendMode() = default;
};

}  // namespace hku