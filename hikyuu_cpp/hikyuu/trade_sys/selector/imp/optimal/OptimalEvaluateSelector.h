/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-22
 *      Author: fasiondog
 */

#pragma once

#include "OptimalSelectorBase.h"

namespace hku {

class OptimalEvaluateSelector : public OptimalSelectorBase {
    OPTIMAL_SELECTOR_IMP(OptimalEvaluateSelector)
    OPTIMAL_SELECTOR_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    OptimalEvaluateSelector();
    OptimalEvaluateSelector(std::function<double(const SystemPtr&, const Datetime&)>&& evaluate);

    virtual ~OptimalEvaluateSelector();

private:
    std::function<double(const SystemPtr&, const Datetime&)> m_evaluate;
};

}  // namespace hku
