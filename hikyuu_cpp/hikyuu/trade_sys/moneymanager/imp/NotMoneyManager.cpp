/*
 * NotMoneyManager.cpp
 *
 *  Created on: 2017年5月22日
 *      Author: Administrator
 */

#include "NotMoneyManager.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::NotMoneyManager)
#endif

namespace hku {

NotMoneyManager::NotMoneyManager() : MoneyManagerBase("MM_Nothing") {
    // 如果已有持仓，则不再买入
    setParam<bool>("if_have_a_position_will_not_buy", false);
}

NotMoneyManager::~NotMoneyManager() {}

double NotMoneyManager ::_getBuyNumber(const Datetime& datetime, const Stock& stock, price_t price,
                                       price_t risk, SystemPart from) {
    if (getParam<bool>("if_have_a_position_will_not_buy") &&
        m_tm->getHoldNumber(datetime, stock) > 0.) {
        return 0.0;
    }
    return m_tm->cash(datetime, m_query.kType()) / price;
}

MoneyManagerPtr HKU_API MM_Nothing() {
    NotMoneyManager* p = new NotMoneyManager();
    return MoneyManagerPtr(p);
}

} /* namespace hku */
