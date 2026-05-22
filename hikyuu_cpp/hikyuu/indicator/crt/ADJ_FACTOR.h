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

/**
 * @brief 计算复权开盘价指标
 *
 * 将开盘价按复权因子进行后复权处理，得到复权后的开盘价序列。
 * 计算公式：ADJ_OPEN = ADJ_FACTOR * OPEN
 *
 * @return Indicator 复权开盘价指标对象
 *
 * @details 设计目的：
 * - 本系列指标（ADJ_*）主要是为了配合因子管理系统快速计算后向等比复权因子而设计
 * - 在因子管理场景中，通过每日增量更新并存储因子值，可以高效地进行复权计算
 *
 * @warning 重要限制：
 * - **周期限制**：仅适用于日线周期。周线、月线等非日线周期存在对齐问题，结果可能不准确
 * - **依赖因子管理**：需要配合因子管理系统的因子值存储使用，每日调用 update_all_factors_values()
 * 更新保存因子值以保证准确性
 * - **与 RECOVER_EQUAL_FORWARD 的关系**：本指标与 RECOVER_EQUAL_FORWARD
 * 本质相同，若非因子管理场景，建议直接使用 RECOVER_EQUAL_FORWARD
 * - **计算起点**：两者均不从上市日期开始计算，而是从当前查询的K线数据起始点开始计算
 *
 * @see ADJ_FACTOR 复权因子
 * @see RECOVER_EQUAL_FORWARD 等比前复权
 */
inline Indicator ADJ_OPEN() {
    return ADJ_FACTOR() * OPEN();
}

inline Indicator ADJ_OPEN(const KData& kdata) {
    return ADJ_OPEN()(kdata);
}

/**
 * @brief 计算复权最高价指标
 *
 * 将最高价按复权因子进行后复权处理，得到复权后的最高价序列。
 * 计算公式：ADJ_HIGH = ADJ_FACTOR * HIGH
 *
 * @return Indicator 复权最高价指标对象
 *
 * @details 设计目的：
 * - 本系列指标（ADJ_*）主要是为了配合因子管理系统快速计算后向等比复权因子而设计
 * - 在因子管理场景中，通过每日增量更新并存储因子值，可以高效地进行复权计算
 *
 * @warning 重要限制：
 * - **周期限制**：仅适用于日线周期。周线、月线等非日线周期存在对齐问题，结果可能不准确
 * - **依赖因子管理**：需要配合因子管理系统的因子值存储使用，每日调用 update_all_factors_values()
 * 更新保存因子值以保证准确性
 * - **与 RECOVER_EQUAL_FORWARD 的关系**：本指标与 RECOVER_EQUAL_FORWARD
 * 本质相同，若非因子管理场景，建议直接使用 RECOVER_EQUAL_FORWARD
 * - **计算起点**：两者均不从上市日期开始计算，而是从当前查询的K线数据起始点开始计算
 *
 * @see ADJ_FACTOR 复权因子
 * @see RECOVER_EQUAL_FORWARD 等比前复权
 */
inline Indicator ADJ_HIGH() {
    return ADJ_FACTOR() * HIGH();
}

inline Indicator ADJ_HIGH(const KData& kdata) {
    return ADJ_HIGH()(kdata);
}

/**
 * @brief 计算复权最低价指标
 *
 * 将最低价按复权因子进行后复权处理，得到复权后的最低价序列。
 * 计算公式：ADJ_LOW = ADJ_FACTOR * LOW
 *
 * @return Indicator 复权最低价指标对象
 *
 * @details 设计目的：
 * - 本系列指标（ADJ_*）主要是为了配合因子管理系统快速计算后向等比复权因子而设计
 * - 在因子管理场景中，通过每日增量更新并存储因子值，可以高效地进行复权计算
 *
 * @warning 重要限制：
 * - **周期限制**：仅适用于日线周期。周线、月线等非日线周期存在对齐问题，结果可能不准确
 * - **依赖因子管理**：需要配合因子管理系统的因子值存储使用，每日调用 update_all_factors_values()
 * 更新保存因子值以保证准确性
 * - **与 RECOVER_EQUAL_FORWARD 的关系**：本指标与 RECOVER_EQUAL_FORWARD
 * 本质相同，若非因子管理场景，建议直接使用 RECOVER_EQUAL_FORWARD
 * - **计算起点**：两者均不从上市日期开始计算，而是从当前查询的K线数据起始点开始计算
 *
 * @see ADJ_FACTOR 复权因子
 * @see RECOVER_EQUAL_FORWARD 等比前复权
 */
