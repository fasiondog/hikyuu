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

SystemWeightList SignalSelector::_getSelected(Datetime date) {
    auto iter = m_sys_dict.find(date);
    if (iter == m_sys_dict.end()) {
        return SystemWeightList();
    }

    SystemWeightList ret;
    ret.reserve(iter->second.size());
    for (const auto& sys : iter->second) {
        ret.emplace_back(sys, 1.0);
    }
    return ret;
}

void SignalSelector::_calculate() {
    // Only the signals the systems already have can be selected, so a mode C cycle sub-system
    // (declared on admission) is never seen here
    // Drop the previous selection first: a second calculate must not duplicate the systems of
    // the same day
    m_sys_dict.clear();

    // Only the systems are kept per date; the weight of a signal selection is always 1.0
    for (const auto& sys : m_real_sys_list) {
        DatetimeList dates = sys->getSG()->getBuySignal();
        for (const auto& date : dates) {
            m_sys_dict[date].push_back(sys);
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
