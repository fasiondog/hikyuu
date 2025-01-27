/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-22
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IHSL_H_
#define INDICATOR_IMP_IHSL_H_

#include "../Indicator.h"

namespace hku {

/* 获取换手率，等于 VOL(k) / CAPITAL(k) */
class IHsl : public IndicatorImp {
    INDICATOR_IMP(IHsl)
    INDICATOR_NEED_CONTEXT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IHsl();
    explicit IHsl(const KData&);
    virtual ~IHsl();
};

} /* namespace hku */

#endif /* INDICATOR_IMP_IHSL_H_ */
