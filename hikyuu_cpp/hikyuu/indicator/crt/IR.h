/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#pragma once

#include "ROCP.h"
#include "STDEV.h"

namespace hku {

/**
 * Information Ratio (IR)

 * @details
 * <pre>
 * Formula: (P-B) / TE

 * P: portfolio return

 * B: benchmark return

 * TE: the standard deviation between the daily p and b in the investment period

 * In practice, P is generally the asset curve of TM and B is the close price of the CSI 300, e.g.:

 * ref_k = sm["sh000300"].get_kdata(query)
 * funds = my_tm.get_funds_curve(ref_k.get_datetime.list())
 * ir = IR(PRICELIST(funds), ref_k.close, 0)
 * </pre>
 * @note If the IR is expected to be calculated from the IC values, please use the ICIR indicator

 * @param p
 * @param b
 * @param n
 * @ingroup Indicator
 */
inline Indicator IR(const Indicator& p, const Indicator& b, int n = 100) {
    Indicator p_return = ROCP(p, n);
    Indicator b_return = ROCP(b, n);
    Indicator x = (p_return - b_return);
    Indicator ret = x / STDEV(x, n);
    ret.name("IR");
    ret.setParam<int>("n", n);
    return ret;
}

}  // namespace hku