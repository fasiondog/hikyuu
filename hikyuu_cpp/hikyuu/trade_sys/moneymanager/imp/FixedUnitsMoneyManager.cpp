/*
 * FixedUnitsMoneyManager.cpp
 *
 *  Created on: 2016年5月3日
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

double FixedUnitsMoneyManager ::_getBuyNumber(const Datetime& datetime, const Stock& stock,
                                              price_t price, price_t risk, SystemPart from) {
    int n = getParam<int>("n");
    m_tm->updateWithWeight(datetime);
    price_t fixed_risk =
      (m_tm->currentCash() > m_tm->initCash()) ? m_tm->currentCash() / n : m_tm->initCash() / n;

    return fixed_risk / risk;
}

MoneyManagerPtr HKU_API MM_FixedUnits(int n) {
    FixedUnitsMoneyManager* p = new FixedUnitsMoneyManager();
    p->setParam<int>("n", n);
    return MoneyManagerPtr(p);
}

} /* namespace hku */
