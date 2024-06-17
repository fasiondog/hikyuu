/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#include "OperatorInvertSubValueSelector.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::OperatorInvertSubValueSelector)
#endif

namespace hku {

SystemWeightList OperatorInvertSubValueSelector::getSelected(Datetime date) {
    SystemWeightList ret;
    HKU_IF_RETURN(!m_se, ret);

    ret = m_se->getSelected(date);
    for (auto& sw : ret) {
        sw.weight = m_value - sw.weight;
    }

    return ret;
}

HKU_API SelectorPtr operator-(double value, const SelectorPtr& se) {
    return make_shared<OperatorInvertSubValueSelector>(se, value);
}

}  // namespace hku