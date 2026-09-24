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
 * Count the total number; count the number of the periods satisfying the condition.
 * COUNT(X,N) counts the number of the periods satisfying the X condition within N periods; if N=0
 * it starts from the first valid value.
 * COUNT(CLOSE>OPEN,20) counts the number of the periods closing up within 20 periods
 */
class ICount : public IndicatorImp {
    INDICATOR_IMP(ICount)
    INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ICount();
    virtual ~ICount() override;
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_ICOUNT_H_ */
