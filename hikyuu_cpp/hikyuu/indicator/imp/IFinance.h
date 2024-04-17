/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-13
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

class IFinance : public IndicatorImp {
    INDICATOR_IMP(IFinance)
    INDICATOR_NEED_CONTEXT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IFinance();
    explicit IFinance(const KData&);
    virtual ~IFinance() = default;
};

}  // namespace hku