/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#include "OperatorSelector.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::OperatorSelector)
#endif

namespace hku {

OperatorSelector::OperatorSelector() : SelectorBase("SE_Operator") {}

OperatorSelector::OperatorSelector(const string& name) : SelectorBase(name) {}

OperatorSelector::OperatorSelector(const string& name, const SelectorPtr& se1,
                                   const SelectorPtr& se2)
: SelectorBase(name) {
    if (se1) {
        m_se1 = se1->clone();
        auto sys_list = m_se1->getProtoSystemList();
        for (auto& sys : sys_list) {
            m_se1_set.insert(sys);
        }
        m_pro_sys_list = std::move(sys_list);
    }
    if (se2) {
        m_se2 = se2->clone();
        auto sys_list = m_se2->getProtoSystemList();
        for (auto& sys : sys_list) {
            m_se2_set.insert(sys);
            m_pro_sys_list.emplace_back(std::move(sys));
        }
    }
}

OperatorSelector::~OperatorSelector() {}

void OperatorSelector::_reset() {
    if (m_se1) {
        m_se1->reset();
        m_se1_set.clear();
        auto sys_list = m_se1->getProtoSystemList();
        for (auto& sys : sys_list) {
            m_se1_set.insert(sys);
        }
    }
    if (m_se2) {
        m_se2->reset();
        m_se2_set.clear();
        auto sys_list = m_se2->getProtoSystemList();
        for (auto& sys : sys_list) {
            m_se2_set.insert(sys);
        }
    }
}

bool OperatorSelector::isMatchAF(const AFPtr& af) {
    return true;
}

void OperatorSelector::_addSystem(const SYSPtr& sys) {
    if (m_se1) {
        m_se1->addSystem(sys);
        m_se1_set.insert(sys);
    }
    if (m_se2) {
        m_se2->addSystem(sys);
        m_se2_set.insert(sys);
    }
}

void OperatorSelector::_removeAll() {
    m_se1_set.clear();
    m_se2_set.clear();
}

SelectorPtr OperatorSelector::_clone() {
    auto p = make_shared<OperatorSelector>();
    if (m_se1) {
        p->m_se1 = m_se1->clone();
        auto sys_list = p->m_se1->getProtoSystemList();
        for (auto& sys : sys_list) {
            p->m_se1_set.insert(sys);
        }
    }
    if (m_se2) {
        p->m_se2 = m_se2->clone();
        auto sys_list = p->m_se2->getProtoSystemList();
        for (auto& sys : sys_list) {
            p->m_se2_set.insert(sys);
        }
    }
    return p;
}

void OperatorSelector::_calculate() {
    SystemList se1_list, se2_list;
    for (const auto& sys : m_real_sys_list) {
        const auto& protoSys = m_real_to_proto[sys];
        if (m_se1_set.find(protoSys) != m_se1_set.end()) {
            se1_list.emplace_back(sys);
        }
        if (m_se2_set.find(protoSys) != m_se2_set.end()) {
            se2_list.emplace_back(sys);
        }
    }
    if (m_se1) {
        m_se1->calculate(se1_list, m_query);
    }
    if (m_se2) {
        m_se2->calculate(se2_list, m_query);
    }
}

}  // namespace hku