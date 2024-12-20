/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-21
 *      Author: fasiondog
 */
#pragma once

#include "hikyuu/indicator/Indicator.h"

namespace hku {

class TaHtSine : public IndicatorImp {
    INDICATOR_IMP(TaHtSine)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    TaHtSine();
    virtual ~TaHtSine() = default;
};

}  // namespace hku