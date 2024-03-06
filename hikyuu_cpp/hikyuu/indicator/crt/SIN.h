/*
 * SIN.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_SIN_H_
#define INDICATOR_CRT_SIN_H_

#include "CVAL.h"

namespace hku {

/**
 * 正弦值
 * @ingroup Indicator
 */
Indicator HKU_API SIN();
Indicator SIN(Indicator::value_t);
Indicator SIN(const Indicator& ind);

inline Indicator SIN(const Indicator& ind) {
    return SIN()(ind);
}

inline Indicator SIN(Indicator::value_t val) {
    return SIN(CVAL(val));
}

}  // namespace hku

#endif /* INDICATOR_CRT_SIN_H_ */
