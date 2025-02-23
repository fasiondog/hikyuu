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
 * 根据输入指标构建单边信号（单纯的只包含买入或卖出信号），如果指标值大于0，则加入信号
 * @param ind 指示指标
 * @param is_buy 加入的是买入信号还是卖出信号
 * @return 信号指示器
 * @ingroup Signal
 */
SignalPtr HKU_API SG_OneSide(const Indicator& ind, bool is_buy);

/** 生成单边买入信号 */
inline SignalPtr SG_Buy(const Indicator& ind) {
    return SG_OneSide(ind, true);
}

/** 生成单边卖出信号 */
inline SignalPtr SG_Sell(const Indicator& ind) {
    return SG_OneSide(ind, false);
}

} /* namespace hku */

#endif /* TRADE_SYS_SIGNAL_CRT_SG_ONESIDE_H_ */
