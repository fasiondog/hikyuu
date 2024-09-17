/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-13
 *      Author: fasiondog
 */

#pragma once
#include "../SelectorBase.h"

namespace hku {

class OptimalSelector : public SelectorBase {
    CLASS_LOGGER_IMP(SE_Optimal)
    SELECTOR_IMP(OptimalSelector)
    SELECTOR_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    OptimalSelector();
    virtual ~OptimalSelector() = default;

    virtual void _checkParam(const string& name) const override;
    virtual void calculate(const SystemList& pf_realSysList, const KQuery& query) override;

    virtual void _reset() override;

    virtual string str() const override;

    const vector<std::pair<Datetime, Datetime>>& getRunRanges() const {
        return m_run_ranges;
    }

private:
    void _calculate_single(const vector<std::pair<size_t, size_t>>& train_ranges,
                           const DatetimeList& dates, const string& key, int mode, size_t test_len,
                           bool trace);

    void _calculate_parallel(const vector<std::pair<size_t, size_t>>& train_ranges,
                             const DatetimeList& dates, const string& key, int mode,
                             size_t test_len, bool trace);

private:
    unordered_map<Datetime, SYSPtr> m_sys_dict;
    vector<std::pair<Datetime, Datetime>> m_run_ranges;
};

}  // namespace hku