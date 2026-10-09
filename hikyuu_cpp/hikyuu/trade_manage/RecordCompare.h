/*
 * RecordCompare.h
 *
 *  Created on: 2026-10-9
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_MANAGE_RECORD_COMPARE_H_
#define TRADE_MANAGE_RECORD_COMPARE_H_

#include <cmath>
#include "../DataType.h"

namespace hku {

// Single source of truth for the tolerance used when comparing record fields for
// equality, so that FundsRecord / CostRecord / TradeRecord / PositionRecord no
// longer each hardcode their own (previously drifting) value.
constexpr double RECORD_EQ_THRESHOLD = 0.0001;

inline bool recordNear(double a, double b, double eps = RECORD_EQ_THRESHOLD) {
    return std::fabs(a - b) < eps;
}

}  // namespace hku

#endif /* TRADE_MANAGE_RECORD_COMPARE_H_ */
