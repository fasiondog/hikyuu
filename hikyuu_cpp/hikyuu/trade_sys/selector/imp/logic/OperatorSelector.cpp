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
        std::unordered_set<System*> dict;
        dict.reserve(sys_list1.size());
        for (const auto& sys : sys_list1) {
            dict.insert(sys.get());
        }
        for (const auto& sys : sys_list2) {
            if (dict.count(sys.get())) {
                ret.insert(sys.get());
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
: SelectorBase(name), m_se1(se1), m_se2(se2) {
    build();
}

OperatorSelector::~OperatorSelector() {}

void OperatorSelector::build() {
    // Idempotent: the base class load has already restored m_pro_sys_list, build must rebuild
    // instead of appending, otherwise the prototype list doubles after a round-trip
    m_pro_sys_list.clear();
    m_se1_set.clear();
    m_se2_set.clear();

    auto inter = findIntersection(m_se1, m_se2);
    if (m_se1 && m_se2) {
        std::map<System*, SYSPtr> tmpdict;
        const auto& raw_sys_list1 = m_se1->getProtoSystemList();
        for (const auto& sys : raw_sys_list1) {
            m_pro_sys_list.emplace_back(sys);
            m_se1_set.insert(sys);
            if (inter.find(sys.get()) != inter.end()) {
                tmpdict[sys.get()] = sys;
            }
        }

        const auto& raw_sys_list2 = m_se2->getProtoSystemList();
        for (size_t i = 0, total = raw_sys_list2.size(); i < total; i++) {
            const auto& sys = raw_sys_list2[i];
            auto iter = inter.find(sys.get());
            if (iter == inter.end()) {
                m_pro_sys_list.emplace_back(sys);
                m_se2_set.insert(sys);
            } else {
                m_se2_set.insert(tmpdict[*iter]);
            }
        }

    } else if (m_se1) {
        // m_se1 = se1->clone();
        auto sys_list = m_se1->getProtoSystemList();
        for (auto& sys : sys_list) {
            m_se1_set.insert(sys);
        }
        m_pro_sys_list = std::move(sys_list);

    } else if (m_se2) {
        // m_se2 = se2->clone();
        auto sys_list = m_se2->getProtoSystemList();
        for (auto& sys : sys_list) {
            m_se2_set.insert(sys);
            m_pro_sys_list.emplace_back(std::move(sys));
        }
    }
}

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
    m_real_to_proto.clear();
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
    m_real_to_proto.clear();
    // A nested logical selector (e.g. (a + b) - c) must clear its own operands too, otherwise the
    // sub-operands would keep their stale prototypes after removeAll
    if (m_se1) {
        m_se1->removeAll();
    }
    if (m_se2) {
        m_se2->removeAll();
    }
}

SelectorPtr OperatorSelector::_clone() {
    auto p = make_shared<OperatorSelector>();
    p->cloneRebuild(m_se1, m_se2);
    return p;
}

void OperatorSelector::cloneRebuild(const SelectorPtr& se1, const SelectorPtr& se2) {
    auto inter = findIntersection(se1, se2);
    if (se1 && se2) {
        m_se1 = se1->clone();
        // Take over the prototypes cloned by the operand's clone instead of re-cloning them (the
        // old code discarded them via removeAll and cloned every prototype again)
        SystemList proto1 = m_se1->getProtoSystemList();
        m_se1->removeAll();
        m_se2 = se2->clone();
        SystemList proto2 = m_se2->getProtoSystemList();
        m_se2->removeAll();

        std::map<System*, SYSPtr> tmpdict;
        const auto& raw_sys_list1 = se1->getProtoSystemList();
        for (size_t i = 0, total = proto1.size(); i < total; i++) {
            const SYSPtr& tmpsys = proto1[i];
            m_pro_sys_list.emplace_back(tmpsys);
            m_se1->addSystem(tmpsys);
            m_se1_set.insert(tmpsys);
            if (inter.find(raw_sys_list1[i].get()) != inter.end()) {
                tmpdict[raw_sys_list1[i].get()] = tmpsys;
            }
        }

        const auto& raw_sys_list2 = se2->getProtoSystemList();
        for (size_t i = 0, total = proto2.size(); i < total; i++) {
            auto iter = inter.find(raw_sys_list2[i].get());
            if (iter == inter.end()) {
                const SYSPtr& tmpsys = proto2[i];
                m_pro_sys_list.emplace_back(tmpsys);
                m_se2_set.insert(tmpsys);
                m_se2->addSystem(tmpsys);
            } else {
                // The shared prototype must be the very instance adopted on the se1 side, otherwise
                // the clone loses the intersection and a later reset would split the two sets
                const SYSPtr& tmpsys = tmpdict[raw_sys_list2[i].get()];
                m_se2_set.insert(tmpsys);
                m_se2->addSystem(tmpsys);
            }
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

void OperatorSelector::_calculate() {
    SystemList se1_list, se2_list;
    // A large portfolio can leave thousands of systems unbound; one warning per system would flood
    // the log, so report the count with a few samples once per calculation instead.
    size_t unbound_count = 0;
    string unbound_samples;
    for (const auto& sys : m_real_sys_list) {
        auto iter = m_real_to_proto.find(sys);
        if (iter == m_real_to_proto.end() || !iter->second) {
            ++unbound_count;
            if (unbound_count <= 3) {
                if (!unbound_samples.empty()) {
                    unbound_samples += ", ";
                }
                unbound_samples += sys->name();
            }
            continue;
        }
        const auto& protoSys = iter->second;
        if (m_se1_set.find(protoSys) != m_se1_set.end()) {
            se1_list.emplace_back(sys);
        }
        if (m_se2_set.find(protoSys) != m_se2_set.end()) {
            se2_list.emplace_back(sys);
        }
    }

    if (unbound_count > 0) {
        HKU_WARN("{} system(s) are not bound to a prototype and were ignored! e.g. [{}]",
                 unbound_count, unbound_samples);
    }

    if (m_se1) {
        m_se1->calculate(se1_list, m_query);
    }
    if (m_se2) {
        m_se2->calculate(se2_list, m_query);
    }
}

SystemWeightList OperatorSelector::getUnionSelected(
  Datetime date, const std::function<double(double, double)>&& func) {
    SystemWeightList ret;
    SystemWeightList sws1, sws2;
    if (m_se1) {
        sws1 = m_se1->getSelected(date);
    }
    if (m_se2) {
        sws2 = m_se2->getSelected(date);
    }

    if (sws1.empty()) {
        for (const auto& sw : sws2) {
            ret.emplace_back(SystemWeight(sw.sys, func(0, sw.weight)));
        }
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
            tmp.weight = std::isnan(sw.weight) ? sw.weight : func(iter->second->weight, sw.weight);
        } else {
            tmp.weight = func(0.0, sw.weight);
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

    sortSystemWeightList(ret);
    return ret;
}

SystemWeightList OperatorSelector::getIntersectionSelected(
  Datetime date, const std::function<double(double, double)>&& func) {
    SystemWeightList ret;
    HKU_IF_RETURN(!m_se1 || !m_se2, ret);

    SystemWeightList sws1 = m_se1->getSelected(date);
    SystemWeightList sws2 = m_se2->getSelected(date);

    HKU_IF_RETURN(sws1.empty() || sws2.empty(), ret);

    unordered_map<System*, SystemWeight*> sw_dict1;
    for (auto& sw : sws1) {
        sw_dict1[sw.sys.get()] = &sw;
    }

    SystemWeight tmp;
    unordered_map<System*, SystemWeight*>::iterator iter;
    for (auto& sw : sws2) {
        iter = sw_dict1.find(sw.sys.get());
        if (iter != sw_dict1.end()) {
            tmp.sys = sw.sys;
            tmp.weight = func(iter->second->weight, sw.weight);
            ret.emplace_back(std::move(tmp));
        }
    }

    sortSystemWeightList(ret);
    return ret;
}

}  // namespace hku