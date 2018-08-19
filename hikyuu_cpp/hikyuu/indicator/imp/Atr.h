/*
 * Atr.h
 *
 *  Created on: 2016年5月4日
 *      Author: Administrator
 */

#ifndef INDICATOR_IMP_ATR_H_
#define INDICATOR_IMP_ATR_H_

#include "../Indicator.h"

namespace hku {

class Atr: public IndicatorImp {
    INDICATOR_IMP(Atr)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    Atr();
    virtual ~Atr();
};

} /* namespace hku */

#endif /* INDICATOR_IMP_ATR_H_ */
