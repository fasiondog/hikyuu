/*
 * NotMoneyManager.cpp
 *
 *  Created on: 2017年5月22日
 *      Author: Administrator
 */

#include "NotMoneyManager.h"

namespace hku {

NotMoneyManager::NotMoneyManager()
: MoneyManagerBase("MM_Nothing") {

}

NotMoneyManager::~NotMoneyManager() {

}


size_t NotMoneyManager
::_getBuyNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk, SystemPart from) {
    return size_t(m_tm->currentCash() / price);
}

MoneyManagerPtr HKU_API MM_Nothing() {
    NotMoneyManager *p = new NotMoneyManager();
    return MoneyManagerPtr(p);
}

} /* namespace hku */
