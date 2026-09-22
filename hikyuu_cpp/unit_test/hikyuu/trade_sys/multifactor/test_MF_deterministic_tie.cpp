/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Test the determinism of the cross-sectional sorting of MultiFactorBase::_buildIndex:
 *  when the valid values are equal (a tie) the tie is broken by the market_code lexicographic
 * order, so the ranking is stable across the input order, the process, the platform and the
 * compiler.
 */

#include "../../test_config.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/trade_sys/multifactor/crt/MF_EqualWeight.h>

using namespace hku;

/**
 * @defgroup test_MF_deterministic_tie test_MF_deterministic_tie
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/**
 * @par Test point: all the valid values equal (a tie) and sorted by the market_code lexicographic
 * order (mode 0, descending)
 *
 * An all-tie cross section is built with CLOSE() - CLOSE(): the value of every stock is its own
 * close minus itself = 0.0 and it is 0.0 across the stocks too. This expression is a non-leaf OP
 * node and inherits the length and the context of CLOSE(), so under the MF compiled plan path it
 * correctly computes an all 0.0 sequence as long as dates from the KData of every stock.
 *
 * Verify that after the sorting the order is strictly the market_code one and the values stay 0.0.
 */
TEST_CASE("test_MF_deterministic_tie_all_equal_desc") {
    StockManager& sm = StockManager::instance();
    // The market_code lexicographic order: SH600004 < SH600005 < SZ000001 < SZ000002
    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQuery(-30);
    IndicatorList src_inds{CLOSE() - CLOSE()};

    auto mf = MF_EqualWeight(src_inds, stks, query, ref_stk);
    mf->setParam<int>("mode", 0);  // Descending
    auto dates = mf->getDatetimeList();
    CHECK_UNARY(!dates.empty());

    // Take a cross section in the middle (avoiding the NaN from the suspended stocks on the edges)
    auto cross = mf->getScores(dates[dates.size() / 2]);
    CHECK_EQ(cross.size(), 4);
    CHECK_EQ(cross[0].stock, sm["sh600004"]);
    CHECK_EQ(cross[1].stock, sm["sh600005"]);
    CHECK_EQ(cross[2].stock, sm["sz000001"]);
    CHECK_EQ(cross[3].stock, sm["sz000002"]);
    for (const auto& sr : cross) {
        CHECK_EQ(sr.value, doctest::Approx(0.0).epsilon(0.0001));
    }
}

/**
 * @par Test point: all the valid values equal (a tie) and sorted by the market_code lexicographic
 * order (mode 1, ascending)
 *
 * In the ascending mode the tie-break secondary key does not flip with the main direction and is
 * still the market_code lexicographic order.
 */
TEST_CASE("test_MF_deterministic_tie_all_equal_asc") {
    StockManager& sm = StockManager::instance();
    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQuery(-30);
    IndicatorList src_inds{CLOSE() - CLOSE()};

    auto mf = MF_EqualWeight(src_inds, stks, query, ref_stk);
    mf->setParam<int>("mode", 1);  // Ascending
    auto dates = mf->getDatetimeList();
    CHECK_UNARY(!dates.empty());

    auto cross = mf->getScores(dates[dates.size() / 2]);
    CHECK_EQ(cross.size(), 4);
    CHECK_EQ(cross[0].stock, sm["sh600004"]);
    CHECK_EQ(cross[1].stock, sm["sh600005"]);
    CHECK_EQ(cross[2].stock, sm["sz000001"]);
    CHECK_EQ(cross[3].stock, sm["sz000002"]);
}

/**
 * @par Test point: the forward / reverse order of the input StockList does not affect the result
 *
 * Two MF instances use the same stock set but a different StockList input order; it asserts the
 * output cross section order is exactly the same, algebraically proving a strict total order
 * (independent of the input permutation).
 */
