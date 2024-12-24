/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"

namespace hku {

class TaAdosc : public IndicatorImp {
    INDICATOR_IMP(TaAdosc)
    INDICATOR_NEED_CONTEXT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    TaAdosc();
    explicit TaAdosc(const KData&, int fast_n, int slow_n);
    virtual ~TaAdosc() = default;
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku