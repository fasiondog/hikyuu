/*
 * FixedSelector.cpp
 *
 *  Created on: 2018年1月12日
 *      Author: fasiondog
 */

#include "FixedSelector.h"

namespace hku {

FixedSelector::FixedSelector() {
    // TODO Auto-generated constructor stub

}

FixedSelector::~FixedSelector() {
    // TODO Auto-generated destructor stub
}

StockList FixedSelector::getSelectedStock(Datetime date) {
    return m_stock_list;
}

} /* namespace hku */
