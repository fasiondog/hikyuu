/*
 * Diff.h
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#ifndef DIFF_H_
#define DIFF_H_

#include "../Indicator.h"

namespace hku {

/*
 * 差分指标，即 a[i] - a[i-1]
 */
class Diff: public hku::IndicatorImp {
    INDICATOR_IMP(Diff)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    Diff();
    virtual ~Diff();
};

} /* namespace hku */
#endif /* DIFF_H_ */
