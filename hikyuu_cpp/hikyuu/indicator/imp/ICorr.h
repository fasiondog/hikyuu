/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator2InImp.h"

namespace hku {

class ICorr : public Indicator2InImp {
    INDICATOR2IN_IMP(ICorr)
    INDICATOR2IN_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ICorr();
    ICorr(const Indicator& ref_ind, int n, bool fill_null);
    virtual ~ICorr();

    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku