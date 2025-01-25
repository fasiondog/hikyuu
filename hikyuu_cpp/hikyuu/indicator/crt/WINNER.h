/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-25
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_WINNER_H_
#define INDICATOR_CRT_WINNER_H_

#include "CVAL.h"

namespace hku {

/**
 * 求绝对值
 * @ingroup Indicator
 */
Indicator HKU_API WINNER();

inline Indicator WINNER(const Indicator& ind) {
    return WINNER()(ind);
}

inline Indicator WINNER(Indicator::value_t val) {
    return WINNER(CVAL(val));
}

}  // namespace hku

#endif /* INDICATOR_CRT_WINNER_H_ */
