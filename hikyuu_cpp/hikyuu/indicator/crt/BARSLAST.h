/*
 * BARSLAST.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-4
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_BARSLAST_H_
#define INDICATOR_CRT_BARSLAST_H_

#include "CVAL.h"

namespace hku {

/**
 * Position where the condition held last time; the number of periods from the last time the
 * condition held to the current one.
 * @details
 * <pre>
 * Usage: BARSLAST(X): the number of days from the last time X was not 0 until now.
 * For example: BARSLAST(CLOSE/REF(CLOSE,1)>=1.1) gives the number of periods from the last limit-up
 * until now.
 * </pre>
 * @ingroup Indicator
 */
Indicator HKU_API BARSLAST();

inline Indicator BARSLAST(const Indicator& ind) {
    return BARSLAST()(ind);
}

inline Indicator BARSLAST(Indicator::value_t val) {
    return BARSLAST(CVAL(val));
}

}  // namespace hku

#endif /* INDICATOR_CRT_BARSLAST_H_ */
