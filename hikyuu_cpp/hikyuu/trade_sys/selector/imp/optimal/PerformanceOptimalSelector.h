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
    virtual void calculate(const SystemList& pf_realSysList, const KQuery& query) override;

    virtual SystemWeightList getSelected(Datetime date);
    virtual SelectorPtr _clone() override;
    virtual void _reset() override;

private:
    void _calculate_single(const vector<std::pair<size_t, size_t>>& train_ranges,
                           const DatetimeList& dates, const string& key, int mode, size_t test_len,
                           bool trace);

    void _calculate_parallel(const vector<std::pair<size_t, size_t>>& train_ranges,
                             const DatetimeList& dates, const string& key, int mode,
                             size_t test_len, bool trace);

private:
    unordered_map<Datetime, SYSPtr> m_sys_dict;
};

}  // namespace hku