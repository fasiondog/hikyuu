/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-13
 *      Author: fasiondog
 */

#pragma once

#include "../MultiFactorBase.h"

namespace hku {

class ICMultiFactor : public MultiFactorBase {
    MULTIFACTOR_IMP(ICMultiFactor)
    MULTIFACTOR_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ICMultiFactor();
    ICMultiFactor(const IndicatorList& inds, const StockList& stks, const KQuery& query,
                  const Stock& ref_stk, int ic_n, int ic_rolling_n);
    virtual ~ICMultiFactor() = default;

    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku