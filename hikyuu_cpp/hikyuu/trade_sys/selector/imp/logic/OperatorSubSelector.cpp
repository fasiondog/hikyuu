/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#include "OperatorSubSelector.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::OperatorSubSelector)
#endif

namespace hku {

// A negative weight (e.g. an empty left operand, 0 - w) is intentional: the raw difference is kept
// for the downstream filter to flip or screen, so it is neither clamped nor dropped here
SystemWeightList OperatorSubSelector::_getSelected(Datetime date) {
    return getUnionSelected(date, [](double w1, double w2) { return w1 - w2; });
}

HKU_API SelectorPtr operator-(const SelectorPtr& se1, const SelectorPtr& se2) {
    return make_shared<OperatorSubSelector>(se1, se2);
}

}  // namespace hku