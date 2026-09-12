/*
 * AF_crt.cpp
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  AF 工厂实现：组装 AllocateFundsBase 的具体实现（L1/L2/L3 三个部件）。
 *  见 docs/design/pf_af_compat/design.md §5
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
    // 等权 = L1 默认 1/N（等价 master EqualWeightAllocateFunds 归一化后的等比例语义）
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
