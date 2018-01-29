/*
 * FixedSelector.cpp
 *
 *  Created on: 2018年1月12日
 *      Author: fasiondog
 */

#include "FixedSelector.h"

namespace hku {

FixedSelector::FixedSelector(): SelectorBase("SE_Fixed") {

}

FixedSelector::~FixedSelector() {

}


SystemWeightList FixedSelector::getSelectedSystemWeightList(Datetime date) {
    SystemWeightList result;
    auto iter = m_sys_list.begin();
    for(; iter != m_sys_list.end(); ++iter) {
        result.push_back(SystemWeight(*iter, 100));
    }
    return result;
}

SelectorPtr HKU_API SE_Fixed() {
    return make_shared<FixedSelector>();
}

SelectorPtr HKU_API SE_Fixed(const StockList& stock_list, const SystemPtr& sys) {
    SelectorPtr p = make_shared<FixedSelector>();
    p->addStockList(stock_list, sys);
    return p;
}

} /* namespace hku */
