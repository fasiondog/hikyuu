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
 * 信息比率（Information Ratio，IR）
 * @details
 * <pre>
 * 公式: (P-B) / TE
 * P: 组合收益率
 * B: 比较基准收益率
 * TE: 投资周期中每天的 p 和 b 之间的标准差
 * 实际使用时，P 一般为 TM 的资产曲线，B 为沪深 3000 收盘价，如:
 * ref_k = sm["sh000300"].get_kdata(query)
 * funds = my_tm.get_funds_curve(ref_k.get_datetime.list())
 * ir = IR(PRICELIST(funds), ref_k.close, 0)
 * </pre>
 * @note 如果希望通过IC值计算IR，请使用 ICIR 指标
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