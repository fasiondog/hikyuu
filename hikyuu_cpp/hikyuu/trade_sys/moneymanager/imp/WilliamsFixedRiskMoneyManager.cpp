/*
 * WilliamsFixedRiskMoneyManager.cpp
 *
 *  Created on: 2016年5月3日
 *      Author: Administrator
 */

#include "WilliamsFixedRiskMoneyManager.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::WilliamsFixedRiskMoneyManager)
#endif

namespace hku {

WilliamsFixedRiskMoneyManager::WilliamsFixedRiskMoneyManager()
: MoneyManagerBase("MM_WilliamsFixedRisk") {
    setParam<double>("p", 0.1);
    setParam<price_t>("max_loss", 1000.00);
}

WilliamsFixedRiskMoneyManager::~WilliamsFixedRiskMoneyManager() {}

void WilliamsFixedRiskMoneyManager::_checkParam(const string& name) const {
    if ("p" == name) {
        double p = getParam<double>("p");
        HKU_ASSERT(p > 0.0);
    } else if ("max_loss" == name) {
        price_t max_loss = getParam<price_t>("max_loss");
        HKU_ASSERT(max_loss > 0.0);
    }
}

double WilliamsFixedRiskMoneyManager::_getBuyNumber(const Datetime& datetime, const Stock& stock,
                                                    price_t price, price_t risk, SystemPart from) {
    price_t max_loss = getParam<price_t>("max_loss");
    HKU_WARN_IF_RETURN(max_loss <= 0.0, 0.0, "max_loss is zero!");
    return m_tm->cash(datetime, m_query.kType()) * getParam<double>("p") / max_loss;
}

MoneyManagerPtr HKU_API MM_WilliamsFixedRisk(double p, price_t max_loss) {
    WilliamsFixedRiskMoneyManager* ptr = new WilliamsFixedRiskMoneyManager();
    ptr->setParam<double>("p", p);
    ptr->setParam<price_t>("max_loss", max_loss);
    return MoneyManagerPtr(ptr);
}

} /* namespace hku */
