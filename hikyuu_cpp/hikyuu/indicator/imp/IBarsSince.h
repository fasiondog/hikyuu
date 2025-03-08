/*
 * IBarsSince.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-4
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IBARSSINCE_H_
#define INDICATOR_IMP_IBARSSINCE_H_

#include "../Indicator.h"

namespace hku {

/*
 * N周期内首个条件成立位置, N为0时，为整个序列
 * 用法：BARSSINCEN(X,N):N周期内第一次X不为0到现在的周期数,N为常量BARSSINCEN(X,N):
 * 例如：BARSSINCEN(HIGH>10,10)表示10个周期内股价超过10元时到当前的周期数
 */
class IBarsSince : public IndicatorImp {
    INDICATOR_IMP(IBarsSince)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IBarsSince();
    virtual ~IBarsSince();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IBARSSINCE_H_ */
