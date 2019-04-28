/*
 * ILast.h
 * 
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-28
 *      Author: fasiondog
 */

#ifndef INDICATOR_IMP_ILAST_H_
#define INDICATOR_IMP_ILAST_H_

#include "../Indicator.h"

namespace hku {

/*
 * 区间存在, LAST (X,M,N) 表示条件X在前M周期到前N周期存在
 */
class ILast: public IndicatorImp {
    INDICATOR_IMP(ILast)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ILast();
    virtual ~ILast();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_ILAST_H_ */
