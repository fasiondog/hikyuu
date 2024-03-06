/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-11-09
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

// 计算线性回归斜率，N支持变量
class ISlope : public IndicatorImp {
    INDICATOR_IMP_SUPPORT_DYNAMIC_STEP(ISlope)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ISlope();
    virtual ~ISlope();
};

}  // namespace hku
