/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-21
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"

namespace hku {

class TaAroon : public IndicatorImp {
    INDICATOR_IMP(TaAroon)
    INDICATOR_NEED_CONTEXT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    TaAroon();
    explicit TaAroon(const KData&, int n);
    virtual ~TaAroon() = default;
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku