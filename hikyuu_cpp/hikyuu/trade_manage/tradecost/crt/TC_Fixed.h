/*
 * TC_Fixed.h
 *
 *  Created on: 2026-11-07
 *      Author: fasiondog
 */

#pragma once
#ifndef CRTFIXEDTC_H_
#define CRTFIXEDTC_H_

#include "../TradeCostBase.h"

namespace hku {

/**
 * Fixed-rate cost for futures, FX, crypto and other markets without a dedicated model
 * @details
 * <pre>
 *   fee = max(commission * price * number * unit, lowest_commission) + per_lot * number + others
 *   sell adds stamptax * price * number * unit
 * </pre>
 *
 * @param commission commission ratio of the turnover value, 0 by default
 * @param lowestCommission the lowest commission value, 0 by default
 * @param stamptax stamp duty charged on sells only, 0 by default
 * @param perLot fixed fee per lot (contract), e.g. a futures exchange fee
 * @param others fixed extra charge (delivery / funding fees)
 * @see FixedTradeCost
 * @ingroup TradeCost
 */
TradeCostPtr HKU_API TC_Fixed(price_t commission = 0.0, price_t lowestCommission = 0.0,
                              price_t stamptax = 0.0, price_t perLot = 0.0, price_t others = 0.0);

void registerPart_TC_Fixed();

}  // namespace hku

#endif /* CRTFIXEDTC_H_ */
