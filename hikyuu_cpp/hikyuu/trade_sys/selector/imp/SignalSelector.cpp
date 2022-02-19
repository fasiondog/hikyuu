/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2022-02-19
 *      Author: fasiondog
 */

#include "SignalSelector.h"

namespace hku {

SignalSelector::SignalSelector() : SelectorBase("SE_Sigal") {}

SignalSelector::~SignalSelector() {}

SystemList SignalSelector::getSelectedSystemList(Datetime date) {
    auto iter = m_date_sys_dict.find(date);
    return iter != m_date_sys_dict.end() ? iter->second : SystemList();
}

void SignalSelector::_calculate() {
    size_t total = m_real_sys_list.size();
    for (size_t i = 0; i < total; i++) {
        auto& sys = m_real_sys_list[i];
        auto sg = sys->getSG();
        auto dates = sg->getBuySignal();
        for (auto& date : dates) {
            auto iter = m_date_sys_dict.find(date);
            if (iter != m_date_sys_dict.end()) {
                iter->second.emplace_back(sys);
            } else {
                m_date_sys_dict[date] = {sys};
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