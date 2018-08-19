/*
 * Weave.h
 *
 *  Created on: 2017年4月19日
 *      Author: Administrator
 */

#ifndef INDICATOR_IMP_WEAVE_H_
#define INDICATOR_IMP_WEAVE_H_

#include "../Indicator.h"

namespace hku {

class Weave: public IndicatorImp {
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    Weave();
    virtual ~Weave();

    virtual bool check();
    virtual void _calculate(const Indicator& data);
    virtual IndicatorImpPtr operator()(const Indicator& ind);
};

} /* namespace hku */

#endif /* INDICATOR_IMP_WEAVE_H_ */
