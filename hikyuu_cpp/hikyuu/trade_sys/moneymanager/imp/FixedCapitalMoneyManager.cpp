/*
 * FixedCapitalMoneyManager.cpp
 *
 *  Created on: 2016年5月3日
 *      Author: Administrator
 */

#include "FixedCapitalMoneyManager.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedCapitalMoneyManager)
#endif

namespace hku {

FixedCapitalMoneyManager::FixedCapitalMoneyManager() : MoneyManagerBase("MM_FixedCapital") {
    setParam<double>("capital", 10000.00);
}

FixedCapitalMoneyManager::~FixedCapitalMoneyManager() {}

void FixedCapitalMoneyManager::_checkParam(const string& name) const {
    if ("capital" == name) {
        double capital = getParam<double>("capital");
        HKU_ASSERT(capital > 0.0);
    }
}

double FixedCapitalMoneyManager ::_getBuyNumber(const Datetime& datetime, const Stock& stock,
                                                price_t price, price_t risk, SystemPart from) {
    double capital = getParam<double>("capital");
    return capital > 0.0 ? size_t(m_tm->cash(datetime, m_query.kType()) / capital) : 0;
}

MoneyManagerPtr HKU_API MM_FixedCapital(double capital) {
    MoneyManagerPtr p = make_shared<FixedCapitalMoneyManager>();
    p->setParam<double>("capital", capital);
    return p;
}

} /* namespace hku */
