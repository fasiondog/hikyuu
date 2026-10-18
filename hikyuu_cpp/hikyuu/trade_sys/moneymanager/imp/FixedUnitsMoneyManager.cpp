/*
 * FixedUnitsMoneyManager.cpp
 *
 *  Created on: 2016-5-3
 *      Author: Administrator
 */

#include "FixedUnitsMoneyManager.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedUnitsMoneyManager)
#endif

namespace hku {

FixedUnitsMoneyManager::FixedUnitsMoneyManager() : MoneyManagerBase("MM_FixedUnits") {
    setParam<int>("n", 33);
}

FixedUnitsMoneyManager::~FixedUnitsMoneyManager() {}

void FixedUnitsMoneyManager::_checkParam(const string& name) const {
    if ("n" == name) {
        int n = getParam<int>("n");
        HKU_ASSERT(n > 0);
    }
}

double FixedUnitsMoneyManager::_getBuyNumber(const Datetime& datetime, const Stock& stock,
                                             price_t price, price_t risk, SystemPart from) {
    int n = getParam<int>("n");
    // Read the weight-updated cash through the standard read API of the trade manager; the query
    // itself must not mutate the account
    price_t cash = m_tm->cash(datetime, m_query.kType());
    price_t fixed_risk = (cash > m_tm->initCash()) ? cash / n : m_tm->initCash() / n;

    return fixed_risk / risk;
}

MoneyManagerPtr HKU_API MM_FixedUnits(int n) {
    MoneyManagerPtr p = make_shared<FixedUnitsMoneyManager>();
    p->setParam<int>("n", n);
    return p;
}

} /* namespace hku */
