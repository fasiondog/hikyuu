/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-21
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"

namespace hku {

class TaMidPrice : public IndicatorImp {
    INDICATOR_IMP(TaMidPrice)
    INDICATOR_NEED_CONTEXT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    TaMidPrice();
    explicit TaMidPrice(const KData&, int n);
    virtual ~TaMidPrice() = default;
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku