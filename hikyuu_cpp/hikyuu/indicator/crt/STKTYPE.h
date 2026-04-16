/*
 * STKTYPE.h
 *
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-04-17
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_CRT_STKTYPE_H_
#define INDICATOR_CRT_STKTYPE_H_

#include "../Indicator.h"

namespace hku {

/**
 * 获取股票类型指标
 * 
 * 返回当前股票的类型值（StockType枚举值）
 * 
 * @param k K线数据上下文（可选）
 * @return 指标对象，所有位置的值均为该股票的type值
 * @ingroup Indicator
 * 
 * @par 示例:
 * @code
 * // 使用默认上下文
 * auto stktype = STKTYPE();
 * 
 * // 指定K线数据
 * Stock stock = sm.getStock("sh000001");
 * KData kdata = stock.getKData(KQuery(0, 100));
 * auto stktype = STKTYPE(kdata);
 * @endcode
 */
Indicator HKU_API STKTYPE();
Indicator HKU_API STKTYPE(const KData& k);

}  // namespace hku

#endif /* INDICATOR_CRT_STKTYPE_H_ */
