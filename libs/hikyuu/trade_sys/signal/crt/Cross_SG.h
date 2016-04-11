/*
 * CROSS_SG.h
 *
 *  Created on: 2015年2月20日
 *      Author: Administrator
 */

#ifndef TRADE_SYS_SIGNAL_CRT_CROSS_SG_H_
#define TRADE_SYS_SIGNAL_CRT_CROSS_SG_H_

#include "../../../indicator/Operand.h"
#include "../SignalBase.h"

namespace hku {

/**
 * 双曲线交叉信号指示器
 * @param fast 快线
 * @param slow 慢线
 * @return 信号指示器
 * @ingroup Signal
 */
SignalPtr HKU_API Cross_SG(const Operand& fast, const Operand& slow);

} /* namespace hku */

#endif /* TRADE_SYS_SIGNAL_CRT_CROSS_SG_H_ */
