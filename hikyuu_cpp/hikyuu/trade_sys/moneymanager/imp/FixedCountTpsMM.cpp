/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-20
 *      Author: fasiondog
 */

#include <cmath>
#include "hikyuu/utilities/arithmetic.h"
#include "FixedCountTpsMM.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedCountTpsMM)
#endif

namespace hku {

FixedCountTpsMM::FixedCountTpsMM() : MoneyManagerBase("MM_FixedCountTpsMM") {
    m_support_mult_buy_sell = true;
}

FixedCountTpsMM::FixedCountTpsMM(const vector<double>& buy_counts,
                                 const vector<double>& sell_counts)
: MoneyManagerBase("MM_FixedCountTpsMM"), m_buy_counts(buy_counts), m_sell_counts(sell_counts) {
    m_support_mult_buy_sell = true;

    HKU_CHECK(!buy_counts.empty(), "buy_counts must not be empty!");

    double total_buy_count = 0.0;
    for (size_t i = 0, total = buy_counts.size(); i < total; i++) {
        HKU_CHECK(std::isfinite(buy_counts[i]) && buy_counts[i] > 0.0,
                  "buy_counts[{}] must be finite and > 0.0!", i);
        total_buy_count += buy_counts[i];
    }

    double total_sell_count = 0.0;
    for (size_t i = 0, total = sell_counts.size(); i < total; i++) {
        HKU_CHECK(std::isfinite(sell_counts[i]) && sell_counts[i] > 0.0,
                  "sell_counts[{}] must be finite and > 0.0!", i);
        total_sell_count += sell_counts[i];
    }

    // Each tier is finite, but the accumulated sum may still overflow to +inf; a NaN diff in the
    // nearly_equal check below would silently swallow the imbalance warning
    HKU_CHECK(std::isfinite(total_buy_count) && std::isfinite(total_sell_count),
              "The total buy/sell count overflowed a finite double!");

    if (!nearly_equal(total_buy_count, total_sell_count)) {
        HKU_WARN(
          "The total number of buy ({}) and the total number ({}) of sell are not "
          "consistent, which may lead to an imbalance.",
          total_buy_count, total_sell_count);
    }
}

FixedCountTpsMM::~FixedCountTpsMM() {}

MoneyManagerPtr FixedCountTpsMM::_clone() {
    auto p = make_shared<FixedCountTpsMM>();
    p->m_buy_counts = m_buy_counts;
    p->m_sell_counts = m_sell_counts;
    return p;
}

double FixedCountTpsMM::_getBuyNumber(const Datetime& datetime, const Stock& stock, price_t price,
                                      price_t risk, SystemPart from) {
    size_t current_buy_count = currentBuyCount(stock);
    return (current_buy_count < m_buy_counts.size()) ? m_buy_counts[current_buy_count] : 0.0;
}

double FixedCountTpsMM::_getSellNumber(const Datetime& datetime, const Stock& stock, price_t price,
                                       price_t risk, SystemPart from) {
    // A non-positive risk means the stop-loss has been breached; since this MM bypasses the base
    // class "!(risk > 0.0) && !m_support_mult_buy_sell" fallback, the whole position must be
    // liquidated here to avoid the remaining shares never being sold
    if (!(risk > 0.0)) {
        return MAX_DOUBLE;
    }
    size_t current_sell_count = currentSellCount(stock);
    if (current_sell_count >= m_sell_counts.size()) {
        return MAX_DOUBLE;
    }
    // A tier above the remaining position would make the sell fail, and the failed sell does not
    // notify the MM, so the tier counter would never advance; settle the position instead
    double number = m_sell_counts[current_sell_count];
    return number > m_tm->getHoldNumber(datetime, stock) ? MAX_DOUBLE : number;
}

MoneyManagerPtr HKU_API MM_FixedCountTps(const vector<double>& buy_counts,
                                         const vector<double>& sell_counts) {
    return make_shared<FixedCountTpsMM>(buy_counts, sell_counts);
}

} /* namespace hku */
