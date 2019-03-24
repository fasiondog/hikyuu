/*
 * ILiuTongPang.h
 *
 *  Created on: 2019年3月6日
 *      Author: fasiondog
 */

#ifndef INDICATOR_IMP_ILIUTONGPAN_H_
#define INDICATOR_IMP_ILIUTONGPAN_H_

#include "../Indicator.h"

namespace hku {

class ILiuTongPan: public IndicatorImp {
    INDICATOR_IMP(ILiuTongPan)
    INDICATOR_IMP_NEED_CONTEXT(ILiuTongPan)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ILiuTongPan();
    ILiuTongPan(const KData&);
    virtual ~ILiuTongPan();
};

} /* namespace hku */

#endif /* INDICATOR_IMP_ILIUTONGPAN_H_ */
