/*
 * ILiuTongPang.h
 *
 *  Created on: 2019-3-6
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_ILIUTONGPAN_H_
#define INDICATOR_IMP_ILIUTONGPAN_H_

#include "../Indicator.h"

namespace hku {

class ILiuTongPan : public IndicatorImp {
    INDICATOR_IMP(ILiuTongPan)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ILiuTongPan();
    virtual ~ILiuTongPan() override;
};

} /* namespace hku */

#endif /* INDICATOR_IMP_ILIUTONGPAN_H_ */
