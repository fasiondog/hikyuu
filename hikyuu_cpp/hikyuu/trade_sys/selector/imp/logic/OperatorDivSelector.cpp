/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#include "OperatorDivSelector.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::OperatorDivSelector)
#endif

namespace hku {

SystemWeightList OperatorDivSelector::_getSelected(Datetime date) {
    // A zero / non-finite divisor leaves the weight undefined, report it as Null so that the
    // downstream allocation ignores it instead of producing inf / NaN
    return getIntersectionSelected(date, [](double w1, double w2) {
        return (w2 == 0.0 || !std::isfinite(w2)) ? Null<double>() : w1 / w2;
    });
}

HKU_API SelectorPtr operator/(const SelectorPtr& se1, const SelectorPtr& se2) {
    return make_shared<OperatorDivSelector>(se1, se2);
}

}  // namespace hku