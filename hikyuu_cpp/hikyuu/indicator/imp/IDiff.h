/*
 * IDiff.h
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#pragma once
#ifndef IDIFF_H_
#define IDIFF_H_

#include "../Indicator.h"

namespace hku {

/*
 * Difference indicator, i.e. a[i] - a[i-n]
 * Parameters: n: difference period, 1 by default
 */
class IDiff : public hku::IndicatorImp {
    INDICATOR_IMP(IDiff)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IDiff();
    virtual ~IDiff() override;

    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */
#endif /* IDIFF_H_ */