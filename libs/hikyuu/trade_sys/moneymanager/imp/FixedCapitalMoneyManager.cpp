/*
 * FixedCapitalMoneyManager.cpp
 *
 *  Created on: 2016年5月3日
 *      Author: Administrator
 */

#include "FixedCapitalMoneyManager.h"

namespace hku {

FixedCapitalMoneyManager::FixedCapitalMoneyManager()
: MoneyManagerBase("MM_FixedCapital") {
    setParam<double>("capital", 10000.00);
}

FixedCapitalMoneyManager::~FixedCapitalMoneyManager() {

}

size_t FixedCapitalMoneyManager
::_getBuyNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk, SystemPart from) {

    double capital = getParam<double>("capital");
    return capital > 0.0 ? size_t(m_tm->currentCash() / capital) : 0;
}

MoneyManagerPtr HKU_API MM_FixedCapital(double capital) {
    FixedCapitalMoneyManager *p = new FixedCapitalMoneyManager();
    p->setParam<double>("capital", capital);
    return MoneyManagerPtr(p);
}

} /* namespace hku */
