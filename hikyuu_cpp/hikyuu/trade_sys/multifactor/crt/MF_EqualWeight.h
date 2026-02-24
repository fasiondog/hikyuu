/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-13
 *      Author: fasiondog
 */

#pragma once
#include "../MultiFactorBase.h"

namespace hku {

MultiFactorPtr HKU_API MF_EqualWeight();

/**
 * @brief 等权重合成因子
 * @param inds 原始因子列表
 * @param stks 计算证券列表
 * @param query 日期范围
 * @param ref_stk 参考证券, 用于日期对齐, 未指定时为 SH000001
 * @param ic_n 默认 IC 对应的 N 日收益率
 * @param spearman 默认使用 spearman 计算相关系数，否则为 pearson
 * @param mode 获取截面数据时排序模式: 0-降序, 1-升序, 2-不排序
 * @param save_all_factors 是否保留因子数据
 * @return MultiFactorPtr
 */

MultiFactorPtr HKU_API MF_EqualWeight(const StockList& stks, const KQuery& query,
                                      const Stock& ref_stk, int ic_n, bool spearman, int mode,
                                      bool save_all_factors);

inline MultiFactorPtr MF_EqualWeight(const IndicatorList& inds, const StockList& stks,
                                     const KQuery& query, const Stock& ref_stk = Stock(),
                                     int ic_n = 5, bool spearman = true, int mode = 0,
                                     bool save_all_factors = false) {
    auto ret = MF_EqualWeight(stks, query, ref_stk, ic_n, spearman, mode, save_all_factors);
    ret->setRefIndicators(inds);
    return ret;
}

inline MultiFactorPtr MF_EqualWeight(const FactorSet& factset, const StockList& stks,
                                     const KQuery& query, const Stock& ref_stk = Stock(),
                                     int ic_n = 5, bool spearman = true, int mode = 0,
                                     bool save_all_factors = false) {
    auto ret = MF_EqualWeight(stks, query, ref_stk, ic_n, spearman, mode, save_all_factors);
    ret->setRefFactorSet(factset);
    return ret;
}

}  // namespace hku