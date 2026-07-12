/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-07-12
 *  Author: fasiondog
 */

#pragma once

#include "../IndicatorImp.h"

namespace hku {

class IRSRSBeta : public IndicatorImp {
    INDICATOR_IMP(IRSRSBeta)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IRSRSBeta();
    virtual ~IRSRSBeta() override;
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku