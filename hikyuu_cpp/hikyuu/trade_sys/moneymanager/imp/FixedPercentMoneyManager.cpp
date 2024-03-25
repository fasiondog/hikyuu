/*
 * PercentRiskMoneyManager.cpp
 *
 *  Created on: 2015年4月4日
 *      Author: fasiondog
 */

#include "FixedPercentMoneyManager.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedPercentMoneyManager)
#endif

namespace hku {

FixedPercentMoneyManager::FixedPercentMoneyManager() : MoneyManagerBase("MM_FixedPercent") {
    setParam<double>("p", 0.02);  // 总资产百分比
}

FixedPercentMoneyManager::~FixedPercentMoneyManager() {}

void FixedPercentMoneyManager::_checkParam(const string& name) const {
    if ("p" == name) {
        double p = getParam<double>("p");
        HKU_ASSERT(p > 0 && p <= 1.0);
    }
}

double FixedPercentMoneyManager ::_getBuyNumber(const Datetime& datetime, const Stock& stock,
                                                price_t price, price_t risk, SystemPart from) {
    double p = getParam<double>("p");
    return m_tm->cash(datetime, m_query.kType()) * p / risk;
}

MoneyManagerPtr HKU_API MM_FixedPercent(double p) {
    FixedPercentMoneyManager* ptr = new FixedPercentMoneyManager();
    ptr->setParam<double>("p", p);
    return MoneyManagerPtr(ptr);
}

} /* namespace hku */
