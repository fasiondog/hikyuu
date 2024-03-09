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