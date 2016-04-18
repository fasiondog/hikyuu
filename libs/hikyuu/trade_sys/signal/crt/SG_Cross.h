/*
 * CROSS_SG.h
 *
 *  Created on: 2015年2月20日
 *      Author: fasiondog
 */

#ifndef TRADE_SYS_SIGNAL_CRT_SG_CROSS_H_
#define TRADE_SYS_SIGNAL_CRT_SG_CROSS_H_

#include "../../../indicator/Operand.h"
#include "../SignalBase.h"

namespace hku {

/**
 * 双线交叉指示器，当快线从下向上穿越慢线时，买入；当快线从上向下穿越慢线时，卖出。
 * @param fast 快线
 * @param slow 慢线
 * @param kpart KDATA|OPEN|HIGH|LOW|CLOSE|AMO|VOL
 * @return 信号指示器
 * @ingroup Signal
 */
SignalPtr HKU_API SG_Cross(const Operand& fast,
        const Operand& slow, const string& kpart = "CLOSE");

} /* namespace hku */

#endif /* TRADE_SYS_SIGNAL_CRT_SG_CROSS_H_ */
