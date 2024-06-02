/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#include "OperatorAddSelector.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::OperatorAddSelector)
#endif

namespace hku {

OperatorAddSelector::OperatorAddSelector() : SelectorBase("SE_Add") {}

OperatorAddSelector::OperatorAddSelector(const SelectorPtr& se1, const SelectorPtr& se2)
: SelectorBase("SE_Add") {
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

OperatorAddSelector::~OperatorAddSelector() {}

void OperatorAddSelector::_reset() {
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

bool OperatorAddSelector::isMatchAF(const AFPtr& af) {
    return true;
}

void OperatorAddSelector::_addSystem(const SYSPtr& sys) {
    if (m_se1) {
        m_se1->addSystem(sys);
        m_se1_set.insert(sys);
    }
    if (m_se2) {
        m_se2->addSystem(sys);
        m_se2_set.insert(sys);
    }
}

void OperatorAddSelector::_removeAll() {
    m_se1_set.clear();
    m_se2_set.clear();
}

SelectorPtr OperatorAddSelector::_clone() {
    auto p = make_shared<OperatorAddSelector>();
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

void OperatorAddSelector::_calculate() {
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

SystemWeightList OperatorAddSelector::getSelected(Datetime date) {
    SystemWeightList ret;
    SystemWeightList sws1, sws2;
    if (m_se1) {
        sws1 = m_se1->getSelected(date);
    }
    if (m_se2) {
        sws2 = m_se2->getSelected(date);
    }

    if (sws1.empty()) {
        ret = std::move(sws2);
        return ret;
    }

    if (sws2.empty()) {
        ret = std::move(sws1);
        return ret;
    }

    unordered_map<System*, SystemWeight*> sw_dict1;
    for (auto& sw : sws1) {
        sw_dict1[sw.sys.get()] = &sw;
    }

    SystemWeight tmp;
    unordered_map<System*, SystemWeight*> sw_dict2;
    unordered_map<System*, SystemWeight*>::iterator iter;
    for (auto& sw : sws2) {
        iter = sw_dict1.find(sw.sys.get());
        tmp.sys = sw.sys;
        if (iter != sw_dict1.end()) {
            tmp.weight = sw.weight + iter->second->weight;
        } else {
            tmp.weight = sw.weight;
        }
        auto& back = ret.emplace_back(std::move(tmp));
        sw_dict2[back.sys.get()] = &back;
    }

    for (auto& sw : sws1) {
        iter = sw_dict2.find(sw.sys.get());
        if (iter == sw_dict2.end()) {
            ret.emplace_back(std::move(sw));
        }
    }

    std::sort(ret.begin(), ret.end(), [](const SystemWeight& a, const SystemWeight& b) {
        if (std::isnan(a.weight) && std::isnan(b.weight)) {
            return false;
        } else if (!std::isnan(a.weight) && std::isnan(b.weight)) {
            return true;
        } else if (std::isnan(a.weight) && !std::isnan(b.weight)) {
            return false;
        }
        return a.weight > b.weight;
    });

    return ret;
}

HKU_API SelectorPtr operator+(const SelectorPtr& se1, const SelectorPtr& se2) {
    return make_shared<OperatorAddSelector>(se1, se2);
}

}  // namespace hku