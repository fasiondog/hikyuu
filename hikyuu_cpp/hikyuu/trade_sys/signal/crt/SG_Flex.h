/*
 * FLEX._SG.h
 *
 *  Created on: 2015-3-21
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_SIGNAL_CRT_FLEX_SG_H_
#define TRADE_SYS_SIGNAL_CRT_FLEX_SG_H_

#include "../../../indicator/Indicator.h"
#include "../SignalBase.h"

namespace hku {

/**
 * Self-crossing single line inflection point indicator.
 * It uses its own EMA(slow_n) as the slow line and itself as the fast line: it buys when the fast
 * line crosses the slow line upward and sells when the fast line crosses the slow line downward.
 * @param op
 * @param slow_n the EMA period of the slow line
 * @return signal generator
 * @ingroup Signal
 */
SignalPtr HKU_API SG_Flex(const Indicator& op, int slow_n);

} /* namespace hku */

#endif /* TRADE_SYS_SIGNAL_CRT_FLEX_SG_H_ */
