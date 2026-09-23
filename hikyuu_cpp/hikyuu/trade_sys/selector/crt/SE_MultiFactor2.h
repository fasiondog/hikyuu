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
 * @brief Stock selection algorithm based on MultiFactor
 * @param mf the MultiFactor instance
 * @param filter the score filter
 * @return SelectorPtr
 * @ingroup Selector
 */
SelectorPtr HKU_API SE_MultiFactor2(const MFPtr& mf,
                                    const ScoresFilterPtr& filter = SCFilter_IgnoreNan());

/**
 * @brief Stock selection algorithm based on MultiFactor, it supports multiple factor input ways
 *
 * Three creation ways are supported:
 * 1. Use a pre-created MultiFactor object directly
 * 2. Create it directly with a FactorSet object
 * 3. Use an IndicatorList (it is converted into a FactorSet internally and automatically)
 *
 * @param factorset the factor input, it can be a FactorSet object or an IndicatorList
 * @param ic_n the ic_n day return corresponding to the ic
 * @param ic_rolling_n the period for calculating the rolling IC (i.e. the n-day moving average of
 *                     the IC)
 * @param ref_stk the reference security, used for the date alignment, sh000001 when it is not given
 * @param spearman spearman is used to calculate the correlation coefficient by default, otherwise
 *                 pearson
 * @param mode "MF_ICIRWeight" | "MF_ICWeight" | "MF_EqualWeight", the name of the factor
 *             synthesis algorithm
 * @param filter the score filter
 * @return SelectorPtr
 * @ingroup Selector
 */
SelectorPtr HKU_API SE_MultiFactor2(const FactorSet& factorset, int ic_n = 5,
                                    int ic_rolling_n = 120, const Stock& ref_stk = Stock(),
                                    bool spearman = true, const string& mode = "MF_ICIRWeight",
                                    const ScoresFilterPtr& filter = SCFilter_IgnoreNan());

/**
 * @brief Convenience interface for creating the MultiFactor2 stock selection algorithm based on an
 * IndicatorList
 * @details It converts the IndicatorList into a FactorSet internally and then calls the main
 *          function
 * @param src_inds the original factor list
 * @param ic_n the ic_n day return corresponding to the ic
 * @param ic_rolling_n the period for calculating the rolling IC (i.e. the n-day moving average of
 *                     the IC)
 * @param ref_stk the reference security, used for the date alignment, sh000001 when it is not given
 * @param spearman spearman is used to calculate the correlation coefficient by default, otherwise
 *                 pearson
 * @param mode "MF_ICIRWeight" | "MF_ICWeight" | "MF_EqualWeight", the name of the factor
 *             synthesis algorithm
 * @param filter the score filter
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