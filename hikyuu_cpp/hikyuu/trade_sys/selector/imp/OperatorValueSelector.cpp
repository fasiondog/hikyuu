/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#include "OperatorValueSelector.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::OperatorValueSelector)
#endif

namespace hku {

OperatorValueSelector::OperatorValueSelector() : SelectorBase("SE_OpearatorValue") {}

OperatorValueSelector::OperatorValueSelector(const string& name) : SelectorBase(name) {}

OperatorValueSelector::OperatorValueSelector(const string& name, const SelectorPtr& se,
                                             double value)
: SelectorBase(name), m_value(value) {
    if (se) {
        m_se = se->clone();
        m_pro_sys_list = m_se->getProtoSystemList();
    }
}

OperatorValueSelector::~OperatorValueSelector() {}

void OperatorValueSelector::_reset() {
    if (m_se) {
        m_se->reset();
    }
}

bool OperatorValueSelector::isMatchAF(const AFPtr& af) {
    return true;
}

SelectorPtr OperatorValueSelector::_clone() {
    auto p = make_shared<OperatorValueSelector>();
    if (m_se) {
        p->m_se = m_se->clone();
    }
    p->m_value = m_value;
    return p;
}

void OperatorValueSelector::_calculate() {
    if (m_se) {
        m_se->calculate(m_real_sys_list, m_query);
    }
}

}  // namespace hku