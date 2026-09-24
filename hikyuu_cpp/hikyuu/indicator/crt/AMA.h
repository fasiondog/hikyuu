/*
 * AMA.h
 *
 *  Created on: 2013-4-8
 *      Author: fasiondog
 */

#pragma once
#ifndef AMA_H_
#define AMA_H_

#include "../Indicator.h"

namespace hku {

/**
 * Perry J. Kaufman adaptive moving average, see "Smarter Trading" (2006, Guangdong Economy
 * Publishing House)
 * @param n the period window for calculating the average, it must be an integer greater than 2,
 *          10 days by default
 * @param fast_n the corresponding fast period N, 2 by default
 * @param slow_n the N value of the corresponding slow EMA line, 30 by default; the indicator
 *               converges when it exceeds about 60 and there is not much influence
 * @return it has 2 result sets, result(0) is AMA and result(1) is ER
 * @ingroup Indicator
 */
Indicator HKU_API AMA(int n = 10, int fast_n = 2, int slow_n = 30);
Indicator HKU_API AMA(int n, int fast_n, const IndParam& slow_n);
Indicator HKU_API AMA(int n, const IndParam& fast_n, int slow_n = 30);
Indicator HKU_API AMA(int n, const IndParam& fast_n, const IndParam& slow_n);

Indicator HKU_API AMA(const IndParam& n, int fast_n = 2, int slow_n = 30);
Indicator HKU_API AMA(const IndParam& n, int fast_n, const IndParam& slow_n);
Indicator HKU_API AMA(const IndParam& n, const IndParam& fast_n, int slow_n = 30);
Indicator HKU_API AMA(const IndParam& n, const IndParam& fast_n, const IndParam& slow_n);

/**
 * Perry J. Kaufman adaptive moving average, see "Smarter Trading" (2006, Guangdong Economy
 * Publishing House)
 * @param ind the data to be calculated
 * @param n the period window for calculating the average, it must be an integer greater than 2,
 *          10 days by default
 * @param fast_n the corresponding fast period N, 2 by default
 * @param slow_n the N value of the corresponding slow EMA line, 30 by default; the indicator
 *               converges when it exceeds about 60 and there is not much influence
 * @ingroup Indicator
 */
inline Indicator AMA(const Indicator& ind, int n = 10, int fast_n = 2, int slow_n = 30) {
    return AMA(n, fast_n, slow_n)(ind);
}

inline Indicator AMA(const Indicator& ind, int n, const IndParam& fast_n, int slow_n = 30) {
    return AMA(n, fast_n, slow_n)(ind);
}

inline Indicator AMA(const Indicator& ind, int n, int fast_n, const IndParam& slow_n) {
    return AMA(n, fast_n, slow_n)(ind);
}

inline Indicator AMA(const Indicator& ind, int n, const IndParam& fast_n, const IndParam& slow_n) {
    return AMA(n, fast_n, slow_n)(ind);
}

inline Indicator AMA(const Indicator& ind, const IndParam& n, int fast_n = 2, int slow_n = 30) {
    return AMA(n, fast_n, slow_n)(ind);
}

inline Indicator AMA(const Indicator& ind, const IndParam& n, const IndParam& fast_n,
                     int slow_n = 30) {
    return AMA(n, fast_n, slow_n)(ind);
}

inline Indicator AMA(const Indicator& ind, const IndParam& n, int fast_n, const IndParam& slow_n) {
    return AMA(n, fast_n, slow_n)(ind);
}

inline Indicator AMA(const Indicator& ind, const IndParam& n, const IndParam& fast_n,
                     const IndParam& slow_n) {
    return AMA(n, fast_n, slow_n)(ind);
}

inline Indicator AMA(const Indicator& ind, int n, const Indicator& fast_n, int slow_n) {
    return AMA(n, IndParam(fast_n), slow_n)(ind);
}

inline Indicator AMA(const Indicator& ind, int n, int fast_n, const Indicator& slow_n) {
    return AMA(n, fast_n, IndParam(slow_n))(ind);
}

inline Indicator AMA(const Indicator& ind, int n, const Indicator& fast_n,
                     const Indicator& slow_n) {
    return AMA(n, IndParam(fast_n), IndParam(slow_n))(ind);
}

inline Indicator AMA(const Indicator& ind, const Indicator& n, int fast_n = 2, int slow_n = 30) {
    return AMA(IndParam(n), fast_n, slow_n)(ind);
}

inline Indicator AMA(const Indicator& ind, const Indicator& n, const Indicator& fast_n,
                     int slow_n = 30) {
    return AMA(IndParam(n), IndParam(fast_n), slow_n)(ind);
}

inline Indicator AMA(const Indicator& ind, const Indicator& n, int fast_n,
                     const Indicator& slow_n) {
    return AMA(IndParam(n), fast_n, IndParam(slow_n))(ind);
}

inline Indicator AMA(const Indicator& ind, const Indicator& n, const Indicator& fast_n,
                     const Indicator& slow_n) {
    return AMA(IndParam(n), IndParam(fast_n), IndParam(slow_n))(ind);
}

}  // namespace hku

#endif /* AMA_H_ */
