/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-09-26
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/trade_sys/system/System.h"
#include "hikyuu/trade_manage/Performance.h"
#include "hikyuu/utilities/Log.h"

namespace hku {

/**
 * Get the combinations of the given sequence indexes, returning the combination set composed of the
 * sequence subscript indexes
 * @note The total number of the combinations is 2**n - 1, so the length of inputs needs to be
 *       controlled
 * @exception The maximum length supported for the input sequence is 15 only, HKUException is thrown
 *            when it is exceeded
 * @param inputs a sequence of any type
 * @return
 */
template <class T>
std::vector<std::vector<size_t>> combinateIndex(const std::vector<T>& inputs) {
    size_t total = inputs.size();
    HKU_CHECK(total <= 15, "The length of inputs exceeds the maximum limit!");

    std::vector<std::vector<size_t>> result;
    std::vector<size_t> current;
    for (size_t i = 0; i < total; i++) {
        for (size_t j = 0, len = result.size(); j < len; j++) {
            current.resize(result[j].size());
            std::copy(result[j].cbegin(), result[j].cend(), current.begin());
            current.push_back(i);
            result.push_back(std::move(current));
        }
        current.clear();
        current.push_back(i);
        result.push_back(std::move(current));
    }

    return result;
}

/**
 * @brief Combine the input indicator sequence
 * @details
 * For example, when the input is [ind1, ind2], the output is [EXIST(ind1,n), EXIST(ind2,n),
 * EXIST(ind1,n)&EXIST(ind2,n)]
 * @param inputs the indicator list to combine
 * @param n the indicator exists within n periods
 * @return std::vector<Indicator>
 */
std::vector<Indicator> HKU_API combinateIndicator(const std::vector<Indicator>& inputs, int n);

/**
 * @brief Combination test of the given security and the buy / sell signal indicators
 * @param stk the given security
 * @param query the given range condition
 * @param tm the given trade management instance
 * @param sys the given trading strategy instance
 * @param buy_inds buy signal indicator list
 * @param sell_inds sell signal indicator list
 * @param n combination time range parameter
 * @return std::map<std::string, Performance>
 */
std::map<std::string, Performance> HKU_API combinateIndicatorAnalysis(
  const Stock& stk, const KQuery& query, TradeManagerPtr tm, SystemPtr sys,
  const std::vector<Indicator>& buy_inds, const std::vector<Indicator>& sell_inds, int n);

/**
 * @brief Output result definition of combinateIndicatorAnalysisWithBlock
 */
struct HKU_API CombinateAnalysisOutput {
    string combinateName;  ///< Name of the buy / sell indicator combination
    string market_code;    ///< Security code
    string name;           ///< Security name
    PriceList values;      ///< The values of every statistics item

    CombinateAnalysisOutput() = default;
    CombinateAnalysisOutput(const CombinateAnalysisOutput&) = default;
    CombinateAnalysisOutput(CombinateAnalysisOutput&& rv)
    : combinateName(std::move(rv.combinateName)),
      market_code(std::move(rv.market_code)),
      name(std::move(rv.name)),
      values(std::move(rv.values)) {}

    CombinateAnalysisOutput& operator=(const CombinateAnalysisOutput&) = default;
    CombinateAnalysisOutput& operator=(CombinateAnalysisOutput&& rv) {
        HKU_IF_RETURN(this == &rv, *this);
        combinateName = std::move(rv.combinateName);
        market_code = std::move(rv.market_code);
        name = std::move(rv.name);
        values = std::move(rv.values);
        return *this;
    }
};

/**
 * @brief Combination test of the buy / sell signal indicators on the given security list
 * @param blk the given block
 * @param query the given range condition
 * @param tm the given trade instance
 * @param sys the given system strategy instance
 * @param buy_inds buy indicator list
 * @param sell_inds sell indicator list
 * @param n combination time range parameter
 * @return vector<CombinateAnalysisOutput>
 */
vector<CombinateAnalysisOutput> HKU_API combinateIndicatorAnalysisWithBlock(
  const Block& blk, const KQuery& query, TradeManagerPtr tm, SystemPtr sys,
  const std::vector<Indicator>& buy_inds, const std::vector<Indicator>& sell_inds, int n);

}  // namespace hku