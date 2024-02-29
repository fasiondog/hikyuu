/*
 * REVERSE.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_REVERSE_H_
#define INDICATOR_CRT_REVERSE_H_

#include "CVAL.h"

namespace hku {

/**
 * 求相反数，REVERSE(X)返回-X
 * @ingroup Indicator
 */
Indicator HKU_API REVERSE();
Indicator REVERSE(Indicator::value_t);
Indicator REVERSE(const Indicator& ind);

inline Indicator REVERSE(const Indicator& ind) {
    return REVERSE()(ind);
}

inline Indicator REVERSE(Indicator::value_t val) {
    return REVERSE(CVAL(val));
}

}  // namespace hku

#endif /* INDICATOR_CRT_REVERSE_H_ */
