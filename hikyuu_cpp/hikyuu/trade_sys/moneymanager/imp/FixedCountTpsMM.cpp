/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-20
 *      Author: fasiondog
 */

#include "FixedCountTpsMM.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::FixedCountTpsMM)
#endif

namespace hku {

FixedCountTpsMM::FixedCountTpsMM() : MoneyManagerBase("MM_FixedCountTpsMM") {}

FixedCountTpsMM::FixedCountTpsMM(const vector<double>& buy_counts,
                                 const vector<double>& sell_counts)
: MoneyManagerBase("MM_FixedCountTpsMM"), m_buy_counts(buy_counts), m_sell_counts(sell_counts) {
    double total_buy_count = 0.0;
    for (size_t i = 0, total = buy_counts.size(); i < total; i++) {
        HKU_CHECK(buy_counts[i] >= 0.0, "buy_counts[{}] must >= 0.0!", i);
        total_buy_count += buy_counts[i];
    }

    double total_sell_count = 0.0;
    for (size_t i = 0, total = sell_counts.size(); i < total; i++) {
        HKU_CHECK(sell_counts[i] >= 0.0, "sell_counts[{}] must >= 0.0!", i);
        total_sell_count += sell_counts[i];
    }

    HKU_WARN_IF(total_buy_count != total_sell_count,
                "The total number of buy ({}) and the total number ({}) of sell are not "
                "consistent, which may lead to an imbalance.",
                total_buy_count, total_sell_count);
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
    size_t current_sell_count = currentSellCount(stock);
    return (current_sell_count < m_sell_counts.size()) ? m_sell_counts[current_sell_count] : 0.0;
}

MoneyManagerPtr HKU_API MM_FixedCountTps(const vector<double>& buy_counts,
                                         const vector<double>& sell_counts) {
    return make_shared<FixedCountTpsMM>(buy_counts, sell_counts);
}

} /* namespace hku */
