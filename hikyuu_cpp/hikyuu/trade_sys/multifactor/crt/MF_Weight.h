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
 * @brief Create a given weight multi-factor model instance
 * @ingroup MultiFactor
 * @return MultiFactorPtr the given weight multi-factor model pointer
 * @details Create an empty given weight multi-factor model; the factor set and the weights need to
 *          be set afterwards
 */
MultiFactorPtr HKU_API MF_Weight();

/**
 * @brief Create a given weight multi-factor model instance (the full parameter version)
 * @ingroup MultiFactor
 * @param weights the weight list, it must be equal to the number of the factors
 * @param stks the security list to be calculated
 * @param query the date range query condition
 * @param ref_stk the reference security, used for the date alignment, empty by default (equivalent
 *                to SH000001)
 * @param ic_n the N-day return period corresponding to the default IC, 5 by default
 * @param spearman whether to use spearman to calculate the correlation coefficient: true means
 *                 spearman, false means pearson, true by default
 * @param mode the sorting mode when getting the cross-section data: 0-descending, 1-ascending,
 *             2-no sorting, 0 by default
 * @param save_all_factors whether to keep all the factor data, false by default
 * @return MultiFactorPtr the given weight multi-factor model pointer
 * @details
 * Create a given weight multi-factor model instance, it uses the given parameters for the factor
 * synthesis calculation.
 * Calculation formula: factor = ind1 * w1 + ind2 * w2 + ... + indn * wn
 * The size of the weight list must be equal to the number of the factors, otherwise an exception is
 * thrown.
 * <pre>
 * Example:
 * // Create the given weight model
 * PriceList weights = {0.3, 0.4, 0.3};  // The weights must equal the number of the factors
 * auto mf = MF_Weight(weights, stocks, query, Stock("SH000001"), 5, true, 0, false);
 * </pre>
 */
MultiFactorPtr HKU_API MF_Weight(const PriceList& weights, const StockList& stks,
                                 const KQuery& query, const Stock& ref_stk = Stock(), int ic_n = 5,
                                 bool spearman = true, int mode = 0, bool save_all_factors = false);

/**
 * @brief Create a given weight multi-factor model instance (the factor set version)
 * @ingroup MultiFactor
 * @param factorset factor set
 * @param weights the weight list, it must be equal to the number of the factors in the factor set
 * @param stks the security list to be calculated
 * @param query the date range query condition
 * @param ref_stk the reference security, used for the date alignment, empty by default
 * @param ic_n the N-day return period corresponding to the default IC, 5 by default
 * @param spearman whether to use spearman to calculate the correlation coefficient: true means
 *                 spearman, false means pearson, true by default
 * @param mode the sorting mode when getting the cross-section data: 0-descending, 1-ascending,
 *             2-no sorting, 0 by default
 * @param save_all_factors whether to keep all the factor data, false by default
 * @return MultiFactorPtr the given weight multi-factor model pointer
 * @details
 * Create a given weight multi-factor model instance, it uses the given factor set and weights for
 * the calculation.
 * Calculation formula: factor = ind1 * w1 + ind2 * w2 + ... + indn * wn
 * Note: it strictly checks whether the size of the weight list is equal to the size of the factor
 * set, and an exception is thrown if they are not equal.
 * This is the key check guaranteeing the correctness of the multi-factor synthesis, ensuring that
 * every factor has its corresponding weight.
 * <pre>
 * Example:
 * // Create the given weight model with the factor set
 * PriceList weights = {0.3, 0.4, 0.3};  // It must match the number of the factors
 * auto mf = MF_Weight(factor_set, weights, stocks, query);
 * </pre>
 */
inline MultiFactorPtr MF_Weight(const FactorSet& factorset, const PriceList& weights,
                                const StockList& stks, const KQuery& query,
                                const Stock& ref_stk = Stock(), int ic_n = 5, bool spearman = true,
                                int mode = 0, bool save_all_factors = false) {
    HKU_CHECK(weights.size() == factorset.size(),
              "The size of weight is not equal to the size of factorset! weights.size()={}, "
              "factorset.size()={}",
              weights.size(), factorset.size());
    auto ret = MF_Weight(weights, stks, query, ref_stk, ic_n, spearman, mode, save_all_factors);
    ret->setRefFactorSet(factorset);
    return ret;
}

/**
 * @brief Create a given weight multi-factor model instance (the indicator list version)
 * @ingroup MultiFactor
 * @param inds the indicator list, it is converted into a factor set automatically
 * @param weights the weight list, it must be equal to the length of the indicator list
 * @param stks the security list to be calculated
 * @param query the date range query condition
 * @param ref_stk the reference security, used for the date alignment, empty by default
 * @param ic_n the N-day return period corresponding to the default IC, 5 by default
 * @param spearman whether to use spearman to calculate the correlation coefficient: true means
 *                 spearman, false means pearson, true by default
 * @param mode the sorting mode when getting the cross-section data: 0-descending, 1-ascending,
 *             2-no sorting, 0 by default
 * @param save_all_factors whether to keep all the factor data, false by default
 * @return MultiFactorPtr the given weight multi-factor model pointer
 * @details
 * Create a given weight multi-factor model instance, it builds the factor set automatically with
 * the indicator list and applies the given weights for the calculation.
 * Calculation formula: factor = ind1 * w1 + ind2 * w2 + ... + indn * wn
 * Note: it strictly checks whether the size of the weight list is equal to the number of the
 * indicators, which is the key to guaranteeing the correctness of the multi-factor synthesis.
 * If the number of the weights does not match the number of the indicators an exception is thrown,
 * preventing a wrong factor synthesis result.
 * <pre>
 * Example:
 * // Create the given weight model with the indicator list
 * PriceList weights = {0.3, 0.4, 0.3};  // It must match the number of the indicators exactly
 * auto mf = MF_Weight(indicators, weights, stocks, query);
 * </pre>
 */
inline MultiFactorPtr MF_Weight(const IndicatorList& inds, const PriceList& weights,
                                const StockList& stks, const KQuery& query,
                                const Stock& ref_stk = Stock(), int ic_n = 5, bool spearman = true,
                                int mode = 0, bool save_all_factors = false) {
    return MF_Weight(FactorSet(inds, query.kType()), weights, stks, query, ref_stk, ic_n, spearman,
                     mode, save_all_factors);
}

}  // namespace hku