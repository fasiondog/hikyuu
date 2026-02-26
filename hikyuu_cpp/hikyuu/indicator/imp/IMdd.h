/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-24
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

class IMdd : public IndicatorImp {
    INDICATOR_IMP(IMdd)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IMdd();
    virtual ~IMdd() override;
    virtual void _checkParam(const string& name) const override;
    virtual bool supportIncrementCalculate() const override;
    virtual size_t min_increment_start() const override;
    virtual void _increment_calculate(const Indicator& ind, size_t start_pos) override;
};

}  // namespace hku