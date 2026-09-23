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
 * Proportion of the profitable positions
 * @details
 * <pre>
 * Usage: WINNER(CLOSE) means the proportion of the profitable positions if sold at the current
 * close price.
 * For example: returning 0.1 means 10% of the positions are profitable; WINNER(10.5) means the
 * proportion of the profitable positions at the price of 10.5 yuan
 * This function is valid for the daily analysis period only.
 * </pre>
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
