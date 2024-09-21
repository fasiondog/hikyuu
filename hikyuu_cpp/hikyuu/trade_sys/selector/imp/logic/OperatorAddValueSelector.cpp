/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#include "OperatorAddValueSelector.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::OperatorAddValueSelector)
#endif

namespace hku {

SystemWeightList OperatorAddValueSelector::getSelected(Datetime date) {
    SystemWeightList ret;
    HKU_IF_RETURN(!m_se, ret);

    ret = m_se->getSelected(date);
    for (auto& sw : ret) {
        sw.weight += m_value;
    }

    return ret;
}

HKU_API SelectorPtr operator+(const SelectorPtr& se, double value) {
    return make_shared<OperatorAddValueSelector>(se, value);
}

}  // namespace hku