/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-22
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"

namespace hku {

class TaSar : public IndicatorImp {
    INDICATOR_IMP(TaSar)
    INDICATOR_NEED_CONTEXT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    TaSar();
    explicit TaSar(const KData&, double acceleration, double maximum);
    virtual ~TaSar() = default;
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku