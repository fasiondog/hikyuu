/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-22
 *      Author: fasiondog
 */

#include "OptimalEvaluateSelector.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::OptimalEvaluateSelector)
#endif

namespace hku {

OptimalEvaluateSelector::OptimalEvaluateSelector() : OptimalSelectorBase("SE_EvaluateOptimal") {}

OptimalEvaluateSelector::OptimalEvaluateSelector(
  std::function<double(const SystemPtr&, const Datetime&)>&& evaluate)
: OptimalSelectorBase("SE_EvaluateOptimal"), m_evaluate(std::move(evaluate)) {}

OptimalEvaluateSelector::~OptimalEvaluateSelector() {}

double OptimalEvaluateSelector::evaluate(const SYSPtr& sys, const Datetime& endDate) noexcept {
    double ret = Null<double>();
    try {
        ret = m_evaluate(sys, endDate);
    } catch (const std::exception& e) {
        HKU_ERROR("Failed evaluate! {}! {}", e.what(), name());
    } catch (...) {
        HKU_ERROR("Failed evaluate! Unknown exception! {}", name());
    }
    return ret;
}

SEPtr HKU_API
SE_EvaluateOptimal(std::function<double(const SystemPtr&, const Datetime&)>&& evaluate) {
    return make_shared<OptimalEvaluateSelector>(std::move(evaluate));
}

}  // namespace hku