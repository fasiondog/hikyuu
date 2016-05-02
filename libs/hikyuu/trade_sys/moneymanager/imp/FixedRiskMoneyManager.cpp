/*
 * FixedRiskMM.cpp
 *
 *  Created on: 2016年5月1日
 *      Author: Administrator
 */

#include <hikyuu/trade_sys/moneymanager/imp/FixedRiskMoneyManager.h>

namespace hku {

FixedRiskMoneyManager::FixedRiskMoneyManager()
: MoneyManagerBase("MM_FixedRisk") {
    setParam<double>("risk", 1000.00);
}

FixedRiskMoneyManager::~FixedRiskMoneyManager() {

}

size_t FixedRiskMoneyManager
::_getBuyNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk) {
    if (risk <= 0.0)
        return 0;

    return getParam<double>("risk") / risk;
}


MoneyManagerPtr HKU_API MM_FixedRisk(double risk) {
    FixedRiskMoneyManager *p = new FixedRiskMoneyManager();
    p->setParam<double>("risk", risk);
    return MoneyManagerPtr(p);
}


} /* namespace hku */
