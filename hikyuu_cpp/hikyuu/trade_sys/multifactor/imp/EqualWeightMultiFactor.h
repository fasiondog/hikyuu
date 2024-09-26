/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-13
 *      Author: fasiondog
 */

#pragma once

#include "../MultiFactorBase.h"

namespace hku {

class EqualWeightMultiFactor : public MultiFactorBase {
    MULTIFACTOR_IMP(EqualWeightMultiFactor)
    MULTIFACTOR_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    EqualWeightMultiFactor();
    EqualWeightMultiFactor(const vector<Indicator>& inds, const StockList& stks,
                           const KQuery& query, const Stock& ref_stk, int ic_n, bool spearman);
    virtual ~EqualWeightMultiFactor() = default;
};

}  // namespace hku