/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-18
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

class IZongGuBen : public IndicatorImp {
    INDICATOR_IMP(IZongGuBen)
    INDICATOR_NEED_CONTEXT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IZongGuBen();
    explicit IZongGuBen(const KData&);
    virtual ~IZongGuBen();
};

} /* namespace hku */
