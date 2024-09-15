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
    SELECTOR_IMP(OptimalSelector)
    SELECTOR_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    OptimalSelector();
    virtual ~OptimalSelector() = default;

    virtual void _checkParam(const string& name) const override;
    virtual void calculate(const SystemList& pf_realSysList, const KQuery& query) override;

    virtual void _reset() override;

    const vector<std::pair<Datetime, Datetime>>& getRunRanges() const {
        return m_run_ranges;
    }

private:
    unordered_map<Datetime, SYSPtr> m_sys_dict;
    vector<std::pair<Datetime, Datetime>> m_run_ranges;
};

}  // namespace hku