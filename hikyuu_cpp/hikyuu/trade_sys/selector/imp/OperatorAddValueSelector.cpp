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

OperatorAddValueSelector::OperatorAddValueSelector() : SelectorBase("SE_Add") {}

OperatorAddValueSelector::OperatorAddValueSelector(const SelectorPtr& se, double value)
: SelectorBase("SE_Add"), m_value(value) {
    if (se) {
        m_se = se->clone();
        m_se->removeAll();
    }
}

OperatorAddValueSelector::~OperatorAddValueSelector() {}

void OperatorAddValueSelector::_reset() {
    if (m_se) {
        m_se->reset();
    }
}

bool OperatorAddValueSelector::isMatchAF(const AFPtr& af) {
    return true;
}

SelectorPtr OperatorAddValueSelector::_clone() {
    OperatorAddValueSelector* p = new OperatorAddValueSelector();
    if (m_se) {
        p->m_se = m_se->clone();
    }
    return SelectorPtr(p);
}

void OperatorAddValueSelector::_calculate() {
    if (m_se) {
        m_se->calculate(m_real_sys_list, m_query);
    }
}

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