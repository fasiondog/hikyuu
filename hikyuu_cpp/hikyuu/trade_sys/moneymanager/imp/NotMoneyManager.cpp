/*
 * NotMoneyManager.cpp
 *
 *  Created on: 2017-5-22
 *      Author: Administrator
 */

#include "NotMoneyManager.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::NotMoneyManager)
#endif

namespace hku {

NotMoneyManager::NotMoneyManager() : MoneyManagerBase("MM_Nothing") {
    // Do not buy any more when there is already a position
    setParam<bool>("if_have_a_position_will_not_buy", false);
    setParam<bool>("if_have_short_will_not_sell_short", false);
}

NotMoneyManager::~NotMoneyManager() {}

double NotMoneyManager::_getBuyNumber(const Datetime& datetime, const Stock& stock, price_t price,
                                      price_t risk, SystemPart from) {
    if (getParam<bool>("if_have_a_position_will_not_buy") &&
        m_tm->getHoldNumber(datetime, stock) > 0.) {
        return 0.0;
    }
    // The cash of the query datetime instead of the latest one, so a historical query does not
    // see the cash of the future trades
    return m_tm->cash(datetime, m_query.kType()) / (price * stock.unit());
}

double NotMoneyManager::_getSellShortNumber(const Datetime& datetime, const Stock& stock,
                                            price_t price, price_t risk, SystemPart from) {
    if (getParam<bool>("if_have_short_will_not_sell_short") && m_tm->haveShort(stock)) {
        return 0.0;
    }
    // Base the size on the net assets: the borrowed proceeds in cash must not inflate the
    // short size
    FundsRecord funds = m_tm->getFunds(datetime, m_query.kType());
    return funds.net_assets() / (price * stock.unit());
}

MoneyManagerPtr HKU_API MM_Nothing() {
    return make_shared<NotMoneyManager>();
}

} /* namespace hku */
