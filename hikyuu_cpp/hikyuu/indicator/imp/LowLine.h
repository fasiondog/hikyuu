/*
 * LowLine.h
 *
 *  Created on: 2016年4月2日
 *      Author: fasiondog
 */

#ifndef INDICATOR_IMP_LOWLINE_H_
#define INDICATOR_IMP_LOWLINE_H_

#include "../Indicator.h"

namespace hku {

class LowLine: public IndicatorImp {
    INDICATOR_IMP(LowLine)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    LowLine();
    virtual ~LowLine();
};

} /* namespace hku */

#endif /* INDICATOR_IMP_LOWLINE_H_ */
