/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-13
 *      Author: fasiondog
 */

#pragma once

#include "../MultiFactorBase.h"

namespace hku {

/**
 * @brief Create a rolling ICIR weight multi-factor model instance
 * @ingroup MultiFactor
 * @return MultiFactorPtr the rolling ICIR weight multi-factor model pointer
 * @details Create an empty rolling ICIR weight multi-factor model; the factor set needs to be set
 *          afterwards
 */
MultiFactorPtr HKU_API MF_ICIRWeight();

/**
 * @brief Create a rolling ICIR weight multi-factor model instance (the full parameter version)
 * @ingroup MultiFactor
 * @param stks the security list to be calculated
 * @param query the date range query condition
 * @param ref_stk the reference security, used for the date alignment, empty by default (equivalent
 *                to SH000001)
 * @param ic_n the N-day return period corresponding to the default IC, 5 by default
 * @param ic_rolling_n the IC rolling window size, 120 by default
 * @param spearman whether to use spearman to calculate the correlation coefficient: true means
 *                 spearman, false means pearson, true by default
 * @param mode the sorting mode when getting the cross-section data: 0-descending, 1-ascending,
 *             2-no sorting, 0 by default
 * @param save_all_factors whether to keep all the factor data, false by default
 * @return MultiFactorPtr the rolling ICIR weight multi-factor model pointer
 * @details
 * Create a rolling ICIR weight multi-factor model instance, it uses the given parameters for the
 * factor synthesis calculation.
 * This model calculates the IR weights from the IC values within the rolling window; it is suitable
 * for the scenarios where the factor weights are adjusted dynamically.
 * <pre>
 * Example:
 * // Create the rolling ICIR weight model
 * auto mf = MF_ICIRWeight(stocks, query, Stock("SH000001"), 5, 120, true, 0, false);
 * </pre>
 */
MultiFactorPtr HKU_API MF_ICIRWeight(const StockList& stks, const KQuery& query,
                                     const Stock& ref_stk = Stock(), int ic_n = 5,
                                     int ic_rolling_n = 120, bool spearman = true, int mode = 0,
                                     bool save_all_factors = false);

/**
 * @brief Create a rolling ICIR weight multi-factor model instance (the factor set version)
 * @ingroup MultiFactor
 * @param factorset factor set
 * @param stks the security list to be calculated
 * @param query the date range query condition
 * @param ref_stk the reference security, used for the date alignment, empty by default
 * @param ic_n the N-day return period corresponding to the default IC, 5 by default
 * @param ic_rolling_n the IC rolling window size, 120 by default
 * @param spearman whether to use spearman to calculate the correlation coefficient: true means
 *                 spearman, false means pearson, true by default
 * @param mode the sorting mode when getting the cross-section data: 0-descending, 1-ascending,
 *             2-no sorting, 0 by default
 * @param save_all_factors whether to keep all the factor data, false by default
 * @return MultiFactorPtr the rolling ICIR weight multi-factor model pointer
 * @details
 * Create a rolling ICIR weight multi-factor model instance, it uses the given factor set for the
 * calculation.
 * This model calculates the IR weights from the IC values within the rolling window; it is suitable
 * for the scenarios where the factor weights are adjusted dynamically.
 * <pre>
 * Example:
 * // Create the rolling ICIR weight model with the factor set
 * auto mf = MF_ICIRWeight(factor_set, stocks, query);
 * </pre>
 */
inline MultiFactorPtr MF_ICIRWeight(const FactorSet& factorset, const StockList& stks,
                                    const KQuery& query, const Stock& ref_stk = Stock(),
                                    int ic_n = 5, int ic_rolling_n = 120, bool spearman = true,
                                    int mode = 0, bool save_all_factors = false) {
    auto ret =
      MF_ICIRWeight(stks, query, ref_stk, ic_n, ic_rolling_n, spearman, mode, save_all_factors);
    ret->setRefFactorSet(factorset);
    return ret;
}

/**
 * @brief Create a rolling ICIR weight multi-factor model instance (the indicator list version)
 * @ingroup MultiFactor
 * @param inds the indicator list, it is converted into a factor set automatically
 * @param stks the security list to be calculated
 * @param query the date range query condition
 * @param ref_stk the reference security, used for the date alignment, empty by default
 * @param ic_n the N-day return period corresponding to the default IC, 5 by default
 * @param ic_rolling_n the IC rolling window size, 120 by default
 * @param spearman whether to use spearman to calculate the correlation coefficient: true means
 *                 spearman, false means pearson, true by default
 * @param mode the sorting mode when getting the cross-section data: 0-descending, 1-ascending,
 *             2-no sorting, 0 by default
 * @param save_all_factors whether to keep all the factor data, false by default
 * @return MultiFactorPtr the rolling ICIR weight multi-factor model pointer
 * @details
 * Create a rolling ICIR weight multi-factor model instance, it builds the factor set automatically
 * with the indicator list for the calculation.
 * This model calculates the IR weights from the IC values within the rolling window; it is suitable
 * for the scenarios where the factor weights are adjusted dynamically.
 * <pre>
 * Example:
 * // Create the rolling ICIR weight model with the indicator list
 * auto mf = MF_ICIRWeight(indicators, stocks, query);
 * </pre>
 */
inline MultiFactorPtr MF_ICIRWeight(const IndicatorList& inds, const StockList& stks,
                                    const KQuery& query, const Stock& ref_stk = Stock(),
                                    int ic_n = 5, int ic_rolling_n = 120, bool spearman = true,
                                    int mode = 0, bool save_all_factors = false) {
    return MF_ICIRWeight(FactorSet(inds, query.kType()), stks, query, ref_stk, ic_n, ic_rolling_n,
                         spearman, mode, save_all_factors);
}

}  // namespace hku