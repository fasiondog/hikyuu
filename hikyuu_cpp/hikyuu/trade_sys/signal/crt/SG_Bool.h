/*
 * SG_Bool.h
 *
 *  Created on: 2017年7月2日
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_SIGNAL_CRT_SG_BOOL_H_
#define TRADE_SYS_SIGNAL_CRT_SG_BOOL_H_

#include "../../../indicator/Indicator.h"
#include "../SignalBase.h"

namespace hku {

/**
 * 布尔信号指示器
 * @param buy 买入指示（结果Indicator中相应位置>0则代表买入）
 * @param sell 卖出指示（结果Indicator中相应位置>0则代表卖出）
 * @return 信号指示器
 * @ingroup Signal
 */
SignalPtr HKU_API SG_Bool(const Indicator& buy, const Indicator& sell);

} /* namespace hku */

#endif /* TRADE_SYS_SIGNAL_CRT_SG_BOOL_H_ */
