/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-21
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"

namespace hku {

class TaCci : public IndicatorImp {
    INDICATOR_IMP(TaCci)
    INDICATOR_NEED_CONTEXT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    TaCci();
    explicit TaCci(const KData&, int n);
    virtual ~TaCci() = default;
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku