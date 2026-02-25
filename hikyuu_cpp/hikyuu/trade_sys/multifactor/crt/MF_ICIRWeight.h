/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-13
 *      Author: fasiondog
 */

#pragma once

#include "../MultiFactorBase.h"

namespace hku {

/**
 * @brief 创建滚动ICIR权重多因子模型实例
 * @ingroup MultiFactor
 * @return MultiFactorPtr 滚动ICIR权重多因子模型指针
 * @details 创建一个空的滚动ICIR权重多因子模型，需要后续设置因子集合
 */
MultiFactorPtr HKU_API MF_ICIRWeight();

/**
 * @brief 创建滚动ICIR权重多因子模型实例（完整参数版本）
 * @ingroup MultiFactor
 * @param stks 计算证券列表
 * @param query 日期范围查询条件
 * @param ref_stk 参考证券，用于日期对齐，默认为空（相当于SH000001）
 * @param ic_n 默认IC对应的N日收益率周期，默认为5
 * @param ic_rolling_n IC滚动窗口大小，默认为120
 * @param spearman 是否使用spearman计算相关系数，true为spearman，false为pearson，默认为true
 * @param mode 获取截面数据时的排序模式：0-降序，1-升序，2-不排序，默认为0
 * @param save_all_factors 是否保留所有因子数据，默认为false
 * @return MultiFactorPtr 滚动ICIR权重多因子模型指针
 * @details 
 * 创建滚动ICIR权重多因子模型实例，使用指定参数进行因子合成计算。
 * 该模型基于滚动窗口内的IC值计算IR权重，适用于动态调整因子权重的场景。
 * <pre>
 * 示例：
 * // 创建滚动ICIR权重模型
 * auto mf = MF_ICIRWeight(stocks, query, Stock("SH000001"), 5, 120, true, 0, false);
 * </pre>
 */
MultiFactorPtr HKU_API MF_ICIRWeight(const StockList& stks, const KQuery& query,
                                     const Stock& ref_stk = Stock(), int ic_n = 5,
                                     int ic_rolling_n = 120, bool spearman = true, int mode = 0,
                                     bool save_all_factors = false);

/**
 * @brief 创建滚动ICIR权重多因子模型实例（使用因子集版本）
 * @ingroup MultiFactor
 * @param factorset 因子集合
 * @param stks 计算证券列表
 * @param query 日期范围查询条件
 * @param ref_stk 参考证券，用于日期对齐，默认为空
 * @param ic_n 默认IC对应的N日收益率周期，默认为5
 * @param ic_rolling_n IC滚动窗口大小，默认为120
 * @param spearman 是否使用spearman计算相关系数，true为spearman，false为pearson，默认为true
 * @param mode 获取截面数据时的排序模式：0-降序，1-升序，2-不排序，默认为0
 * @param save_all_factors 是否保留所有因子数据，默认为false
 * @return MultiFactorPtr 滚动ICIR权重多因子模型指针
 * @details 
 * 创建滚动ICIR权重多因子模型实例，使用指定的因子集合进行计算。
 * 该模型基于滚动窗口内的IC值计算IR权重，适用于动态调整因子权重的场景。
 * <pre>
 * 示例：
 * // 使用因子集创建滚动ICIR权重模型
 * auto mf = MF_ICIRWeight(factor_set, stocks, query);
 * </pre>
 */
inline MultiFactorPtr MF_ICIRWeight(const FactorSet& factorset, const StockList& stks,
                                    const KQuery& query, const Stock& ref_stk = Stock(),
                                    int ic_n = 5, int ic_rolling_n = 120, bool spearman = true,
                                    int mode = 0, bool save_all_factors = false) {
    auto ret =
      MF_ICIRWeight(stks, query, ref_stk, ic_n, ic_rolling_n, spearman, mode, save_all_factors);
    ret->setRefFactorSet(factorset);
    return ret;
}

/**
 * @brief 创建滚动ICIR权重多因子模型实例（使用指标列表版本）
 * @ingroup MultiFactor
 * @param inds 指标列表，将自动转换为因子集
 * @param stks 计算证券列表
 * @param query 日期范围查询条件
 * @param ref_stk 参考证券，用于日期对齐，默认为空
 * @param ic_n 默认IC对应的N日收益率周期，默认为5
 * @param ic_rolling_n IC滚动窗口大小，默认为120
 * @param spearman 是否使用spearman计算相关系数，true为spearman，false为pearson，默认为true
 * @param mode 获取截面数据时的排序模式：0-降序，1-升序，2-不排序，默认为0
 * @param save_all_factors 是否保留所有因子数据，默认为false
 * @return MultiFactorPtr 滚动ICIR权重多因子模型指针
 * @details 
 * 创建滚动ICIR权重多因子模型实例，使用指标列表自动构建因子集进行计算。
 * 该模型基于滚动窗口内的IC值计算IR权重，适用于动态调整因子权重的场景。
 * <pre>
 * 示例：
 * // 使用指标列表创建滚动ICIR权重模型
 * auto mf = MF_ICIRWeight(indicators, stocks, query);
 * </pre>
 */
inline MultiFactorPtr MF_ICIRWeight(const IndicatorList& inds, const StockList& stks,
                                    const KQuery& query, const Stock& ref_stk = Stock(),
                                    int ic_n = 5, int ic_rolling_n = 120, bool spearman = true,
                                    int mode = 0, bool save_all_factors = false) {
    return MF_ICIRWeight(FactorSet(inds, query.kType()), stks, query, ref_stk, ic_n, ic_rolling_n,
                         spearman, mode, save_all_factors);
}

}  // namespace hku