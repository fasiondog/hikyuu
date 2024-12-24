/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"

namespace hku {

class TaBbands : public IndicatorImp {
    INDICATOR_IMP(TaBbands)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    TaBbands();
    virtual ~TaBbands() = default;
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku