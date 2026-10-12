/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-05-28
 *      Author: fasiondog
 */

#pragma once

#include "Indicator.h"

namespace hku {

/**
 * Perform a multiple linear regression analysis on a stock
 *
 * @details The return of the stock close price is used as the dependent variable, and the input
 * indicators are used as the independent variables for the multiple linear regression.
 * The regression model is: Y = alpha + beta1*X1 + beta2*X2 + ... + betan*Xn
 *
 * @note Non-finite handling strategy: if any factor or the return at a time point is non-finite
 *       (NaN or ±Inf), the data at that time point is discarded, but it does not affect the data at
 *       the other time points or the other factors. A rank-deficient design matrix (exact factor
 *       collinearity, which leaves no unique least-squares solution) makes the whole regression
 * fail and returns an empty vector.
 *
 * @param stk the stock object
 * @param query K-line query condition, used to get the time range and data type needed by the
 *              regression analysis
 * @param inds indicator list, used as the independent variables (factors) of the regression
 * @return the regression coefficient vector, the first element is alpha (the intercept), followed
 * by the beta coefficients; an empty vector is returned if the data is insufficient or the
 *         regression fails
 *
 * @example
 * @code
 * // C++ example
 * Stock stk = getStock("sh000001");
 * Indicator ind1 = MA(CLOSE(), 5);
 * Indicator ind2 = MA(CLOSE(), 10);
 * auto result = multi_regression(stk, KQuery(-252), ind1, ind2);
 * double alpha = result[0];  // Intercept
 * double beta1 = result[1];  // Coefficient of the first factor
 * double beta2 = result[2];  // Coefficient of the second factor
 * @endcode
 *
 * @ingroup Indicator
 */
HKU_API std::vector<double> multi_regression(const Stock& stk, const KQuery& query,
                                             const IndicatorList& inds);

/**
 * Perform a multiple linear regression analysis on a stock (variadic version)
 *
 * @details It has the same function as the above function, providing a variadic interface to pass
 * multiple indicators more conveniently
 *
 * @param stk the stock object
 * @param query K-line query condition
 * @param ind1 the first indicator (required)
 * @param inds more indicators (optional)
 * @return the regression coefficient vector
 *
 * @ingroup Indicator
 */
template <typename... IndicatorArgs>
std::vector<double> multi_regression(const Stock& stk, const KQuery& query, const Indicator& ind1,
                                     IndicatorArgs... inds) {
    return multi_regression(stk, query, IndicatorList{ind1, inds...});
}

/**
 * Perform a multiple linear regression analysis on a stock (full version)
 *
 * @details It returns the complete regression result, including the coefficients, the residual
 * sequence, the residual sum of squares and the R² value
 *
 * @param stk the stock object
 * @param query K-line query condition
 * @param inds indicator list, used as the independent variables (factors) of the regression
 * @return the regression result vector in the following format:
 *         [alpha, beta1, beta2, ..., betan, e1, e2, ..., en, RSS, R²]
 *         - alpha: the intercept
 *         - beta1~betan: the coefficient of every factor
 *         - e1~en: the residual of every data point (actual value - predicted value)
 *         - RSS: the residual sum of squares
 *         - R²: the coefficient of determination
 *         An empty vector is returned if the data is insufficient or the regression fails
 *
 * @ingroup Indicator
 */
HKU_API std::vector<double> multi_regression_full(const Stock& stk, const KQuery& query,
                                                  const IndicatorList& inds);

/**
 * Perform a multiple linear regression analysis on a stock (full version, variadic)
 *
 * @details It has the same function as the above function, providing a variadic interface
 *
 * @param stk the stock object
 * @param query K-line query condition
 * @param ind1 the first indicator (required)
 * @param inds more indicators (optional)
 * @return the regression result vector
 *
 * @ingroup Indicator
 */
template <typename... IndicatorArgs>
std::vector<double> multi_regression_full(const Stock& stk, const KQuery& query,
                                          const Indicator& ind1, IndicatorArgs... inds) {
    return multi_regression_full(stk, query, IndicatorList{ind1, inds...});
}

namespace detail {

/**
 * Cross-section Spearman rank correlation over one day's factor and return values.
 *
 * Non-finite pairs (NaN or ±Inf) are dropped before ranking (a stock with a missing factor
 * or return on day t is excluded from that day's cross-section), matching ISpearman. Returns
 * 0.0 when a valid cross-section collapses to zero variance ("no discrimination means no
 * predictive power"), so a downstream MA does not spread NaN. Null is returned when fewer
 * than two valid pairs remain.
 */
HKU_API Indicator::value_t cross_section_spearman(const Indicator::value_t* a,
                                                  const Indicator::value_t* b, size_t n);

/**
 * Cross-section Pearson correlation over one day's factor and return values.
 *
 * Matches ICorr's arithmetic (raw rolling sums): any non-finite value in the input propagates
 * through the sums and yields Null output (via the widened finite-positive variance guard),
 * with no per-stock filtering step. Zero variance also yields Null.
 */
HKU_API Indicator::value_t cross_section_pearson(const Indicator::value_t* a,
                                                 const Indicator::value_t* b, size_t n);

}  // namespace detail

}  // namespace hku