/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"

namespace hku {

class TaHtPhasor : public IndicatorImp {
    INDICATOR_IMP(TaHtPhasor)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    TaHtPhasor();
    virtual ~TaHtPhasor() = default;
};

}  // namespace hku