inline Indicator ADJ_LOW() {
    return ADJ_FACTOR() * LOW();
}

inline Indicator ADJ_LOW(const KData& kdata) {
    return ADJ_LOW()(kdata);
}

/**
 * @brief 计算复权收盘价指标
 *
 * 将收盘价按复权因子进行后复权处理，得到复权后的收盘价序列。
 * 计算公式：ADJ_CLOSE = ADJ_FACTOR * CLOSE
 *
 * @return Indicator 复权收盘价指标对象
 *
 * @details 设计目的：
 * - 本系列指标（ADJ_*）主要是为了配合因子管理系统快速计算后向等比复权因子而设计
 * - 在因子管理场景中，通过每日增量更新并存储因子值，可以高效地进行复权计算
 *
 * @warning 重要限制：
 * - **周期限制**：仅适用于日线周期。周线、月线等非日线周期存在对齐问题，结果可能不准确
 * - **依赖因子管理**：需要配合因子管理系统的因子值存储使用，每日调用 update_all_factors_values()
 * 更新保存因子值以保证准确性
 * - **与 RECOVER_EQUAL_FORWARD 的关系**：本指标与 RECOVER_EQUAL_FORWARD
 * 本质相同，若非因子管理场景，建议直接使用 RECOVER_EQUAL_FORWARD
 * - **计算起点**：两者均不从上市日期开始计算，而是从当前查询的K线数据起始点开始计算
 *
 * @see ADJ_FACTOR 复权因子
 * @see RECOVER_EQUAL_FORWARD 等比前复权
 */
inline Indicator ADJ_CLOSE() {
    return ADJ_FACTOR() * CLOSE();
}

inline Indicator ADJ_CLOSE(const KData& kdata) {
    return ADJ_CLOSE()(kdata);
}

/**
 * @brief 计算复权成交量指标
 *
 * 将成交量按复权因子进行后复权处理，得到复权后的成交量序列。
 * 计算公式：ADJ_VOL = VOL / ADJ_FACTOR
 *
 * @return Indicator 复权成交量指标对象
 *
 * @details 设计目的：
 * - 本系列指标（ADJ_*）主要是为了配合因子管理系统快速计算后向等比复权因子而设计
 * - 在因子管理场景中，通过每日增量更新并存储因子值，可以高效地进行复权计算
 * -
 * 注意：成交量复权使用除法，与价格复权使用乘法相反。这是因为当股本增加时，每股对应的成交量应该相应减少
 *
 * @warning 重要限制：
 * - **周期限制**：仅适用于日线周期。周线、月线等非日线周期存在对齐问题，结果可能不准确
 * - **依赖因子管理**：需要配合因子管理系统的因子值存储使用，每日调用 update_all_factors_values()
 * 更新保存因子值以保证准确性
 * - **与 RECOVER_EQUAL_FORWARD 的关系**：本指标与 RECOVER_EQUAL_FORWARD
 * 本质相同，若非因子管理场景，建议直接使用 RECOVER_EQUAL_FORWARD
 * - **计算起点**：两者均不从上市日期开始计算，而是从当前查询的K线数据起始点开始计算
 *
 * @see ADJ_FACTOR 复权因子
 * @see RECOVER_EQUAL_FORWARD 等比前复权
 */
inline Indicator ADJ_VOL() {
    return VOL() / ADJ_FACTOR();
}

inline Indicator ADJ_VOL(const KData& kdata) {
    return ADJ_VOL()(kdata);
}

}  // namespace hku