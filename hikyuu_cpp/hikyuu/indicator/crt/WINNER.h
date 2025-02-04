/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-25
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_WINNER_H_
#define INDICATOR_CRT_WINNER_H_

#include "CVAL.h"

namespace hku {

/**
 * 获利盘比例
 * @details
 * <pre>
 * 用法: WINNER(CLOSE)　表示以当前收市价卖出的获利盘比例。
 * 例如: 返回0.1表示10%获利盘;WINNER(10.5)表示10.5元价格的获利盘比例
 * 该函数仅对日线分析周期有效。
 * </pre>
 * @ingroup Indicator
 */
Indicator HKU_API WINNER();

inline Indicator WINNER(const Indicator& ind) {
    return WINNER()(ind);
}

inline Indicator WINNER(Indicator::value_t val) {
    return WINNER(CVAL(val));
}

}  // namespace hku

#endif /* INDICATOR_CRT_WINNER_H_ */
