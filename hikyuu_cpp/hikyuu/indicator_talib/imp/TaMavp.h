/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/indicator/Indicator2InImp.h"

namespace hku {

class TaMavp : public Indicator2InImp {
    INDICATOR2IN_IMP(TaMavp)
    INDICATOR2IN_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    TaMavp();
    TaMavp(const Indicator& ref_ind, int min_n, int max_n, int matype, bool fill_null);
    virtual ~TaMavp();

    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku