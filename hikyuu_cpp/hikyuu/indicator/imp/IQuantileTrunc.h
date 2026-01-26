/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-03
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/* 分位数截断处理 */
class IQuantileTrunc : public IndicatorImp {
    INDICATOR_IMP(IQuantileTrunc)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IQuantileTrunc();
    virtual ~IQuantileTrunc();
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku