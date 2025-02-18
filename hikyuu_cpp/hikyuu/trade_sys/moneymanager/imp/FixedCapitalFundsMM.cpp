/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-19
 *      Author: fasiondog
 */

#include "FixedCapitalFundsMM.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedCapitalFundsMM)
#endif

namespace hku {

FixedCapitalFundsMM::FixedCapitalFundsMM() : MoneyManagerBase("MM_FixedCapitalFunds") {
    setParam<double>("capital", 10000.00);
}

FixedCapitalFundsMM::~FixedCapitalFundsMM() {}

void FixedCapitalFundsMM::_checkParam(const string& name) const {
    if ("capital" == name) {
        double capital = getParam<double>("capital");
        HKU_ASSERT(capital > 0.0);
    }
}

double FixedCapitalFundsMM ::_getBuyNumber(const Datetime& datetime, const Stock& stock,
                                           price_t price, price_t risk, SystemPart from) {
    double capital = getParam<double>("capital");
    FundsRecord funds = m_tm->getFunds(datetime, m_query.kType());
    return funds.total_assets() / capital;
}

MoneyManagerPtr HKU_API MM_FixedCapitalFunds(double capital) {
    MoneyManagerPtr p = make_shared<FixedCapitalFundsMM>();
    p->setParam<double>("capital", capital);
    return p;
}

} /* namespace hku */
