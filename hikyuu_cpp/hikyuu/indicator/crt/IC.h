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
 * @note IC 原本需要 “t 时刻因子值→t+1 时刻收益”，改为计算 “t 时刻因子值→t 时刻之前 N
        天的收益”（比如过去 5 天的收益），并称之为 “当前 IC”。(否则当前值都会是缺失NA)
        如需严格“t 时刻因子值→t+1 时刻收益“计算，请设置 strict=True (注意此模式下, 后n位为 NA)
 * @param stks 证券组合
 * @param query 查询条件
 * @param ref_stk 参照证券，默认 sh000300 沪深300
 * @param n 时间窗口 (对应 n 日收益率)
 * @param spearman 使用 spearman 相关系数，否则为 pearson
 * @param strict 严格模式， 遵循IC定义“t 时刻因子值→t+1 时刻收益”
 * @return Indicator
 * @ingroup Indicator
 */
Indicator HKU_API IC(const StockList& stks, const KQuery& query,
                     const Stock& ref_stk = getStock("sh000300"), int n = 1, bool spearman = true,
                     bool strict = false);

Indicator HKU_API IC(const Block& blk, const KQuery& query,
                     const Stock& ref_stk = getStock("sh000300"), int n = 1, bool spearman = true,
                     bool strict = false);

inline Indicator IC(const Indicator& ind, const StockList& stks, const KQuery& query,
                    const Stock& ref_stk = getStock("sh000300"), int n = 1, bool spearman = true,
                    bool strict = false) {
    return IC(stks, query, ref_stk, n, spearman, strict)(ind);
}

inline Indicator IC(const Indicator& ind, const Block& blk, const KQuery& query,
                    const Stock& ref_stk = getStock("sh000300"), int n = 1, bool spearman = true,
                    bool strict = false) {
    return IC(blk, query, ref_stk, n, spearman, strict)(ind);
}

}  // namespace hku