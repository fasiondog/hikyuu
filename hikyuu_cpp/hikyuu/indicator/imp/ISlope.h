/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-11-09
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

// Calculate the linear regression slope, the goodness of fit R² and the relative maximum
// residual; N supports a variable
// Result set:
//   result(0): slope
//   result(1): goodness of fit R²
//   result(2): relative maximum residual RelMaxRes = max|yi - ŷi| / ȳ
class ISlope : public IndicatorImp {
    INDICATOR_IMP(ISlope)
    INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ISlope();
    virtual ~ISlope() override;
    virtual void _checkParam(const string& name) const override;
    virtual bool supportIncrementCalculate() const override;
    virtual size_t min_increment_start() const override;
    virtual void _increment_calculate(const Indicator& ind, size_t start_pos) override;
};

}  // namespace hku