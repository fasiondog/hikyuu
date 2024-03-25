/*
 * FixedCountMoneyManager.cpp
 *
 *  Created on: 2013-4-19
 *      Author: fasiondog
 */

#include "FixedCountMoneyManager.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedCountMoneyManager)
#endif

namespace hku {

FixedCountMoneyManager::FixedCountMoneyManager() : MoneyManagerBase("MM_FixedCount") {
    setParam<double>("n", 100);
}

FixedCountMoneyManager::~FixedCountMoneyManager() {}

void FixedCountMoneyManager::_checkParam(const string& name) const {
    if ("n" == name) {
        double n = getParam<double>("n");
        HKU_ASSERT(n > 0.0);
    }
}

double FixedCountMoneyManager ::_getBuyNumber(const Datetime& datetime, const Stock& stock,
                                              price_t price, price_t risk, SystemPart from) {
    return getParam<double>("n");
}

double FixedCountMoneyManager::_getSellShortNumber(const Datetime& datetime, const Stock& stock,
                                                   price_t price, price_t risk, SystemPart from) {
    return getParam<double>("n");
}

MoneyManagerPtr HKU_API MM_FixedCount(double n) {
    FixedCountMoneyManager* p = new FixedCountMoneyManager();
    p->setParam<double>("n", n);
    return MoneyManagerPtr(p);
}

} /* namespace hku */
