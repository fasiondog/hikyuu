/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-02
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_KALMAN_H_
#define INDICATOR_CRT_KALMAN_H_

#include "CVAL.h"

namespace hku {

/**
 * Kalman filter
 * @note Non-finite input (NaN or ±Inf) at bar i triggers a predict-only step (state held,
 * uncertainty grows by q); no permanent contamination. Leading non-finite values keep the filter
 * unseeded until the first finite observation, which then becomes the initial state.
 * @param q noise variance, must be > 0
 * @param r measurement noise variance, must be > 0
 * @ingroup Indicator
 */
Indicator HKU_API KALMAN(double q = 0.01, double r = 0.1);

inline Indicator KALMAN(const Indicator& ind, double q = 0.01, double r = 0.1) {
    return KALMAN(q, r)(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_KALMAN_H_ */
