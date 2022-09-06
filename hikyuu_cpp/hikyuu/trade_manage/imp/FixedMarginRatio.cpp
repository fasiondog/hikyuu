/*
 *  Copyright (c) 2022 hikyuu.org
 *
 *  Created on: 2022-09-01
 *      Author: fasiondog
 */

#include "FixedMarginRatio.h"

namespace hku {

FixedMarginRatio::FixedMarginRatio(const MarginRecord& mr)
: MarginRatioBase("MR_Fixed"), m_margin(mr) {
    HKU_CHECK(mr.initRatio > 0.0 && mr.initRatio <= 1.0, "Invalid ratio: {}", mr.initRatio);
    HKU_CHECK(mr.maintainRatio > 0.0 && mr.maintainRatio <= 1.0, "Invalid ratio: {}",
              mr.maintainRatio);
}

FixedMarginRatio::~FixedMarginRatio() {}

MarginRecord FixedMarginRatio::getMarginRatio(Datetime datetime, const Stock& stk) const {
    return m_margin;
}

MarginRatioPtr FixedMarginRatio::_clone() {
    return make_shared<FixedMarginRatio>(m_margin);
}

MRPtr HKU_API MR_Fixed(double initRatio, double maintainRatio) {
    return make_shared<FixedMarginRatio>(initRatio, maintainRatio);
}

}  // namespace hku