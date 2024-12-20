/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-21
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"

namespace hku {

class TaAtr : public IndicatorImp {
    INDICATOR_IMP(TaAtr)
    INDICATOR_NEED_CONTEXT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    TaAtr();
    explicit TaAtr(const KData&, int n);
    virtual ~TaAtr() = default;
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku