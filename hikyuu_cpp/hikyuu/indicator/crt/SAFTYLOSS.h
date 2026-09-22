/*
 * SAFTYLOSS.h
 *
 *  Created on: 2013-4-12
 *      Author: fasiondog
 */

#pragma once
#ifndef SAFTYLOSS_H_
#define SAFTYLOSS_H_

#include "../Indicator.h"

namespace hku {

/**
 * Alexander Elder's safe zone stop-loss
 * @details
 * <pre>
 * See "Come Into My Trading Room" (2007, Earthquake Press) by Alexander Elder, P202
 * Calculation description: within the lookback period (generally 10 to 20 days), add up the lengths
 *         of all the downward crossings and divide by the number of the downward crossings to get
 *         the average noise (i.e. within the lookback period, the length by which every low price
 * is lower than the previous day's low price divided by the number of times), and subtract (the
 * previous day's average noise multiplied by a multiple) from today's low price to get the
 * stop-loss line. To offset the fluctuation and guarantee that the stop-loss line moves upward, the
 * highest value within N days (generally 3 days) is taken based on the above result
 * </pre>
 * @note: the first (lookback period width + the width for taking the highest value) points in the
 *        returned result are invalid
 * @param n1 the lookback time window for calculating the average noise, 10 days by default
 * @param n2 take the highest value within n2 days for the preliminary stop-loss line, 3 by default
 * @param p the noise coefficient, 2 by default
 * @ingroup Indicator
 */
Indicator HKU_API SAFTYLOSS(int n1 = 10, int n2 = 3, double p = 2.0);
Indicator HKU_API SAFTYLOSS(const IndParam& n1, const IndParam& n2, double p = 2.0);
Indicator HKU_API SAFTYLOSS(const IndParam& n1, const IndParam& n2, const IndParam& p);

/**
 * Alexander Elder's safe zone stop-loss
 * @details
 * <pre>
 * See "Come Into My Trading Room" (2007, Earthquake Press) by Alexander Elder, P202
 * Calculation description: within the lookback period (generally 10 to 20 days), add up the lengths
 *         of all the downward crossings and divide by the number of the downward crossings to get
 *         the average noise (i.e. within the lookback period, the length by which every low price
 * is lower than the previous day's low price divided by the number of times), and subtract (the
 * previous day's average noise multiplied by a multiple) from today's low price to get the
 * stop-loss line. To offset the fluctuation and guarantee that the stop-loss line moves upward, the
 * highest value within N days (generally 3 days) is taken based on the above result
 * </pre>
 * @note: the first (lookback period width + the width for taking the highest value) points in the
 *        returned result are invalid
 * @param data the input data, a single input
 * @param n1 the lookback time window for calculating the average noise, 10 days by default
 * @param n2 take the highest value within n2 days for the preliminary stop-loss line, 3 by default
 * @param p the noise coefficient, 2 by default
 * @ingroup Indicator
 */
inline Indicator SAFTYLOSS(const Indicator& data, int n1 = 10, int n2 = 3, double p = 2.0) {
    return SAFTYLOSS(n1, n2, p)(data);
}

inline Indicator SAFTYLOSS(const Indicator& data, const IndParam& n1, const IndParam& n2,
                           double p = 2.0) {
    return SAFTYLOSS(n1, n2, p)(data);
}

inline Indicator SAFTYLOSS(const Indicator& data, const IndParam& n1, const IndParam& n2,
                           const IndParam& p) {
    return SAFTYLOSS(n1, n2, p)(data);
}

inline Indicator SAFTYLOSS(const Indicator& data, const Indicator& n1, const Indicator& n2,
                           double p = 2.0) {
    return SAFTYLOSS(IndParam(n1), IndParam(n2), p)(data);
}

inline Indicator SAFTYLOSS(const Indicator& data, const Indicator& n1, const Indicator& n2,
                           const Indicator& p) {
    return SAFTYLOSS(IndParam(n1), IndParam(n2), IndParam(p))(data);
}

}  // namespace hku

#endif /* SAFTYLOSS_H_ */
