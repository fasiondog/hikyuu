/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator2InImp.h"

namespace hku {

class ICov : public Indicator2InImp {
    INDICATOR2IN_IMP(ICov)
    INDICATOR2IN_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ICov();
    ICov(const Indicator& ref_ind, int n, bool fill_null);
    virtual ~ICov() override;

    virtual void _checkParam(const string& name) const override;
    virtual bool supportIncrementCalculate() const override;
    virtual size_t min_increment_start() const override;
    virtual void _increment_calculate(const Indicator& ind, size_t start_pos) override;
};

}  // namespace hku