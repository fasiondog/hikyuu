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

double MaxFundsOptimalSelector::evaluate(const SYSPtr& sys, const Datetime& endDate) noexcept {
    double ret = Null<double>();
    try {
        auto funds = sys->getTM()->getFunds(endDate);
        ret = funds.total_assets();
    } catch (const std::exception& e) {
        HKU_ERROR("Get funds failed! {}! {}", e.what(), name());
    } catch (...) {
        HKU_ERROR("Get funds failed! Unknown exception! {}", name());
    }
    return ret;
}

SEPtr HKU_API SE_MaxFundsOptimal() {
    return make_shared<MaxFundsOptimalSelector>();
}

}  // namespace hku