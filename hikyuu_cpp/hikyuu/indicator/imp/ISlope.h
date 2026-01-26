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
    INDICATOR_IMP(ISlope)
    INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ISlope();
    virtual ~ISlope();
    virtual void _checkParam(const string& name) const override;
    virtual bool supportIncrementCalculate() const override;
    virtual size_t min_increment_start() const override;
    virtual void _increment_calculate(const Indicator& ind, size_t start_pos) override;
};

}  // namespace hku
