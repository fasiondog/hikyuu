/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#pragma once

#include "MA.h"
#include "STDEV.h"

namespace hku {

/**
 * @brief 计算指定的因子相对于参考证券的 ICIR （实际为 RankIC）
 * @details IR:信息比率(Information Ratio,简称IR)=
 * IC的多周期均值/IC的标准方差，代表因子获取稳定Alpha的能力。
 * @note IC 原本需要 “t 时刻因子值→t+1 时刻收益”，改为计算 “t 时刻因子值→t 时刻之前 N
 *       天的收益”（比如过去 5 天的收益），并称之为 “当前 IC”。(否则当前值都会是缺失NA)
 *       如需严格“t 时刻因子值→t+1 时刻收益“计算，请设置 strict=True (注意此模式下, 后n位为 NA)
 * @param ind 因子公式
 * @param stks 证券组合
 * @param query 查询条件
 * @param ref_stk 参照证券，默认 sh000300 沪深300
 * @param n IC对应的N日收益率
 * @param rolling_n 滚动时间窗口
 * @param spearman 使用 spearman 相关系数，否则为 pearson
 * @param strict 是否严格模式
 * @return Indicator
 * @ingroup Indicator
 */
inline Indicator ICIR(const Indicator& ind, const StockList& stks, const KQuery& query,
                      const Stock& ref_stk = getStock("sh000300"), int n = 1, int rolling_n = 120,
                      bool spearman = true, bool strict = false) {
    Indicator ic = IC(ind, stks, query, ref_stk, n, spearman, strict);
    Indicator x = MA(ic, rolling_n) / STDEV(ic, rolling_n);
    x.name("ICIR");
    x.setParam<int>("n", n);
    x.setParam<int>("rolling_n", rolling_n);
    return x;
}

inline Indicator ICIR(const Indicator& ind, const Block& blk, const KQuery& query,
                      const Stock& ref_stk = getStock("sh000300"), int n = 1, int rolling_n = 120,
                      bool spearman = true, bool strict = false) {
    Indicator ic = IC(ind, blk, query, ref_stk, n, spearman, strict);
    Indicator x = MA(ic, rolling_n) / STDEV(ic, rolling_n);
    x.name("ICIR");
    x.setParam<int>("n", n);
    x.setParam<int>("rolling_n", rolling_n);
    return x;
}

}  // namespace hku