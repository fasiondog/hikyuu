/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-05
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_ITR_H_
#define INDICATOR_IMP_ITR_H_

#include "../Indicator.h"

namespace hku {

/*
 * 真实波动幅度（TR）是以下三个值中的最大值：
 *  1. 当前周期最高价与最低价之差
 *  2. 当前周期最高价与前一周期收盘价之差的绝对值
 *  3. 当前周期最低价与前一周期收盘价之差的绝对值
 */
class ITr : public IndicatorImp {
    INDICATOR_IMP(ITr)
    INDICATOR_NEED_CONTEXT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ITr();
    explicit ITr(const KData&);
    virtual ~ITr();
};

} /* namespace hku */

#endif /* INDICATOR_IMP_ITR_H_ */
