/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"

namespace hku {

class TaStoch : public IndicatorImp {
    INDICATOR_IMP(TaStoch)
    INDICATOR_NEED_CONTEXT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    TaStoch();
    explicit TaStoch(const KData&, int fastk_n, int slowk_n, int slowk_matype, int slowd_n,
                     int slowd_matype);
    virtual ~TaStoch() = default;
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku