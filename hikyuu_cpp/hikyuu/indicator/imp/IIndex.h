/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-09
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

class IIndex : public IndicatorImp {
    INDICATOR_IMP(IIndex)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IIndex();
    IIndex(const string& kpart, bool fill_null);
    virtual ~IIndex();

    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku