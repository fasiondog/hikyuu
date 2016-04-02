/*
 * PercentRiskMoneyManager.cpp
 *
 *  Created on: 2015年4月4日
 *      Author: Administrator
 */

#include <hikyuu/trade_sys/moneymanager/imp/PercentRiskMoneyManager.h>

namespace hku {

PercentRiskMoneyManager::PercentRiskMoneyManager()
: MoneyManagerBase("PercentRisk"){
    setParam<double>("p", 0.02); //总资产百分比
}

PercentRiskMoneyManager::~PercentRiskMoneyManager() {

}

size_t PercentRiskMoneyManager
::_getBuyNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk) {
    double p = getParam<double>("p");
    if (p <= 0.0 || p > 1.0) {
        HKU_ERROR("Error param (p = " << p
                << ") [PercentRiskMoneyManager::getBuyNumber]");
        return 0;
    }

    return int(m_tm->currentCash() * p / risk);
}

MoneyManagerPtr HKU_API PercentRisk_MM(double p) {
    PercentRiskMoneyManager *ptr = new PercentRiskMoneyManager();
    ptr->setParam<double>("p", p);
    return MoneyManagerPtr(ptr);
}

} /* namespace hku */
