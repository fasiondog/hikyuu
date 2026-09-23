/*
 * AF_crt.cpp
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  The AF factory implementation: assemble the concrete implementations of AllocateFundsBase (the three parts L1/L2/L3).
 *  See docs/design/pf_af_compat/design.md §5
 */

#include "AF_EqualWeight.h"
#include "AF_FixedWeight.h"
#include "AF_FixedWeightList.h"
#include "AF_FixedAmount.h"
#include "AF_MultiFactor.h"

#include "../imp/EqualWeightAllocateFunds.h"
#include "../imp/FixedWeightAllocateFunds.h"
#include "../imp/FixedWeightListAllocateFunds.h"
#include "../imp/FixedAmountAllocateFunds.h"
#include "../imp/MultiFactorAllocateFunds.h"

namespace hku {

AFPtr HKU_API AF_EqualWeight() {
    // The equal weight = the L1 default 1/N (equivalent to the normalized equal-proportion semantics of master EqualWeightAllocateFunds)
    return std::make_shared<EqualWeightAllocateFunds>();
}

AFPtr HKU_API AF_FixedWeight(double weight) {
    auto p = std::make_shared<FixedWeightAllocateFunds>();
    p->setParam<double>("weight", weight);
    return p;
}

AFPtr HKU_API AF_FixedWeightList(const vector<double>& weights) {
    HKU_ERROR_IF_RETURN(weights.empty(), AF_EqualWeight(), "Input weights is empty!");
    auto p = std::make_shared<FixedWeightListAllocateFunds>();
    p->setParam<PriceList>("weights", PriceList(weights.begin(), weights.end()));
    return p;
}

AFPtr HKU_API AF_FixedAmount(double amount) {
    auto p = std::make_shared<FixedAmountAllocateFunds>();
    p->setParam<double>("fixed-amount", amount);
    return p;
}

AFPtr HKU_API AF_MultiFactor() {
    return std::make_shared<MultiFactorAllocateFunds>();
}

}  // namespace hku
