/*
 * SINGLE_SG.h
 *
 *  Created on: 2015-2-22
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_SIGNAL_CRT_SINGLE_SG_H_
#define TRADE_SYS_SIGNAL_CRT_SINGLE_SG_H_

#include "../../../indicator/Indicator.h"
#include "../SignalBase.h"

namespace hku {

/**
 * Single line inflection point signal
 * @details
 * <pre>
 * The curve inflection point algorithm given in "Smarter Trading" is used to judge the curve trend,
 * the formula is as follows:
 *
 *     filter = percentage * STDEV((AMA-AMA[1], N)
 *     Buy  When AMA - AMA[1] > filter
 *     or Buy When AMA - AMA[2] > filter
 *     or Buy When AMA - AMA[3] > filter
 * </pre>
 * @param ind
 * @param filter_n N-day period
 * @param filter_p filter percentage
 * @return
 * @ingroup Signal
 */
SignalPtr HKU_API SG_Single(const Indicator& ind, int filter_n = 20, double filter_p = 0.1);

/**
 * Single line inflection point signal 2
 * @details
 * <pre>
 * The curve inflection point algorithm given in "Smarter Trading" is used to judge the curve trend,
 * the formula is as follows:
 *
 *     filter = percentage * STDEV((AMA-AMA[1], N)
 *     Buy  When AMA - lowest(AMA,n) > filter
 *     Sell When highest(AMA, n) - AMA > filter
 * </pre>
 * @param ind
 * @param filter_n N-day period
 * @param filter_p filter percentage
 * @return
 * @ingroup Signal
 */
SignalPtr HKU_API SG_Single2(const Indicator& ind, int filter_n = 20, double filter_p = 0.1);

} /* namespace hku */

#endif /* TRADE_SYS_SIGNAL_CRT_SINGLE_SG_H_ */
