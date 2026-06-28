/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-11-09
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/* 计算线性回归拟合优度R²，N支持变量 */
class IFitR2 : public IndicatorImp {
    INDICATOR_IMP(IFitR2)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IFitR2();
    virtual ~IFitR2() override;
    virtual void _checkParam(const string& name) const override;
    virtual bool supportIncrementCalculate() const override;
    virtual size_t min_increment_start() const override;
    virtual void _increment_calculate(const Indicator& ind, size_t start_pos) override;
};

}  // namespace hku