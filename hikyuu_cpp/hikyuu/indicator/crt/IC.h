/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-09
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/Block.h"
#include "../Indicator.h"

namespace hku {

/**
 * @brief 计算指定的因子相对于参考证券的 IC （实际为 RankIC）
 * @param stks 证券组合
 * @param query 查询条件
 * @param ref_stk 参照证券，默认 sh000300 沪深300
 * @param n 时间窗口 (对应 n 日收益率)
 * @param spearman 使用 spearman 相关系数，否则为 pearson
 * @return Indicator
 * @ingroup Indicator
 */
Indicator HKU_API IC(const StockList& stks, const KQuery& query,
                     const Stock& ref_stk = getStock("sh000300"), int n = 1, bool spearman = true);

Indicator HKU_API IC(const Block& blk, const KQuery& query,
                     const Stock& ref_stk = getStock("sh000300"), int n = 1, bool spearman = true);

inline Indicator IC(const Indicator& ind, const StockList& stks, const KQuery& query,
                    const Stock& ref_stk = getStock("sh000300"), int n = 1, bool spearman = true) {
    return IC(stks, query, ref_stk, n, spearman)(ind);
}

inline Indicator IC(const Indicator& ind, const Block& blk, const KQuery& query,
                    const Stock& ref_stk = getStock("sh000300"), int n = 1, bool spearman = true) {
    return IC(blk, query, ref_stk, n, spearman)(ind);
}

}  // namespace hku