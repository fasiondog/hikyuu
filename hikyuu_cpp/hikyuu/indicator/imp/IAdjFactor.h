/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-05-17
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

// 后复权累计比例因子
class IAdjFactor : public IndicatorImp {
    INDICATOR_IMP(IAdjFactor)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IAdjFactor();
    virtual ~IAdjFactor() override;

    virtual bool supportIncrementCalculate() const override;
    virtual void _increment_calculate(const Indicator& ind, size_t start_pos) override;
};

} /* namespace hku */
