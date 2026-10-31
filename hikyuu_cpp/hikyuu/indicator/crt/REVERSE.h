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
#include "hikyuu/part/PartRegister.h"

namespace hku {

/**
 * Calculate the opposite number, REVERSE(X) returns -X
 * @ingroup Indicator
 */
Indicator HKU_API REVERSE();

inline Indicator REVERSE(const Indicator& ind) {
    return REVERSE()(ind);
}

inline Indicator REVERSE(Indicator::value_t val) {
    return REVERSE(CVAL(val));
}

inline void registerPart_REVERSE() {
    registerIndicatorPart("core.ind.reverse", [] { return REVERSE(); });
}

}  // namespace hku

#endif /* INDICATOR_CRT_REVERSE_H_ */
