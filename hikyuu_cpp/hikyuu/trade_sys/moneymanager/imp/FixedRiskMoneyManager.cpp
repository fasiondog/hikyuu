/*
 * FixedRiskMM.cpp
 *
 *  Created on: 2016年5月1日
 *      Author: Administrator
 */

#include "FixedRiskMoneyManager.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedRiskMoneyManager)
#endif

namespace hku {

FixedRiskMoneyManager::FixedRiskMoneyManager() : MoneyManagerBase("MM_FixedRisk") {
    setParam<double>("risk", 1000.00);
}

FixedRiskMoneyManager::~FixedRiskMoneyManager() {}

double FixedRiskMoneyManager ::_getBuyNumber(const Datetime& datetime, const Stock& stock,
                                             price_t price, price_t risk, SystemPart from) {
    return getParam<double>("risk") / risk;
}

MoneyManagerPtr HKU_API MM_FixedRisk(double risk) {
    FixedRiskMoneyManager* p = new FixedRiskMoneyManager();
    p->setParam<double>("risk", risk);
    return MoneyManagerPtr(p);
}

} /* namespace hku */
