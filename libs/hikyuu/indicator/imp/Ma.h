/*
 * Ma.h
 *
 *  Created on: 2013-2-10
 *      Author: fasiondog
 */

#ifndef MA_H_
#define MA_H_

#include "../Indicator.h"

namespace hku {

class Ma: public IndicatorImp {
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    Ma(int n = 22);
    Ma(const Indicator& indicator, int n = 22);
    virtual ~Ma();

    virtual string name() const;
    IndicatorImpPtr operator()(const Indicator& ind);
};

} /* namespace hku */
#endif /* MA_H_ */
