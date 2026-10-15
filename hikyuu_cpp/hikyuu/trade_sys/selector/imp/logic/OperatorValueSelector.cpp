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
: SelectorBase(name), m_se(se), m_value(value) {
    if (m_se) {
        m_pro_sys_list = m_se->getProtoSystemList();
    }
}

OperatorValueSelector::~OperatorValueSelector() {}

void OperatorValueSelector::_reset() {
    if (m_se) {
        m_se->reset();
    }
}

SelectorPtr OperatorValueSelector::_clone() {
    auto p = make_shared<OperatorValueSelector>();
    if (m_se) {
        p->m_se = m_se->clone();
        // Keep the exposed prototype list on the same instances as the inner selector, otherwise
        // the clone would carry two different prototype instance sets
        p->m_pro_sys_list = p->m_se->getProtoSystemList();
    }
    p->m_value = m_value;
    return p;
}

void OperatorValueSelector::_addSystem(const SYSPtr& sys) {
    if (m_se) {
        m_se->addSystem(sys);
    }
}

void OperatorValueSelector::_removeAll() {
    if (m_se) {
        m_se->removeAll();
    }
}

void OperatorValueSelector::bindRealToProto(const SYSPtr& real, const SYSPtr& proto) {
    if (m_se) {
        m_se->bindRealToProto(real, proto);
    }
}

void OperatorValueSelector::_calculate() {
    if (m_se) {
        m_se->calculate(m_real_sys_list, m_query);
    }
}

}  // namespace hku