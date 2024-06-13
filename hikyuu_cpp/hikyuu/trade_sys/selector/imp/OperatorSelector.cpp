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

std::unordered_set<System*> OperatorSelector::findIntersection(const SelectorPtr& se1,
                                                               const SelectorPtr& se2) {
    std::unordered_set<System*> ret;
    if (se1 && se2) {
        const auto& sys_list1 = se1->getProtoSystemList();
        const auto& sys_list2 = se2->getProtoSystemList();
        for (const auto& sys1 : sys_list1) {
            for (const auto sys2 : sys_list2) {
                if (sys1 == sys2) {
                    ret.insert(sys1.get());
                }
            }
        }
    }
    return ret;
}

void OperatorSelector::sortSystemWeightList(SystemWeightList& swlist) {
    std::sort(swlist.begin(), swlist.end(), [](const SystemWeight& a, const SystemWeight& b) {
        if (std::isnan(a.weight) && std::isnan(b.weight)) {
            return false;
        } else if (!std::isnan(a.weight) && std::isnan(b.weight)) {
            return true;
        } else if (std::isnan(a.weight) && !std::isnan(b.weight)) {
            return false;
        }
        return a.weight > b.weight;
    });
}

OperatorSelector::OperatorSelector() : SelectorBase("SE_Operator") {}

OperatorSelector::OperatorSelector(const string& name) : SelectorBase(name) {}

OperatorSelector::OperatorSelector(const string& name, const SelectorPtr& se1,
                                   const SelectorPtr& se2)
: SelectorBase(name) {
    auto inter = findIntersection(se1, se2);
    if (se1 && se2) {
        m_se1 = se1->clone();
        m_se1->removeAll();
        m_se2 = se2->clone();
        m_se2->removeAll();

        std::map<System*, SYSPtr> tmpdict;
        const auto& raw_sys_list1 = se1->getProtoSystemList();
        for (const auto& sys : raw_sys_list1) {
            auto tmpsys = sys->clone();
            m_pro_sys_list.emplace_back(tmpsys);
            m_se1->addSystem(tmpsys);
            m_se1_set.insert(tmpsys);
            if (inter.find(sys.get()) != inter.end()) {
                tmpdict[sys.get()] = tmpsys;
            }
        }

        const auto& raw_sys_list2 = se2->getProtoSystemList();
        for (size_t i = 0, total = raw_sys_list2.size(); i < total; i++) {
            const auto& sys = raw_sys_list2[i];
            auto tmpsys = sys->clone();
            auto iter = inter.find(sys.get());
            if (iter == inter.end()) {
                m_pro_sys_list.emplace_back(tmpsys);
                m_se2_set.insert(tmpsys);
            } else {
                m_se2_set.insert(tmpdict[*iter]);
            }
            m_se2->addSystem(tmpsys);
        }

    } else if (se1) {
        m_se1 = se1->clone();
        auto sys_list = m_se1->getProtoSystemList();
        for (auto& sys : sys_list) {
            m_se1_set.insert(sys);
        }
        m_pro_sys_list = std::move(sys_list);

    } else if (se2) {
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