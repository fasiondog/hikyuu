/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-05-17
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"
#include "KDATA.h"

namespace hku {

/**
 * @brief 计算复权因子指标
 *
 * 基于股票的权息数据（送股、配股、转增、现金分红等）计算后复权因子序列。
 * 复权因子表示如果上市时持有1股，经过所有送股、配股、转增后，现在持有多少股。
 * 采用累乘方式计算，确保价格、成交量和成交金额的复权处理一致性。
 *
 * 该指标需要设置 KData 上下文才能正常工作，通过 setContext() 方法设置。
 *
 * @return Indicator 复权因子指标对象
 *
 * @par 使用示例:
 * @code{.cpp}
 * // 获取某只股票的复权因子
 * Stock stock = sm.getStock("sh000001");
 * KData kdata = stock.getKData(Query(-100));
 * Indicator adj_factor = ADJ_FACTOR();
 * adj_factor.setContext(kdata);
 * @endcode
 *
 * @see ADJ_OPEN 复权开盘价
 * @see ADJ_HIGH 复权最高价
 * @see ADJ_LOW 复权最低价
 * @see ADJ_CLOSE 复权收盘价
 * @see ADJ_VOL 复权成交量
 */
Indicator HKU_API ADJ_FACTOR();

Indicator HKU_API ADJ_FACTOR(const KData& kdata);

inline Indicator ADJ_OPEN() {
    return ADJ_FACTOR() * OPEN();
}

inline Indicator ADJ_HIGH() {
    return ADJ_FACTOR() * HIGH();
}

inline Indicator ADJ_LOW() {
    return ADJ_FACTOR() * LOW();
}

inline Indicator ADJ_CLOSE() {
    return ADJ_FACTOR() * CLOSE();
}

inline Indicator ADJ_VOL() {
    return VOL() / ADJ_FACTOR();
}

inline Indicator ADJ_AMO() {
    return AMO();  // 成交量不需要复权
}

}  // namespace hku