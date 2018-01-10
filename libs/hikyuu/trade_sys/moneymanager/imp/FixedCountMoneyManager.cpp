/*
 * FixedCountMoneyManager.cpp
 *
 *  Created on: 2013-4-19
 *      Author: fasiondog
 */

#include "FixedCountMoneyManager.h"

namespace hku {

FixedCountMoneyManager::FixedCountMoneyManager()
: MoneyManagerBase("MM_FixedCount") {
    setParam<int>("n", 100);
}

FixedCountMoneyManager::~FixedCountMoneyManager() {

}

size_t FixedCountMoneyManager
::_getBuyNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk, SystemPart from) {
    return getParam<int>("n");
}


MoneyManagerPtr HKU_API MM_FixedCount(int n) {
    FixedCountMoneyManager *p = new FixedCountMoneyManager();
    p->setParam<int>("n", n);
    return MoneyManagerPtr(p);
}

} /* namespace hku */
