/*
 * PercentRiskMoneyManager.cpp
 *
 *  Created on: 2015年4月4日
 *      Author: fasiondog
 */

#include "FixedPercentMoneyManager.h"

namespace hku {

FixedPercentMoneyManager::FixedPercentMoneyManager()
: MoneyManagerBase("MM_FixedPercent"){
    setParam<double>("p", 0.02); //总资产百分比
}

FixedPercentMoneyManager::~FixedPercentMoneyManager() {

}

size_t FixedPercentMoneyManager
::_getBuyNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk, SystemPart from) {
    double p = getParam<double>("p");
    if (p <= 0.0 || p > 1.0) {
        HKU_ERROR("Error param (p = " << p
                << ") [PercentRiskMoneyManager::_getBuyNumber]");
        return 0;
    }

    if (risk == 0.0) {
        HKU_ERROR("risk is zero! [PercentRiskMoneyManager::_getBuyNumber");
        return 0;
    }

    return int(m_tm->currentCash() * p / risk);
}

MoneyManagerPtr HKU_API MM_FixedPercent(double p) {
    FixedPercentMoneyManager *ptr = new FixedPercentMoneyManager();
    ptr->setParam<double>("p", p);
    return MoneyManagerPtr(ptr);
}

} /* namespace hku */
