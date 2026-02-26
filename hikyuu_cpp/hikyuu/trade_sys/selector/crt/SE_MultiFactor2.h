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
 * @brief 基于 MultiFactor 选股算法
 * @param mf MultiFactor 实例
 * @param filter 评分过滤器
 * @return SelectorPtr
 * @ingroup Selector
 */
SelectorPtr HKU_API SE_MultiFactor2(const MFPtr& mf,
                                    const ScoresFilterPtr& filter = SCFilter_IgnoreNan());

/**
 * @brief 基于 MultiFactor 选股算法，支持多种因子输入方式
 * 
 * 支持三种创建方式：
 * 1. 直接使用预创建的 MultiFactor 对象
 * 2. 使用 FactorSet 对象直接创建
 * 3. 使用 IndicatorList（内部自动转换为 FactorSet）
 * 
 * @param factorset 因子输入，可以是 FactorSet 对象或 IndicatorList
 * @param ic_n ic 对应的 ic_n 日收益率
 * @param ic_rolling_n 计算滚动 IC （即 IC 的 n 日移动平均）周期
 * @param ref_stk 参考证券,用于日期对齐，未指定时为 sh000001
 * @param spearman 默认使用 spearman 计算相关系数，否则为 pearson
 * @param mode "MF_ICIRWeight" | "MF_ICWeight" | "MF_EqualWeight" 因子合成算法名称
 * @param filter 评分过滤器
 * @return SelectorPtr
 * @ingroup Selector
 */
SelectorPtr HKU_API SE_MultiFactor2(const FactorSet& factorset, int ic_n = 5,
                                    int ic_rolling_n = 120, const Stock& ref_stk = Stock(),
                                    bool spearman = true, const string& mode = "MF_ICIRWeight",
                                    const ScoresFilterPtr& filter = SCFilter_IgnoreNan());

/**
 * @brief 基于 IndicatorList 创建 MultiFactor2 选股算法的便捷接口
 * @details 内部将 IndicatorList 转换为 FactorSet 后调用主函数
 * @param src_inds 原始因子列表
 * @param ic_n ic 对应的 ic_n 日收益率
 * @param ic_rolling_n 计算滚动 IC （即 IC 的 n 日移动平均）周期
 * @param ref_stk 参考证券,用于日期对齐，未指定时为 sh000001
 * @param spearman 默认使用 spearman 计算相关系数，否则为 pearson
 * @param mode "MF_ICIRWeight" | "MF_ICWeight" | "MF_EqualWeight" 因子合成算法名称
 * @param filter 评分过滤器
 * @return SelectorPtr
 * @ingroup Selector
 */
inline SelectorPtr SE_MultiFactor2(const IndicatorList& src_inds, int ic_n = 5,
                                   int ic_rolling_n = 120, const Stock& ref_stk = Stock(),
                                   bool spearman = true, const string& mode = "MF_ICIRWeight",
                                   const ScoresFilterPtr& filter = SCFilter_IgnoreNan()) {
    return SE_MultiFactor2(FactorSet(src_inds), ic_n, ic_rolling_n, ref_stk, spearman, mode,
                           filter);
}

}  // namespace hku