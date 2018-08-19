/*
 * Ma.h
 *
 *  Created on: 2013-2-10
 *      Author: fasiondog
 */

#ifndef SMA_H_
#define SMA_H_

#include "../Indicator.h"

namespace hku {

class Sma: public IndicatorImp {
    INDICATOR_IMP(Sma)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    Sma();
    virtual ~Sma();
};

} /* namespace hku */
#endif /* SMA_H_ */
