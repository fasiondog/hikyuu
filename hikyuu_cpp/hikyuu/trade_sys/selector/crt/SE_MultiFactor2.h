/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-30
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/trade_sys/multifactor/MultiFactorBase.h"
#include "hikyuu/trade_sys/multifactor/buildin_scfilter.h"
#include "../SelectorBase.h"

namespace hku {

/**
 * 基于 MultiFactor 选股算法
 * @return SelectorPtr
 * @ingroup Selector
 */
SelectorPtr HKU_API SE_MultiFactor2(const MFPtr& mf,
                                    const ScoresFilterPtr& filter = SCFilter_IgnoreNan());

/**
 * 基于 MultiFactor 选股算法
 * @param src_inds 原始因子公式
 * @param ic_n ic 对应的 ic_n 日收益率
 * @param ic_rolling_n 计算滚动 IC （即 IC 的 n 日移动平均）周期
 * @param ref_stk 参照对比证券，未指定时，默认使用 sh000300 沪深300指数
 * @param spearman 默认使用 spearman 计算相关系数，否则为 pearson
 * @param mode "MF_ICIRWeight" | "MF_ICWeight" | "MF_EqualWeight" 因子合成算法名称
 * @return SelectorPtr
 * @ingroup Selector
 */
SelectorPtr HKU_API SE_MultiFactor2(const IndicatorList& src_inds, int ic_n = 5,
                                    int ic_rolling_n = 120, const Stock& ref_stk = Stock(),
                                    bool spearman = true, const string& mode = "MF_ICIRWeight",
                                    const ScoresFilterPtr& filter = SCFilter_IgnoreNan());

}  // namespace hku