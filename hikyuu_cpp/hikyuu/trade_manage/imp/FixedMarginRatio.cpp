/*
 *  Copyright (c) 2022 hikyuu.org
 *
 *  Created on: 2022-09-01
 *      Author: fasiondog
 */

#include "FixedMarginRatio.h"

namespace hku {

FixedMarginRatio::FixedMarginRatio(double ratio) : MarginRatioBase("MR_Fixed"), m_ratio(ratio) {
    HKU_CHECK(ratio >= 0.0, "Invalid ratio: {}", ratio);
}

FixedMarginRatio::~FixedMarginRatio() {}

double FixedMarginRatio::getMarginRatio(Datetime datetime, const Stock& stk) const {
    return m_ratio;
}

MarginRatioPtr FixedMarginRatio::_clone() {
    return make_shared<FixedMarginRatio>(m_ratio);
}

MRPtr HKU_API MR_Fixed(double ratio) {
    return make_shared<FixedMarginRatio>(ratio);
}

}  // namespace hku