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
 * @brief Create an equal weight multi-factor model instance
 * @ingroup MultiFactor
 * @return MultiFactorPtr the equal weight multi-factor model pointer
 * @details Create an empty equal weight multi-factor model; the factor set needs to be set
 *          afterwards
 */
MultiFactorPtr HKU_API MF_EqualWeight();

/**
 * @brief Create an equal weight multi-factor model instance (the full parameter version)
 * @ingroup MultiFactor
 * @param stks the security list to be calculated
 * @param query the date range query condition
 * @param ref_stk the reference security, used for the date alignment, SH000001 by default
 * @param ic_n the N-day return period corresponding to the default IC, 5 by default
 * @param spearman whether to use spearman to calculate the correlation coefficient: true means
 *                 spearman, false means pearson, true by default
 * @param mode the sorting mode when getting the cross-section data: 0-descending, 1-ascending,
 *             2-no sorting, 0 by default
 * @param save_all_factors whether to keep all the factor data, false by default
 * @return MultiFactorPtr the equal weight multi-factor model pointer
 * @details
 * Create an equal weight multi-factor model instance, it uses the given parameters for the factor
 * synthesis calculation.
 * <pre>
 * Example:
 * // Create the equal weight model
 * auto mf = MF_EqualWeight(stocks, query, Stock("SH000001"), 5, true, 0, false);
 * </pre>
 */
MultiFactorPtr HKU_API MF_EqualWeight(const StockList& stks, const KQuery& query,
                                      const Stock& ref_stk, int ic_n, bool spearman, int mode,
                                      bool save_all_factors);

/**
 * @brief Create an equal weight multi-factor model instance (the factor set version)
 * @ingroup MultiFactor
 * @param factset factor set
 * @param stks the security list to be calculated
 * @param query the date range query condition
 * @param ref_stk the reference security, used for the date alignment, empty by default
 * @param ic_n the N-day return period corresponding to the default IC, 5 by default
 * @param spearman whether to use spearman to calculate the correlation coefficient: true means
 *                 spearman, false means pearson, true by default
 * @param mode the sorting mode when getting the cross-section data: 0-descending, 1-ascending,
 *             2-no sorting, 0 by default
 * @param save_all_factors whether to keep all the factor data, false by default
 * @return MultiFactorPtr the equal weight multi-factor model pointer
 * @details
 * Create an equal weight multi-factor model instance, it uses the given factor set for the
 * calculation.
 * <pre>
 * Example:
 * // Create the equal weight model with the factor set
 * auto mf = MF_EqualWeight(factor_set, stocks, query);
 * </pre>
 */
inline MultiFactorPtr MF_EqualWeight(const FactorSet& factset, const StockList& stks,
                                     const KQuery& query, const Stock& ref_stk = Stock(),
                                     int ic_n = 5, bool spearman = true, int mode = 0,
                                     bool save_all_factors = false) {
    auto ret = MF_EqualWeight(stks, query, ref_stk, ic_n, spearman, mode, save_all_factors);
    ret->setRefFactorSet(factset);
    return ret;
}

/**
 * @brief Create an equal weight multi-factor model instance (the indicator list version)
 * @ingroup MultiFactor
 * @param inds the indicator list, it is converted into a factor set automatically
 * @param stks the security list to be calculated
 * @param query the date range query condition
 * @param ref_stk the reference security, used for the date alignment, empty by default
 * @param ic_n the N-day return period corresponding to the default IC, 5 by default
 * @param spearman whether to use spearman to calculate the correlation coefficient: true means
 *                 spearman, false means pearson, true by default
 * @param mode the sorting mode when getting the cross-section data: 0-descending, 1-ascending,
 *             2-no sorting, 0 by default
 * @param save_all_factors whether to keep all the factor data, false by default
 * @return MultiFactorPtr the equal weight multi-factor model pointer
 * @details
 * Create an equal weight multi-factor model instance, it builds the factor set automatically with
 * the indicator list for the calculation.
 * <pre>
 * Example:
 * // Create the equal weight model with the indicator list
 * auto mf = MF_EqualWeight(indicators, stocks, query);
 * </pre>
 */
inline MultiFactorPtr MF_EqualWeight(const IndicatorList& inds, const StockList& stks,
                                     const KQuery& query, const Stock& ref_stk = Stock(),
                                     int ic_n = 5, bool spearman = true, int mode = 0,
                                     bool save_all_factors = false) {
    return MF_EqualWeight(FactorSet(inds, query.kType()), stks, query, ref_stk, ic_n, spearman,
                          mode, save_all_factors);
}

}  // namespace hku