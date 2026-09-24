/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-13
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_SIGNAL_CRT_SG_ONESIDE_H_
#define TRADE_SYS_SIGNAL_CRT_SG_ONESIDE_H_

#include "../../../indicator/Indicator.h"
#include "../SignalBase.h"

namespace hku {

/**
 * Build a one-sided signal (containing the buy or the sell signals only) from the input indicator;
 * a signal is added if the indicator value is greater than 0
 * @param ind the indicating indicator
 * @param is_buy whether the added signal is a buy or a sell signal
 * @return signal generator
 * @ingroup Signal
 */
SignalPtr HKU_API SG_OneSide(const Indicator& ind, bool is_buy);

/** Generate a one-sided buy signal */
inline SignalPtr SG_Buy(const Indicator& ind) {
    return SG_OneSide(ind, true);
}

/** Generate a one-sided sell signal */
inline SignalPtr SG_Sell(const Indicator& ind) {
    return SG_OneSide(ind, false);
}

} /* namespace hku */

#endif /* TRADE_SYS_SIGNAL_CRT_SG_ONESIDE_H_ */
