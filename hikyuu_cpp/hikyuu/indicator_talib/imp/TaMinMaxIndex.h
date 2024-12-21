/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-21
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"

namespace hku {

class TaMinMaxIndex : public IndicatorImp {
    INDICATOR_IMP(TaMinMaxIndex)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    TaMinMaxIndex();
    virtual ~TaMinMaxIndex() = default;
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku