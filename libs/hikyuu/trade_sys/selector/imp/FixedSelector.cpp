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

} /* namespace hku */
