/*
 * INot.h
 *
 *  Created on: 2019-4-2
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_INOT_H_
#define INDICATOR_IMP_INOT_H_

#include "../Indicator.h"

namespace hku {

class INot : public IndicatorImp {
    INDICATOR_IMP(INot)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    INot();
    virtual ~INot();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_INOT_H_ */
