/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-11-09
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_ITIME_H_
#define INDICATOR_IMP_ITIME_H_

#include "../Indicator.h"

namespace hku {

/*
 * 通达信时间函数实现
 */
class ITime : public IndicatorImp {
    INDICATOR_IMP(ITime)
    INDICATOR_NEED_CONTEXT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ITime();
    ITime(const KData&, const string& type);
    virtual ~ITime();
    virtual void _checkParam(const string& name) const override;
};

}  // namespace hku

#endif  // INDICATOR_IMP_ITIME_H_