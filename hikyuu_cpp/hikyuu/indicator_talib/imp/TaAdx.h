/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"

namespace hku {

class TaAdx : public IndicatorImp {
    INDICATOR_IMP(TaAdx)
    INDICATOR_NEED_CONTEXT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    TaAdx();
    explicit TaAdx(const KData&, int n);
    virtual ~TaAdx() = default;
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku