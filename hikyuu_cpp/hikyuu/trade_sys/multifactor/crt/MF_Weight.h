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
 * @brief 创建指定权重多因子模型实例
 * @ingroup MultiFactor
 * @return MultiFactorPtr 指定权重多因子模型指针
 * @details 创建一个空的指定权重多因子模型，需要后续设置因子集合和权重
 */
MultiFactorPtr HKU_API MF_Weight();

/**
 * @brief 创建指定权重多因子模型实例（完整参数版本）
 * @ingroup MultiFactor
 * @param weights 权重列表，必须与因子数量相等
 * @param stks 计算证券列表
 * @param query 日期范围查询条件
 * @param ref_stk 参考证券，用于日期对齐，默认为空（相当于SH000001）
 * @param ic_n 默认IC对应的N日收益率周期，默认为5
 * @param spearman 是否使用spearman计算相关系数，true为spearman，false为pearson，默认为true
 * @param mode 获取截面数据时的排序模式：0-降序，1-升序，2-不排序，默认为0
 * @param save_all_factors 是否保留所有因子数据，默认为false
 * @return MultiFactorPtr 指定权重多因子模型指针
 * @details 
 * 创建指定权重多因子模型实例，使用指定参数进行因子合成计算。
 * 计算公式：factor = ind1 * w1 + ind2 * w2 + ... + indn * wn
 * 权重列表大小必须与因子数量相等，否则将抛出异常。
 * <pre>
 * 示例：
 * // 创建指定权重模型
 * PriceList weights = {0.3, 0.4, 0.3};  // 权重必须与因子数相等
 * auto mf = MF_Weight(weights, stocks, query, Stock("SH000001"), 5, true, 0, false);
 * </pre>
 */
MultiFactorPtr HKU_API MF_Weight(const PriceList& weights, const StockList& stks,
                                 const KQuery& query, const Stock& ref_stk = Stock(), int ic_n = 5,
                                 bool spearman = true, int mode = 0, bool save_all_factors = false);

/**
 * @brief 创建指定权重多因子模型实例（使用因子集版本）
 * @ingroup MultiFactor
 * @param factorset 因子集合
 * @param weights 权重列表，必须与因子集的因子数量相等
 * @param stks 计算证券列表
 * @param query 日期范围查询条件
 * @param ref_stk 参考证券，用于日期对齐，默认为空
 * @param ic_n 默认IC对应的N日收益率周期，默认为5
 * @param spearman 是否使用spearman计算相关系数，true为spearman，false为pearson，默认为true
 * @param mode 获取截面数据时的排序模式：0-降序，1-升序，2-不排序，默认为0
 * @param save_all_factors 是否保留所有因子数据，默认为false
 * @return MultiFactorPtr 指定权重多因子模型指针
 * @details 
 * 创建指定权重多因子模型实例，使用指定的因子集合和权重进行计算。
 * 计算公式：factor = ind1 * w1 + ind2 * w2 + ... + indn * wn
 * 特别注意：会严格检查权重列表大小是否与因子集大小相等，如果不相等将抛出异常。
 * 这是保证多因子合成正确性的关键检查，确保每个因子都有对应的权重。
 * <pre>
 * 示例：
 * // 使用因子集创建指定权重模型
 * PriceList weights = {0.3, 0.4, 0.3};  // 必须与因子数量匹配
 * auto mf = MF_Weight(factor_set, weights, stocks, query);
 * </pre>
 */
inline MultiFactorPtr MF_Weight(const FactorSet& factorset, const PriceList& weights,
                                const StockList& stks, const KQuery& query,
                                const Stock& ref_stk = Stock(), int ic_n = 5, bool spearman = true,
                                int mode = 0, bool save_all_factors = false) {
    HKU_CHECK(weights.size() == factorset.size(),
              "The size of weight is not equal to the size of factorset! weights.size()={}, "
              "factorset.size()={}",
              weights.size(), factorset.size());
    auto ret = MF_Weight(weights, stks, query, ref_stk, ic_n, spearman, mode, save_all_factors);
    ret->setRefFactorSet(factorset);
    return ret;
}

/**
 * @brief 创建指定权重多因子模型实例（使用指标列表版本）
 * @ingroup MultiFactor
 * @param inds 指标列表，将自动转换为因子集
 * @param weights 权重列表，必须与指标列表的长度相等
 * @param stks 计算证券列表
 * @param query 日期范围查询条件
 * @param ref_stk 参考证券，用于日期对齐，默认为空
 * @param ic_n 默认IC对应的N日收益率周期，默认为5
 * @param spearman 是否使用spearman计算相关系数，true为spearman，false为pearson，默认为true
 * @param mode 获取截面数据时的排序模式：0-降序，1-升序，2-不排序，默认为0
 * @param save_all_factors 是否保留所有因子数据，默认为false
 * @return MultiFactorPtr 指定权重多因子模型指针
 * @details 
 * 创建指定权重多因子模型实例，使用指标列表自动构建因子集并应用指定权重进行计算。
 * 计算公式：factor = ind1 * w1 + ind2 * w2 + ... + indn * wn
 * 特别注意：会严格检查权重列表大小是否与指标数量相等，这是保证多因子合成正确性的关键。
 * 如果权重数量与指标数量不匹配，将抛出异常，防止错误的因子合成结果。
 * <pre>
 * 示例：
 * // 使用指标列表创建指定权重模型
 * PriceList weights = {0.3, 0.4, 0.3};  // 必须与指标数量完全匹配
 * auto mf = MF_Weight(indicators, weights, stocks, query);
 * </pre>
 */
inline MultiFactorPtr MF_Weight(const IndicatorList& inds, const PriceList& weights,
                                const StockList& stks, const KQuery& query,
                                const Stock& ref_stk = Stock(), int ic_n = 5, bool spearman = true,
                                int mode = 0, bool save_all_factors = false) {
    return MF_Weight(FactorSet(inds, query.kType()), weights, stks, query, ref_stk, ic_n, spearman,
                     mode, save_all_factors);
}

}  // namespace hku