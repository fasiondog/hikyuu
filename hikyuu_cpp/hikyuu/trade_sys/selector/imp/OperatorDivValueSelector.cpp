/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#include "OperatorDivValueSelector.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::OperatorDivValueSelector)
#endif

namespace hku {

SystemWeightList OperatorDivValueSelector::getSelected(Datetime date) {
    SystemWeightList ret;
    HKU_IF_RETURN(!m_se, ret);

    ret = m_se->getSelected(date);
    for (auto& sw : ret) {
        sw.weight /= m_value;
    }

    return ret;
}

HKU_API SelectorPtr operator/(const SelectorPtr& se, double value) {
    return make_shared<OperatorDivValueSelector>(se, value);
}

}  // namespace hku