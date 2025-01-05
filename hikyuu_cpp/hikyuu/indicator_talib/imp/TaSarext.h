/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-22
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"

namespace hku {

class TaSarext : public IndicatorImp {
    INDICATOR_IMP(TaSarext)
    INDICATOR_NEED_CONTEXT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    TaSarext();
    TaSarext(double startvalue, double offsetonreverse, double accelerationinitlong,
             double accelerationlong, double accelerationmaxlong, double accelerationinitshort,
             double accelerationshort, double accelerationmaxshort);
    TaSarext(const KData&, double startvalue, double offsetonreverse, double accelerationinitlong,
             double accelerationlong, double accelerationmaxlong, double accelerationinitshort,
             double accelerationshort, double accelerationmaxshort);
    virtual ~TaSarext() = default;
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku