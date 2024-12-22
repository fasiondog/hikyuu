/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"

namespace hku {

class TaUltosc : public IndicatorImp {
    INDICATOR_IMP(TaUltosc)
    INDICATOR_NEED_CONTEXT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    TaUltosc();
    explicit TaUltosc(const KData&, int n1, int n2, int n3);
    virtual ~TaUltosc() = default;
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku