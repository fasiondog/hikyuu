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
 * 差分指标，即 a[i] - a[i-1]
 */
class IDiff : public hku::IndicatorImp {
    INDICATOR_IMP(IDiff)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IDiff();
    virtual ~IDiff();
};

} /* namespace hku */
#endif /* IDIFF_H_ */
