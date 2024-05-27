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
        m_se1->removeAll();
    }
    if (se2) {
        m_se2 = se2->clone();
        m_se2->removeAll();
    }
}

OperatorAddSelector::~OperatorAddSelector() {}

void OperatorAddSelector::_reset() {
    if (m_se1) {
        m_se1->reset();
    }
    if (m_se2) {
        m_se2->reset();
    }
}

bool OperatorAddSelector::isMatchAF(const AFPtr& af) {
    return true;
}

SelectorPtr OperatorAddSelector::_clone() {
    OperatorAddSelector* p = new OperatorAddSelector();
    if (m_se1) {
        p->m_se1 = m_se1->clone();
    }
    if (m_se2) {
        p->m_se2 = m_se2->clone();
    }
    return SelectorPtr(p);
}

void OperatorAddSelector::_calculate() {
    if (m_se1) {
        m_se1->calculate(m_real_sys_list, m_query);
    }
    if (m_se2) {
        m_se2->calculate(m_real_sys_list, m_query);
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