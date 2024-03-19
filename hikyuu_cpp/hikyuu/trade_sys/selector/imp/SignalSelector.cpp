/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2022-02-19
 *      Author: fasiondog
 */

#include "SignalSelector.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::SignalSelector)
#endif

namespace hku {

SignalSelector::SignalSelector() : SelectorBase("SE_Sigal") {}

SignalSelector::~SignalSelector() {}

bool SignalSelector::isMatchAF(const AFPtr& af) {
    HKU_WARN_IF_RETURN(
      af->getParam<bool>("adjust_running_sys"), false,
      "AF will adjust running system funds, but this se is not suitable the case!");
    return true;
}

SystemWeightList SignalSelector::getSelected(Datetime date) {
    auto iter = m_sys_dict.find(date);
    return iter != m_sys_dict.end() ? iter->second : SystemWeightList();
}

void SignalSelector::_calculate() {
    size_t total = m_real_sys_list.size();
    for (size_t i = 0; i < total; i++) {
        auto& sys = m_real_sys_list[i];
        auto sg = sys->getSG();
        auto dates = sg->getBuySignal();
        for (auto& date : dates) {
            auto iter = m_sys_dict.find(date);
            if (iter != m_sys_dict.end()) {
                iter->second.emplace_back(sys, 1.0);
            } else {
                m_sys_dict[date] = {SystemWeight(sys, 1.0)};
            }
        }
    }
}

SEPtr HKU_API SE_Signal() {
    return make_shared<SignalSelector>();
}

SEPtr HKU_API SE_Signal(const StockList& stock_list, const SystemPtr& sys) {
    SelectorPtr p = make_shared<SignalSelector>();
    p->addStockList(stock_list, sys);
    return p;
}

}  // namespace hku