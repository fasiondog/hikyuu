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
 * @param n 时间窗口
 * @param ref_stk 参照证券，默认 sh000300 沪深300
 * @return Indicator
 * @ingroup Indicator
 */
Indicator HKU_API IC(const StockList& stks, const KQuery& query, int n = 1,
                     const Stock& ref_stk = getStock("sh000300"));

Indicator HKU_API IC(const Block& blk, const KQuery& query, int n = 1,
                     const Stock& ref_stk = getStock("sh000300"));

inline Indicator IC(const Indicator& ind, const StockList& stks, const KQuery& query, int n = 1,
                    const Stock& ref_stk = getStock("sh000300")) {
    return IC(stks, query, n, ref_stk)(ind);
}

inline Indicator IC(const Indicator& ind, const Block& blk, const KQuery& query, int n = 1,
                    const Stock& ref_stk = getStock("sh000300")) {
    return IC(blk, query, n, ref_stk)(ind);
}

}  // namespace hku