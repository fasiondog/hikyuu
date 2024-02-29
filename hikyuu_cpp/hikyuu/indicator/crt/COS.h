/*
 * COS.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_COS_H_
#define INDICATOR_CRT_COS_H_

#include "CVAL.h"

namespace hku {

/**
 * 余弦值
 * @ingroup Indicator
 */
Indicator HKU_API COS();
Indicator COS(Indicator::value_t);
Indicator COS(const Indicator& ind);

inline Indicator COS(const Indicator& ind) {
    return COS()(ind);
}

inline Indicator COS(Indicator::value_t val) {
    return COS(CVAL(val));
}

}  // namespace hku

#endif /* INDICATOR_CRT_COS_H_ */
