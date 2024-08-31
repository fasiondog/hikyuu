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
            for (const auto& sys2 : sys_list2) {
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
: SelectorBase(name), m_se1(se1), m_se2(se2) {
    build();
}

OperatorSelector::~OperatorSelector() {}

void OperatorSelector::build() {
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
    m_real_to_proto.clear();
}

SelectorPtr OperatorSelector::_clone() {
    HKU_THROW("OperatorSelector Could't support clone!");
#if 0    
    // OperatorSelector 不支持 clone 操作
    // 如果要实现 clone, 需要实现类似 indicator 一整套机制
    // 需要寻找最底层的系统策略实例，并在生成 clone 对象后，对原上传该系统实例也需要用该clone对象
    auto p = make_shared<OperatorSelector>();
    p->cloneRebuild(m_se1, m_se2);
    return p;
#endif
}

void OperatorSelector::cloneRebuild(const SelectorPtr& se1, const SelectorPtr& se2) {
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