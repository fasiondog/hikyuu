/*
 *  Copyright (c) 2022 hikyuu.org
 *
 *  Created on: 2022-09-01
 *      Author: fasiondog
 */

#include "FixedMarginRatio.h"

namespace hku {

FixedMarginRatio::FixedMarginRatio(double ratio) : MarginRatioBase("MR_Fixed"), m_margin(ratio) {
    setParam<double>("ratio", ratio);
    HKU_CHECK(ratio > 0.0 && ratio <= 1.0, "Invalid ratio: {}", ratio);
}

FixedMarginRatio::~FixedMarginRatio() {}

double FixedMarginRatio::getMarginRatio(Datetime datetime, const Stock& stk) const {
    return m_margin;
}

MarginRatioPtr FixedMarginRatio::_clone() {
    return make_shared<FixedMarginRatio>(m_margin);
}

MRPtr HKU_API MR_Fixed(double initRatio) {
    return make_shared<FixedMarginRatio>(initRatio);
}

}  // namespace hku