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


SystemList FixedSelector::getSelectedSystemList(Datetime date) {
    return m_sys_list;
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
