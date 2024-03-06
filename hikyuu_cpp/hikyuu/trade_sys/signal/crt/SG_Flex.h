/*
 * FLEX._SG.h
 *
 *  Created on: 2015年3月21日
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_SIGNAL_CRT_FLEX_SG_H_
#define TRADE_SYS_SIGNAL_CRT_FLEX_SG_H_

#include "../../../indicator/Indicator.h"
#include "../SignalBase.h"

namespace hku {

/**
 * 自交叉单线拐点指示器。
 * 使用自身的EMA(slow_n)作为慢线，自身作为快线，快线向上穿越慢线买入，快线向下穿越慢线卖出。
 * @param op
 * @param slow_n 慢线EMA周期
 * @return 信号指示器
 * @ingroup Signal
 */
SignalPtr HKU_API SG_Flex(const Indicator& op, int slow_n);

} /* namespace hku */

#endif /* TRADE_SYS_SIGNAL_CRT_FLEX_SG_H_ */