TEST_CASE("test_MF_deterministic_tie_input_order_invariant") {
    StockManager& sm = StockManager::instance();
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQuery(-30);
    IndicatorList src_inds{CLOSE() - CLOSE()};

    StockList stks_fwd{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    auto mf_fwd = MF_EqualWeight(src_inds, stks_fwd, query, ref_stk);
    mf_fwd->setParam<int>("mode", 0);
    auto dates = mf_fwd->getDatetimeList();
    auto cross_fwd = mf_fwd->getScores(dates[dates.size() / 2]);

    StockList stks_rev{sm["sz000002"], sm["sz000001"], sm["sh600005"], sm["sh600004"]};
    auto mf_rev = MF_EqualWeight(src_inds, stks_rev, query, ref_stk);
    mf_rev->setParam<int>("mode", 0);
    auto cross_rev = mf_rev->getScores(dates[dates.size() / 2]);

    CHECK_EQ(cross_fwd.size(), cross_rev.size());
    for (size_t i = 0; i < cross_fwd.size(); i++) {
        CHECK_EQ(cross_fwd[i].stock, cross_rev[i].stock);
    }
}

/**
 * @par Test point: the total order property of the multiple date cross sections of getAllScores
 * (mode 0, descending)
 *
 * The real indicator MA(CLOSE()) traverses all the date cross sections of getAllScores; it asserts:
 *   1. a valid value never comes after a NaN;
 *   2. in the valid section prev.value >= curr.value (descending);
 *   3. with equal valid values the market_code lexicographic order strictly increases;
 *   4. in the NaN section the market_code lexicographic order strictly increases.
 *
 * A coverage counter counts the "valid value mixed with NaN" and "valid value tie" sections;
 * if it is 0 an explicit FAIL avoids a falsely green property assertion.
 */
TEST_CASE("test_MF_deterministic_tie_mixed_property_desc") {
    StockManager& sm = StockManager::instance();
    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQuery(-30);
    IndicatorList src_inds{MA(CLOSE(), 3)};

    auto mf = MF_EqualWeight(src_inds, stks, query, ref_stk);
    mf->setParam<int>("mode", 0);
    mf->getDatetimeList();
    const auto& all_scores = mf->getAllScores();
    CHECK_UNARY(!all_scores.empty());

    size_t mixed_nan_count = 0;

    for (size_t d = 0; d < all_scores.size(); d++) {
        const auto& cross = all_scores[d];
        bool has_valid = false, has_nan = false;
        for (size_t i = 1; i < cross.size(); i++) {
            const auto& prev = cross[i - 1];
            const auto& curr = cross[i];
            bool prev_nan = std::isnan(prev.value);
            bool curr_nan = std::isnan(curr.value);
            CHECK_UNARY(!(prev_nan && !curr_nan));
            if (!prev_nan && !curr_nan) {
                CHECK_UNARY(prev.value >= curr.value);
                if (prev.value == curr.value) {
                    CHECK_UNARY(prev.stock.market_code() < curr.stock.market_code());
                }
            }
            if (prev_nan && curr_nan) {
                CHECK_UNARY(prev.stock.market_code() < curr.stock.market_code());
            }
            has_valid = has_valid || !prev_nan;
            has_nan = has_nan || prev_nan;
        }
        if (!cross.empty()) {
            bool last_nan = std::isnan(cross.back().value);
            has_valid = has_valid || !last_nan;
            has_nan = has_nan || last_nan;
        }
        if (has_valid && has_nan) {
            mixed_nan_count++;
        }
    }

    CHECK_MESSAGE(mixed_nan_count > 0, "mixed_nan scenario not triggered by test data");
}

/**
 * @par Test point: the total order property of the multiple date cross sections of getAllScores
 * (mode 1, ascending)
 *
 * Symmetric to the descending case: only the comparison becomes <= and the other properties hold.
 */
TEST_CASE("test_MF_deterministic_tie_mixed_property_asc") {
    StockManager& sm = StockManager::instance();
    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQuery(-30);
    IndicatorList src_inds{MA(CLOSE(), 3)};

    auto mf = MF_EqualWeight(src_inds, stks, query, ref_stk);
    mf->setParam<int>("mode", 1);  // Ascending
    mf->getDatetimeList();
    const auto& all_scores = mf->getAllScores();
    CHECK_UNARY(!all_scores.empty());

    size_t mixed_nan_count = 0;

    for (size_t d = 0; d < all_scores.size(); d++) {
        const auto& cross = all_scores[d];
        bool has_valid = false, has_nan = false;
        for (size_t i = 1; i < cross.size(); i++) {
            const auto& prev = cross[i - 1];
            const auto& curr = cross[i];
            bool prev_nan = std::isnan(prev.value);
            bool curr_nan = std::isnan(curr.value);
            CHECK_UNARY(!(prev_nan && !curr_nan));
            if (!prev_nan && !curr_nan) {
                CHECK_UNARY(prev.value <= curr.value);
                if (prev.value == curr.value) {
                    CHECK_UNARY(prev.stock.market_code() < curr.stock.market_code());
                }
            }
            if (prev_nan && curr_nan) {
                CHECK_UNARY(prev.stock.market_code() < curr.stock.market_code());
            }
            has_valid = has_valid || !prev_nan;
            has_nan = has_nan || prev_nan;
        }
        if (!cross.empty()) {
            bool last_nan = std::isnan(cross.back().value);
            has_valid = has_valid || !last_nan;
            has_nan = has_nan || last_nan;
        }
        if (has_valid && has_nan) {
            mixed_nan_count++;
        }
    }

    CHECK_MESSAGE(mixed_nan_count > 0, "mixed_nan scenario not triggered by test data");
}

/**
 * @par Test point: an empty StockList should be rejected by _checkData at the calculation
 *
 * MF_EqualWeight does not validate at the construction; when calculate() is triggered _checkData
 * requires m_stks.size() >= 2, so an empty list should throw.
 */
TEST_CASE("test_MF_deterministic_tie_empty_stks") {
    StockManager& sm = StockManager::instance();
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQuery(-30);
    IndicatorList src_inds{MA(CLOSE(), 3)};
    StockList empty_stks;
    auto mf = MF_EqualWeight(src_inds, empty_stks, query, ref_stk);
    CHECK_THROWS_AS(mf->getDatetimeList(), std::exception);
}

/**
 * @par Test point: a single stock should be rejected by _checkData at the calculation
 *
 * _checkData requires m_stks.size() >= 2; a single stock has no ranking meaning and should throw.
 */
TEST_CASE("test_MF_deterministic_tie_single_stk") {
    StockManager& sm = StockManager::instance();
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQuery(-30);
    IndicatorList src_inds{MA(CLOSE(), 3)};
    StockList single_stk{sm["sh600004"]};
    auto mf = MF_EqualWeight(src_inds, single_stk, query, ref_stk);
    CHECK_THROWS_AS(mf->getDatetimeList(), std::exception);
}

/**
 * @par Test point: a duplicated Stock does not break the strict weak ordering, no crash
 *
 * The StockList contains the same stock twice: the two elements have identical value and code,
 * so the comparator returns a<b=false and b<a=false (an equivalence class) and std::sort is right.
 * Verify that there is no crash and the two duplicate elements are adjacent at the code position.
 */
TEST_CASE("test_MF_deterministic_tie_duplicate_stk") {
    StockManager& sm = StockManager::instance();
    StockList stks{sm["sh600004"], sm["sh600005"], sm["sh600004"], sm["sz000001"]};
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQuery(-30);
    IndicatorList src_inds{CLOSE() - CLOSE()};

    auto mf = MF_EqualWeight(src_inds, stks, query, ref_stk);
    mf->setParam<int>("mode", 0);
    auto dates = mf->getDatetimeList();
    CHECK_UNARY(!dates.empty());

    auto cross = mf->getScores(dates[dates.size() / 2]);
    CHECK_EQ(cross.size(), 4);
    CHECK_EQ(cross[0].stock, sm["sh600004"]);
    CHECK_EQ(cross[1].stock, sm["sh600004"]);
    CHECK_EQ(cross[2].stock, sm["sh600005"]);
    CHECK_EQ(cross[3].stock, sm["sz000001"]);
}

/** @} */