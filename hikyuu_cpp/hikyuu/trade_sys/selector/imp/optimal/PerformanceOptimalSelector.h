/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-13
 *      Author: fasiondog
 */

#pragma once

#include "OptimalSelectorBase.h"

namespace hku {

class PerformanceOptimalSelector : public OptimalSelectorBase {
    CLASS_LOGGER_IMP(SE_Optimal)
    OPTIMAL_SELECTOR_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    PerformanceOptimalSelector();
    virtual ~PerformanceOptimalSelector() = default;

    virtual void _checkParam(const string& name) const override;

    virtual SelectorPtr _clone() override;

private:
    void _calculate_parallel(const vector<std::pair<size_t, size_t>>& train_ranges,
                             const DatetimeList& dates, size_t test_len, bool trace) override;
};

}  // namespace hku