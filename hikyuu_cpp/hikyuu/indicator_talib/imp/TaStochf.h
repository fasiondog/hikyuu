/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"

namespace hku {

class TaStochf : public IndicatorImp {
    INDICATOR_IMP(TaStochf)
    INDICATOR_NEED_CONTEXT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    TaStochf();
    explicit TaStochf(const KData&, int fastk_n, int fastd_n, int fastd_matype);
    virtual ~TaStochf() = default;
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku