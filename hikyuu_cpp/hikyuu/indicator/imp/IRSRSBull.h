/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-07-12
 *  Author: fasiondog
 */

#pragma once

#include "../IndicatorImp.h"

namespace hku {

class IRSRSBull : public IndicatorImp {
    INDICATOR_IMP(IRSRSBull)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IRSRSBull();
    virtual ~IRSRSBull() override;
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku