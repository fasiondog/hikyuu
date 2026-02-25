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
 * @brief 创建等权重多因子模型实例
 * @ingroup MultiFactor
 * @return MultiFactorPtr 等权重多因子模型指针
 * @details 创建一个空的等权重多因子模型，需要后续设置因子集合
 */
MultiFactorPtr HKU_API MF_EqualWeight();

/**
 * @brief 创建等权重多因子模型实例（完整参数版本）
 * @ingroup MultiFactor
 * @param stks 计算证券列表
 * @param query 日期范围查询条件
 * @param ref_stk 参考证券，用于日期对齐，默认为SH000001
 * @param ic_n 默认IC对应的N日收益率周期，默认为5
 * @param spearman 是否使用spearman计算相关系数，true为spearman，false为pearson，默认为true
 * @param mode 获取截面数据时的排序模式：0-降序，1-升序，2-不排序，默认为0
 * @param save_all_factors 是否保留所有因子数据，默认为false
 * @return MultiFactorPtr 等权重多因子模型指针
 * @details 
 * 创建等权重多因子模型实例，使用指定参数进行因子合成计算。
 * <pre>
 * 示例：
 * // 创建等权重模型
 * auto mf = MF_EqualWeight(stocks, query, Stock("SH000001"), 5, true, 0, false);
 * </pre>
 */
MultiFactorPtr HKU_API MF_EqualWeight(const StockList& stks, const KQuery& query,
                                      const Stock& ref_stk, int ic_n, bool spearman, int mode,
                                      bool save_all_factors);

/**
 * @brief 创建等权重多因子模型实例（使用因子集版本）
 * @ingroup MultiFactor
 * @param factset 因子集合
 * @param stks 计算证券列表
 * @param query 日期范围查询条件
 * @param ref_stk 参考证券，用于日期对齐，默认为空
 * @param ic_n 默认IC对应的N日收益率周期，默认为5
 * @param spearman 是否使用spearman计算相关系数，true为spearman，false为pearson，默认为true
 * @param mode 获取截面数据时的排序模式：0-降序，1-升序，2-不排序，默认为0
 * @param save_all_factors 是否保留所有因子数据，默认为false
 * @return MultiFactorPtr 等权重多因子模型指针
 * @details 
 * 创建等权重多因子模型实例，使用指定的因子集合进行计算。
 * <pre>
 * 示例：
 * // 使用因子集创建等权重模型
 * auto mf = MF_EqualWeight(factor_set, stocks, query);
 * </pre>
 */
inline MultiFactorPtr MF_EqualWeight(const FactorSet& factset, const StockList& stks,
                                     const KQuery& query, const Stock& ref_stk = Stock(),
                                     int ic_n = 5, bool spearman = true, int mode = 0,
                                     bool save_all_factors = false) {
    auto ret = MF_EqualWeight(stks, query, ref_stk, ic_n, spearman, mode, save_all_factors);
    ret->setRefFactorSet(factset);
    return ret;
}

/**
 * @brief 创建等权重多因子模型实例（使用指标列表版本）
 * @ingroup MultiFactor
 * @param inds 指标列表，将自动转换为因子集
 * @param stks 计算证券列表
 * @param query 日期范围查询条件
 * @param ref_stk 参考证券，用于日期对齐，默认为空
 * @param ic_n 默认IC对应的N日收益率周期，默认为5
 * @param spearman 是否使用spearman计算相关系数，true为spearman，false为pearson，默认为true
 * @param mode 获取截面数据时的排序模式：0-降序，1-升序，2-不排序，默认为0
 * @param save_all_factors 是否保留所有因子数据，默认为false
 * @return MultiFactorPtr 等权重多因子模型指针
 * @details 
 * 创建等权重多因子模型实例，使用指标列表自动构建因子集进行计算。
 * <pre>
 * 示例：
 * // 使用指标列表创建等权重模型
 * auto mf = MF_EqualWeight(indicators, stocks, query);
 * </pre>
 */
inline MultiFactorPtr MF_EqualWeight(const IndicatorList& inds, const StockList& stks,
                                     const KQuery& query, const Stock& ref_stk = Stock(),
                                     int ic_n = 5, bool spearman = true, int mode = 0,
                                     bool save_all_factors = false) {
    return MF_EqualWeight(FactorSet(inds, query.kType()), stks, query, ref_stk, ic_n, spearman,
                          mode, save_all_factors);
}

}  // namespace hku