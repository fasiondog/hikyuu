/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-10-22
 *      Author: fasiondog
 */

#include "../../test_config.h"
#include <algorithm>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/multifactor/buildin_scfilter.h>

using namespace hku;

/**
 * @defgroup test_SCFilter_AmountLimit test_SCFilter_AmountLimit
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

namespace {

// A cross-section in score order whose amount order is exactly the reverse: the smallest amount
// gets the highest score. The filter keeps the largest amounts, so the kept part is the tail of
// the input, and only the output order tells whether the score order survived the filter.
struct CrossSection {
    ScoreRecordList scores;                 // score descending == amount ascending
    std::vector<price_t> amounts;           // parallel to scores
};

CrossSection buildReversedCrossSection(const KQuery& query, const Datetime& date) {
    StockManager& sm = StockManager::instance();
    std::vector<Stock> stks{sm["sz000004"], sm["sh600004"], sm["sh600005"], sm["sz000002"],
                            sm["sz000001"]};

    std::vector<std::pair<Stock, price_t>> valid;
    for (const auto& stk : stks) {
        if (stk.isNull()) {
            continue;
        }
        auto kr = stk.getKRecord(date, query.kType());
        if (kr.isValid()) {
            valid.emplace_back(stk, kr.transAmount);
        }
    }

    // amount ascending
    std::sort(valid.begin(), valid.end(),
              [](const auto& a, const auto& b) { return a.second < b.second; });

    CrossSection cs;
    size_t total = valid.size();
    for (size_t i = 0; i < total; i++) {
        cs.scores.emplace_back(valid[i].first, static_cast<ScoreRecord::value_t>(total - i));
        cs.amounts.emplace_back(valid[i].second);
    }
    return cs;
}

}  // namespace

/** @par Test points */
TEST_CASE("test_SCFilter_AmountLimit_keeps_score_order") {
    KQuery query = KQueryByDate(Datetime(20110104), Datetime(20111206), KQuery::DAY);
    Datetime date(201106010000);

    CrossSection cs = buildReversedCrossSection(query, date);
    REQUIRE(cs.scores.size() >= 4);

    auto filter = SCFilter_AmountLimit(0.4);
    ScoreRecordList ret = filter->filter(cs.scores, date, query);
    REQUIRE(!ret.empty());
    CHECK(ret.size() < cs.scores.size());

    /** @arg the output keeps the score order of the input, it is not re-sorted by amount */
    std::vector<price_t> out_amounts;
    for (const auto& sc : ret) {
        auto kr = sc.stock.getKRecord(date, query.kType());
        out_amounts.emplace_back(kr.transAmount);
    }
    CHECK(std::is_sorted(out_amounts.begin(), out_amounts.end()));

    /** @arg the kept part is the one with the largest amounts */
    price_t min_kept = *std::min_element(out_amounts.begin(), out_amounts.end());
    for (size_t i = 0; i < cs.scores.size(); i++) {
        bool dropped = true;
        for (const auto& sc : ret) {
            if (sc.stock == cs.scores[i].stock) {
                dropped = false;
                break;
            }
        }
        if (dropped) {
            CHECK(cs.amounts[i] <= min_kept);
        }
    }
}

/** @par Test points */
TEST_CASE("test_SCFilter_AmountLimit_ignores_input_order") {
    KQuery query = KQueryByDate(Datetime(20110104), Datetime(20111206), KQuery::DAY);
    Datetime date(201106010000);

    CrossSection cs = buildReversedCrossSection(query, date);
    REQUIRE(cs.scores.size() >= 4);

    // The same cross-section in the reverse (score ascending) order
    ScoreRecordList reversed(cs.scores.rbegin(), cs.scores.rend());

    auto filter = SCFilter_AmountLimit(0.4);
    ScoreRecordList a = filter->filter(cs.scores, date, query);
    ScoreRecordList b = filter->filter(reversed, date, query);

    /** @arg the amount filter keeps the largest amounts whatever the order of the input is */
    REQUIRE_EQ(a.size(), b.size());
    std::vector<string> kept_a, kept_b;
    for (const auto& sc : a) {
        kept_a.emplace_back(sc.stock.market_code());
    }
    for (const auto& sc : b) {
        kept_b.emplace_back(sc.stock.market_code());
    }
    std::sort(kept_a.begin(), kept_a.end());
    std::sort(kept_b.begin(), kept_b.end());
    CHECK(kept_a == kept_b);
}

/** @par Test points */
TEST_CASE("test_SCFilter_AmountLimit_boundaries_and_determinism") {
    KQuery query = KQueryByDate(Datetime(20110104), Datetime(20111206), KQuery::DAY);
    Datetime date(201106010000);

    CrossSection cs = buildReversedCrossSection(query, date);
    REQUIRE(cs.scores.size() >= 4);

    /** @arg limit = 0 keeps everything, still in the score order */
    auto all = SCFilter_AmountLimit(0.0);
    ScoreRecordList ret_all = all->filter(cs.scores, date, query);
    REQUIRE_EQ(ret_all.size(), cs.scores.size());
    for (size_t i = 0; i < ret_all.size(); i++) {
        CHECK_EQ(ret_all[i].stock, cs.scores[i].stock);
    }

    /** @arg limit = 1.0 keeps at least one */
    auto one = SCFilter_AmountLimit(1.0);
    ScoreRecordList ret_one = one->filter(cs.scores, date, query);
    CHECK_EQ(ret_one.size(), 1);
    CHECK_EQ(ret_one[0].stock, cs.scores.back().stock);  // the largest amount

    /** @arg repeated runs give the same result */
    auto filter = SCFilter_AmountLimit(0.4);
    ScoreRecordList a = filter->filter(cs.scores, date, query);
    ScoreRecordList b = filter->filter(cs.scores, date, query);
    REQUIRE_EQ(a.size(), b.size());
    for (size_t i = 0; i < a.size(); i++) {
        CHECK_EQ(a[i].stock, b[i].stock);
    }
}

/** @} */
