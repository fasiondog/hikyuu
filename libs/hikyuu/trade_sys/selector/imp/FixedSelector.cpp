/*
 * FixedSelector.cpp
 *
 *  Created on: 2018年1月12日
 *      Author: fasiondog
 */

#include "FixedSelector.h"

namespace hku {

FixedSelector::FixedSelector() {

}

FixedSelector::~FixedSelector() {

}

StockList FixedSelector::getSelectedStock(Datetime date) {
    return m_stock_list;
}

SelectorPtr HKU_API SE_Fixed() {
    return make_shared<FixedSelector>();
}

SelectorPtr HKU_API SE_Fixed(const StockList& stock_list) {
    SelectorPtr p = make_shared<FixedSelector>();
    p->addStockList(stock_list);
    return p;
}

} /* namespace hku */
