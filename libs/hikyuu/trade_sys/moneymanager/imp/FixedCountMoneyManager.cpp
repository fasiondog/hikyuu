/*
 * FixedCountMoneyManager.cpp
 *
 *  Created on: 2013-4-19
 *      Author: fasiondog
 */

#include "FixedCountMoneyManager.h"

namespace hku {

FixedCountMoneyManager::FixedCountMoneyManager(int n)
: MoneyManagerBase("FixedCount") {
    if (n < 0) {
        HKU_ERROR("Invalid param! (n >= 1) " << m_params
                << " [FixedCountMoneyManager::FixedCountMoneyManager]");
        addParam<int>("n", 0);
        return;
    }

    addParam<int>("n", n);
}

FixedCountMoneyManager::~FixedCountMoneyManager() {

}

size_t FixedCountMoneyManager
::getBuyNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk) {
    size_t n = getParam<int>("n");
    if (m_tm) {
        price_t cash = 0.0;//m_tm->cash();
        //boost::tie(cash, boost::tuples::ignore, boost::tuples::ignore,
        //        boost::tuples::ignore) = m_tm->getFunds(datetime);
        FundsRecord funds = m_tm->getFunds(datetime);
        cash = funds.cash;
        CostRecord cost = m_tm->getBuyCost(datetime, stock, price, n);
        price_t money = roundUp(price * n + cost.total, m_tm->precision());
        if (money > cash) {
            m_tm->checkin(datetime, roundEx(money - cash, m_tm->precision()));
        }
    }

    return n;
}

size_t FixedCountMoneyManager::getSellNumber(const Datetime& datetime, const Stock& stock,
        price_t price, price_t risk) {
    return getParam<int>("n");
}

void FixedCountMoneyManager::_reset() {

}

MoneyManagerPtr FixedCountMoneyManager::_clone() {
    int n = getParam<int>("n");
    return MoneyManagerPtr(new FixedCountMoneyManager(n));
}

MoneyManagerPtr HKU_API FixedCount_MM(int n) {
    return MoneyManagerPtr(new FixedCountMoneyManager(n));
}

} /* namespace hku */
