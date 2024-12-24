/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-22
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"

namespace hku {

class TaStddev : public IndicatorImp {
    INDICATOR_IMP(TaStddev)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    TaStddev();
    virtual ~TaStddev() = default;
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku