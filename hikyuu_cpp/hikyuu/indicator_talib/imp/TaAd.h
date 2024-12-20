/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"

namespace hku {

class TaAd : public IndicatorImp {
    INDICATOR_IMP(TaAd)
    INDICATOR_NEED_CONTEXT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    TaAd();
    explicit TaAd(const KData&);
    virtual ~TaAd() = default;
};

}  // namespace hku