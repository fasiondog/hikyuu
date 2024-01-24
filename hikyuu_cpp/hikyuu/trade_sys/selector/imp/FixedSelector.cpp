/*
 * FixedSelector.cpp
 *
 *  Created on: 2018年1月12日
 *      Author: fasiondog
 */

#include "FixedSelector.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedSelector)
#endif

namespace hku {

FixedSelector::FixedSelector() : SelectorBase("SE_Fixed") {}

FixedSelector::~FixedSelector() {}

bool FixedSelector::isMatchAF(const AFPtr& af) {
    return true;
}

SystemList FixedSelector::getSelectedOnOpen(Datetime date) {
    SystemList result;
    for (auto& sys : m_real_sys_list) {
        if (!sys->getParam<bool>("buy_delay")) {
            result.emplace_back(sys);
        }
    }
    return result;
}

SystemList FixedSelector::getSelectedOnClose(Datetime date) {
    SystemList result;
    for (auto& sys : m_real_sys_list) {
        if (sys->getParam<bool>("buy_delay")) {
            result.emplace_back(sys);
        }
    }
    return result;
}

void FixedSelector::_calculate() {}

SelectorPtr HKU_API SE_Fixed() {
    return make_shared<FixedSelector>();
}

SelectorPtr HKU_API SE_Fixed(const StockList& stock_list, const SystemPtr& sys) {
    SelectorPtr p = make_shared<FixedSelector>();
    p->addStockList(stock_list, sys);
    return p;
}

} /* namespace hku */
