/*
 * WilliamsFixedRiskMoneyManager.cpp
 *
 *  Created on: 2016年5月3日
 *      Author: Administrator
 */

#include "WilliamsFixedRiskMoneyManager.h"

namespace hku {

WilliamsFixedRiskMoneyManager::WilliamsFixedRiskMoneyManager()
: MoneyManagerBase("MM_WilliamsFixedRisk") {
    setParam<double>("p", 0.1);
    setParam<price_t>("max_loss", 1000.00);
}

WilliamsFixedRiskMoneyManager::~WilliamsFixedRiskMoneyManager() {

}

size_t WilliamsFixedRiskMoneyManager
::_getBuyNumber(const Datetime& datetime, const Stock& stock,
            price_t price, price_t risk, SystemPart from) {
    price_t max_loss = getParam<price_t>("max_loss");
    if (max_loss <= 0.0) {
        HKU_WARN("max_loss is zero! "
                "[WilliamsFixedRiskMoneyManager::_getBuyNumber]");
        return 0;
    }

    return m_tm->currentCash() * getParam<double>("p") / max_loss;
}

MoneyManagerPtr HKU_API MM_WilliamsFixedRisk(double p, price_t max_loss) {
    WilliamsFixedRiskMoneyManager *ptr = new WilliamsFixedRiskMoneyManager();
    ptr->setParam<double>("p", p);
    ptr->setParam<price_t>("max_loss", max_loss);
    return MoneyManagerPtr(ptr);
}


} /* namespace hku */
