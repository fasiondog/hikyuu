/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-21
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"

namespace hku {

class TaApo : public IndicatorImp {
    INDICATOR_IMP(TaApo)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    TaApo();
    virtual ~TaApo() = default;
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku