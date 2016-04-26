/*
 * FixedCountMoneyManager.cpp
 *
 *  Created on: 2013-4-19
 *      Author: fasiondog
 */

#include "FixedCountMoneyManager.h"

namespace hku {

FixedCountMoneyManager::FixedCountMoneyManager()
: MoneyManagerBase("FixedCount") {
    setParam<int>("n", 100);
}

FixedCountMoneyManager::~FixedCountMoneyManager() {

}

size_t FixedCountMoneyManager
::_getBuyNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk) {
    size_t n = getParam<int>("n");

    price_t cash = m_tm->currentCash();
    CostRecord cost = m_tm->getBuyCost(datetime, stock, price, n);
    price_t money = roundUp(price * n + cost.total, m_tm->precision());
    if (money > cash) {
        m_tm->checkin(datetime, roundUp(money - cash, m_tm->precision()));
    }

    return n;
}

size_t FixedCountMoneyManager::_getSellNumber(const Datetime& datetime, const Stock& stock,
        price_t price, price_t risk) {
    return getParam<int>("n");
}

MoneyManagerPtr HKU_API FixedCount_MM(int n) {
    FixedCountMoneyManager *p = new FixedCountMoneyManager();
    p->setParam<int>("n", n);
    return MoneyManagerPtr(p);
}

} /* namespace hku */
