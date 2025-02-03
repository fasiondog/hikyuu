/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator2InImp.h"

namespace hku {

class ISpearman : public Indicator2InImp {
    INDICATOR2IN_IMP(ISpearman)
    INDICATOR2IN_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ISpearman();
    ISpearman(const Indicator& ref_ind, int n, bool fill_null);
    virtual ~ISpearman();

    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku