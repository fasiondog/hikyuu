/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-05-28
 *      Author: fasiondog
 */

#pragma once

#include "Indicator.h"

namespace hku {

/**
 * 对股票进行多元线性回归分析
 *
 * @details 使用股票收盘价的收益率作为因变量，输入的指标作为自变量进行多元线性回归。
 * 回归模型为：Y = alpha + beta1*X1 + beta2*X2 + ... + betan*Xn
 *
 * @note NaN值处理策略：如果某个时间点的任何因子或收益率为NaN，则该时间点的数据被舍弃，
 *       但不会影响其他时间点的数据和其他因子。
 *
 * @param stk 股票对象
 * @param query K线查询条件，用于获取回归分析所需的时间范围和数据类型
 * @param inds 指标列表，作为回归的自变量（因子）
 * @return 回归系数向量，第一个元素是alpha(截距)，后续是各个beta系数
 *         如果数据不足或回归失败，返回空向量
 *
 * @example
 * @code
 * // C++示例
 * Stock stk = getStock("sh000001");
 * Indicator ind1 = MA(CLOSE(), 5);
 * Indicator ind2 = MA(CLOSE(), 10);
 * auto result = multi_regression(stk, KQuery(-252), ind1, ind2);
 * double alpha = result[0];  // 截距项
 * double beta1 = result[1];  // 第一个因子的系数
 * double beta2 = result[2];  // 第二个因子的系数
 * @endcode
 *
 * @ingroup Indicator
 */
HKU_API std::vector<double> multi_regression(const Stock& stk, const KQuery& query,
                                             const IndicatorList& inds);

/**
 * 对股票进行多元线性回归分析（可变参数版本）
 *
 * @details 与上述函数功能相同，提供可变参数接口以便更便捷地传入多个指标
 *
 * @param stk 股票对象
 * @param query K线查询条件
 * @param ind1 第一个指标（必需）
 * @param inds 更多指标（可选）
 * @return 回归系数向量
 *
 * @ingroup Indicator
 */
template <typename... IndicatorArgs>
std::vector<double> multi_regression(const Stock& stk, const KQuery& query, const Indicator& ind1,
                                     IndicatorArgs... inds) {
    return multi_regression(stk, query, IndicatorList{ind1, inds...});
}

/**
 * 对股票进行多元线性回归分析（完整版本）
 *
 * @details 返回完整的回归结果，包括系数、残差序列、残差平方和和R²值
 *
 * @param stk 股票对象
 * @param query K线查询条件
 * @param inds 指标列表，作为回归的自变量（因子）
 * @return 回归结果向量，格式如下：
 *         [alpha, beta1, beta2, ..., betan, e1, e2, ..., en, RSS, R²]
 *         - alpha: 截距项
 *         - beta1~betan: 各因子系数
 *         - e1~en: 各数据点的残差（实际值-预测值）
 *         - RSS: 残差平方和
 *         - R²: 决定系数
 *         如果数据不足或回归失败，返回空向量
 *
 * @ingroup Indicator
 */
HKU_API std::vector<double> multi_regression_full(const Stock& stk, const KQuery& query,
                                                  const IndicatorList& inds);

/**
 * 对股票进行多元线性回归分析（完整版本，可变参数）
 *
 * @details 与上述函数功能相同，提供可变参数接口
 *
 * @param stk 股票对象
 * @param query K线查询条件
 * @param ind1 第一个指标（必需）
 * @param inds 更多指标（可选）
 * @return 回归结果向量
 *
 * @ingroup Indicator
 */
template <typename... IndicatorArgs>
std::vector<double> multi_regression_full(const Stock& stk, const KQuery& query,
                                          const Indicator& ind1, IndicatorArgs... inds) {
    return multi_regression_full(stk, query, IndicatorList{ind1, inds...});
}

}  // namespace hku