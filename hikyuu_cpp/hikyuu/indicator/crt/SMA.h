/*
 * SMA.h
 *
 *  Created on: 2015-2-16
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_SMA_H_
#define INDICATOR_CRT_SMA_H_

#include "../Indicator.h"

namespace hku {

/**
 * Calculate the moving average
 * @details
 * <pre>
 * Usage: if Y=SMA(X,N,M) then Y=[M*X+(N-M)*Y')/N, where Y' is the Y value of the previous period
 * </pre>
 * @param n the period window for calculating the average, it must be an integer greater than 0
 * @param m coefficient
 * @ingroup Indicator
 */
Indicator HKU_API SMA(int n = 22, double m = 2.0);
Indicator HKU_API SMA(int, const IndParam& m);
Indicator HKU_API SMA(const IndParam& n, double m = 2.0);
Indicator HKU_API SMA(const IndParam& n, const IndParam& m);

/**
 * Calculate the moving average
 * @details
 * <pre>
 * Usage: if Y=SMA(X,N,M) then Y=[M*X+(N-M)*Y')/N, where Y' is the Y value of the previous period
 * </pre>
 * @param ind the data to be calculated
 * @param n the period window for calculating the average, it must be an integer greater than 0
 * @param m coefficient
 * @ingroup Indicator
 */
inline Indicator SMA(const Indicator& ind, int n = 22, double m = 2.0) {
    return SMA(n, m)(ind);
}

inline Indicator SMA(const Indicator& ind, int n, const IndParam& m) {
    return SMA(n, m)(ind);
}

inline Indicator SMA(const Indicator& ind, const IndParam& n, double m = 2.0) {
    return SMA(n, m)(ind);
}

inline Indicator SMA(const Indicator& ind, const IndParam& n, const IndParam& m) {
    return SMA(n, m)(ind);
}

inline Indicator SMA(const Indicator& ind, int n, const Indicator& m) {
    return SMA(n, IndParam(m))(ind);
}

inline Indicator SMA(const Indicator& ind, const Indicator& n, double m = 2.0) {
    return SMA(IndParam(n), m)(ind);
}

inline Indicator SMA(const Indicator& ind, const Indicator& n, const Indicator& m) {
    return SMA(IndParam(n), IndParam(m))(ind);
}

}  // namespace hku

#endif /* INDICATOR_CRT_SMA_H_ */
