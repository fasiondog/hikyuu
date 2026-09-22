/*
 * IRef.h
 *
 *  Created on: 2015-3-21
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_RIGHTSHIFT_H_
#define INDICATOR_IMP_RIGHTSHIFT_H_

#include "../Indicator.h"

namespace hku {

/*
 * REF forward reference (i.e. shift right)
 * Reference the data of several periods before.
 * Usage: REF(X,A) references the X value A periods before.
 * For example: REF(CLOSE,1) means the close price of the previous period, which is the previous
 * close on the daily line.
 */
class IRef : public IndicatorImp {
    INDICATOR_IMP(IRef)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IRef();
    virtual ~IRef() override;
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */

#endif /* INDICATOR_IMP_RIGHTSHIFT_H_ */
