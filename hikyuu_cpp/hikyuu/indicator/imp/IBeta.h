/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-XX-XX
 *  Author: fasiondog
 */

#pragma once

#include "../Indicator2InImp.h"

namespace hku {

class IBeta : public Indicator2InImp {
    INDICATOR2IN_IMP(IBeta)
    INDICATOR2IN_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IBeta();
    IBeta(const Indicator& ref_ind, int n, bool fill_null);
    virtual ~IBeta() override;

    virtual void _checkParam(const string& name) const override;
    virtual bool supportIncrementCalculate() const override;
    virtual size_t min_increment_start() const override;
    virtual void _increment_calculate(const Indicator& ind, size_t start_pos) override;
};

}  // namespace hku