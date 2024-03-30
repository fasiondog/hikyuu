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
 * @brief 等权重合成因子
 * @param inds 原始因子列表
 * @param stks 计算证券列表
 * @param query 日期范围
 * @param ref_stk 参考证券
 * @param ic_n 默认 IC 对应的 N 日收益率
 * @return MultiFactorPtr
 */
MultiFactorPtr HKU_API MF_EqualWeight(const IndicatorList& inds, const StockList& stks,
                                      const KQuery& query, const Stock& ref_stk, int ic_n = 5);

}  // namespace hku