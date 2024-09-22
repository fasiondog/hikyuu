/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-22
 *      Author: fasiondog
 */

#include "MaxFundsOptimalSelector.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::MaxFundsOptimalSelector)
#endif

namespace hku {

MaxFundsOptimalSelector::MaxFundsOptimalSelector() : OptimalSelectorBase("SE_MaxFundsOptimal") {}

MaxFundsOptimalSelector::~MaxFundsOptimalSelector() {}

double MaxFundsOptimalSelector::evaluate(const SYSPtr& sys, const Datetime& endDate) {
    auto funds = sys->getTM()->getFunds(endDate);
    return funds.total_assets();
}

SEPtr HKU_API SE_MaxFundsOptimal() {
    return make_shared<MaxFundsOptimalSelector>();
}

}  // namespace hku