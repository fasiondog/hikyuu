/*
 * SG_Bool.h
 *
 *  Created on: 2017-7-2
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_SIGNAL_CRT_SG_BOOL_H_
#define TRADE_SYS_SIGNAL_CRT_SG_BOOL_H_

#include "../../../indicator/Indicator.h"
#include "../SignalBase.h"

namespace hku {

/**
 * Boolean signal generator
 * @param buy the buy indication (a value > 0 at the corresponding position of the result Indicator
 *            means a buy)
 * @param sell the sell indication (a value > 0 at the corresponding position of the result
 * Indicator means a sell)
 * @param alternate whether the buy and sell signals appear alternately, true by default
 * @return signal generator
 * @ingroup Signal
 */
SignalPtr HKU_API SG_Bool(const Indicator& buy, const Indicator& sell, bool alternate = true);

} /* namespace hku */

#endif /* TRADE_SYS_SIGNAL_CRT_SG_BOOL_H_ */
