/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-21
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"

namespace hku {

class TaMedPrice : public IndicatorImp {
    INDICATOR_IMP(TaMedPrice)
    INDICATOR_NEED_CONTEXT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    TaMedPrice();
    explicit TaMedPrice(const KData&);
    virtual ~TaMedPrice() = default;
};

}  // namespace hku