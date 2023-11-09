/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-11-09
 *      Author: fasiondog
 */

#include "ISlope.h"

namespace hku {

ISlope::ISlope() : IndicatorImp("SLOPE", 1) {
    setParam<int>("n", 22);
}

ISlope::~ISlope() {}

bool ISlope::check() {
    return getParam<int>("n") >= 1;
}

void ISlope::_calculate(const Indicator& ind) {}

}  // namespace hku