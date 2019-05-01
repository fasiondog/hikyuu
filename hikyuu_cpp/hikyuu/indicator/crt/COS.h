/*
 * COS.h
 *
 *  Copyright (c) 2019 hikyuu.org
 * 
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#ifndef INDICATOR_CRT_COS_H_
#define INDICATOR_CRT_COS_H_

#include "CVAL.h"

namespace hku {

/**
 * 余弦值
 * @ingroup Indicator
 */
Indicator HKU_API COS();
Indicator COS(price_t);
Indicator COS(const Indicator& ind);

inline Indicator COS(const Indicator& ind) {
    return COS()(ind);
}

inline Indicator COS(price_t val) {
    return COS(CVAL(val));
}

}

#endif /* INDICATOR_CRT_COS_H_ */
