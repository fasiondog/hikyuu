/*
 * ICount.h
 *
 *  Created on: 2019-3-25
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_ICOUNT_H_
#define INDICATOR_IMP_ICOUNT_H_

#include "../Indicator.h"

namespace hku {

/*
 * 统计总数 统计满足条件的周期数。
 * COUNT(X,N),统计N周期中满足X条件的周期数,若N=0则从第一个有效值开始。
 * COUNT(CLOSE>OPEN,20)表示统计20周期内收阳的周期数
 */
class ICount : public IndicatorImp {
    INDICATOR_IMP_SUPPORT_DYNAMIC_STEP(ICount)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ICount();
    virtual ~ICount();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_ICOUNT_H_ */
