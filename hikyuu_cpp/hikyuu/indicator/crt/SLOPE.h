/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-11-09
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_SLOPE_H_
#define INDICATOR_CRT_SLOPE_H_

#include "../Indicator.h"

namespace hku {

/**
 * Calculate the linear regression slope, the goodness of fit R² and the relative maximum residual;
 * N supports a variable
 * @param n time window
 * @return Indicator, it contains three result sets:
 *         - result(0): slope
 *         - result(1): goodness of fit R²
 *         - result(2): relative maximum residual RelMaxRes = max|yi - ŷi| / ȳ
 */
Indicator HKU_API SLOPE(int n = 22);
Indicator HKU_API SLOPE(const IndParam& n);

/**
 * Calculate the linear regression slope, the goodness of fit R² and the relative maximum residual;
 * N supports a variable
 * @param ind the indicator to be calculated
 * @param n time window
 * @return Indicator, it contains three result sets:
 *         - result(0): slope
 *         - result(1): goodness of fit R²
 *         - result(2): relative maximum residual RelMaxRes = max|yi - ŷi| / ȳ
 */
inline Indicator SLOPE(const Indicator& ind, int n = 22) {
    return SLOPE(n)(ind);
}

inline Indicator HKU_API SLOPE(const Indicator& ind, const IndParam& n) {
    return SLOPE(n)(ind);
}

inline Indicator SLOPE(const Indicator& ind, const Indicator& n) {
    return SLOPE(IndParam(n))(ind);
}

}  // namespace hku

#endif