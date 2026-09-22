/*
 * Saftyloss_ST.h
 *
 *  Created on: 2013-4-21
 *      Author: fasiondog
 */

#pragma once
#ifndef SAFTYLOSS_ST_H_
#define SAFTYLOSS_ST_H_

#include "../imp/IndicatorStoploss.h"

namespace hku {

/**
 * Alexander Elder's safe zone stop-loss
 * @details
 * <pre>
 * See "Come Into My Trading Room" (2007, Earthquake Press) by Alexander Elder, P202
 * Calculation description: within the lookback period (generally 10 to 20 days), add up the lengths
 *         of all the downward crossings and divide by the number of the downward crossings to get
 *         the average noise, and subtract (the previous day's average noise multiplied by a
 * multiple) from today's low price to get the stop-loss line. To offset the fluctuation and
 * guarantee that the stop-loss line moves upward, the highest value within N days (generally 3
 * days) is taken based on the above result
 * </pre>
 * @note: the first (lookback period width + the width for taking the highest value) points in the
 *        returned result are invalid
 * @see SAFTYLOSS
 * @param n1 the lookback time window for calculating the average noise, 10 days by default
 * @param n2 take the highest value within n2 days for the preliminary stop-loss line, 3 by default
 * @param p the noise coefficient, 2 by default
 * @ingroup Stoploss
 */
StoplossPtr HKU_API ST_Saftyloss(int n1 = 10, int n2 = 3, double p = 2.0);

}  // namespace hku

#endif /* SAFTYLOSS_ST_H_ */
