/*
 * test_Performance.cpp
 *
 *  Created on: 2026-10-08
 *      Author: fasiondog
 */

#include "doctest/doctest.h"

#include <algorithm>

#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/Performance.h>
#include <hikyuu/trade_manage/crt/TC_Zero.h>
#include <hikyuu/trade_manage/crt/crtTM.h>

using namespace hku;

/**
 * @defgroup test_Performance test_Performance
 * @ingroup test_hikyuu_trade_manage_suite
 * @{
 */

namespace {

// Independent brute-force day scan over the account position snapshots, asserting that the
// interval-merge based flat statistics equal it. Keeping the expectation data-driven avoids
// hardcoding day counts that depend on the ex-rights data of the test securities.
void assert_flat_matches_scan(const Performance& p, const TradeManagerPtr& tm,
                              const Datetime& eval_dt) {
    const PositionRecordList& his = tm->getHistoryPositionList();
    const PositionRecordList& cur = tm->getPositionList();
    DatetimeList days = getDateRange(tm->firstDatetime(), Datetime(eval_dt.date() + bd::days(1)));
    int64_t total = 0, streaks = 0, max_streak = 0, cur_streak = 0;
    for (const auto& day : days) {
        bool hold = false;
        for (const auto& pos : his) {
            if (pos.takeDatetime <= day && day < pos.cleanDatetime) {
                hold = true;
                break;
            }
        }
        if (!hold) {
            for (const auto& pos : cur) {
                if (pos.takeDatetime <= day) {
                    hold = true;
                    break;
                }
            }
        }
        if (hold) {
            cur_streak = 0;
        } else {
            total++;
            if (cur_streak == 0) {
                streaks++;
            }
            cur_streak++;
            max_streak = std::max(max_streak, cur_streak);
        }
    }
    CHECK_EQ(p.get("Total Time Flat"), (double)total);
    CHECK_EQ(p.get("Max Time Flat"), (double)max_streak);
    CHECK_EQ(p.get("Avg Time Flat"), streaks != 0 ? (double)total / (double)streaks : 0.0);
    if (!days.empty()) {
        CHECK(p.get("Time Flat / Total Time %") ==
              doctest::Approx(100.0 * total / (double)days.size()).epsilon(0.001));
    }
}

}  // namespace

/** @par Test points */
TEST_CASE("test_Performance_basic_keys") {
    Performance p;

    /** @arg The built-in english keys and legacy chinese keys both exist */
    CHECK_UNARY(p.exist("Account Initial Capital"));
    CHECK_UNARY(p.exist("帐户初始金额"));
    CHECK_UNARY(!p.exist("Not Exist Key"));

    /** @arg All the built-in keys are initialized to 0 and keep the names/values order */
    StringList names = p.names();
    PriceList values = p.values();
    CHECK_EQ(names.size(), values.size());
    CHECK_EQ(names.size(), 53);
    for (size_t i = 0; i < names.size(); ++i) {
        CHECK_EQ(values[i], 0.0);
    }

    /** @arg get a not exist key returns Null */
    CHECK_UNARY(std::isnan(p.get("Not Exist Key")));

    /** @arg reset clears the values but keeps the NaN ones */
    p.setValue("Account Initial Capital", 100.0);
    p.reset();
    CHECK_EQ(p.get("Account Initial Capital"), 0.0);
}

/** @par Test points */
TEST_CASE("test_Performance_register_key") {
    /** @arg addKey registers the key and the optional chinese alias */
    Performance p;
    p.addKey("My Test Key", "我的测试项");
    CHECK_UNARY(p.exist("My Test Key"));
    CHECK_UNARY(p.exist("我的测试项"));
    CHECK_EQ(p.names().back(), "My Test Key");
    p.setValue("My Test Key", 3.0);
    CHECK_EQ(p.get("My Test Key"), 3.0);
    CHECK_EQ(p.get("我的测试项"), 3.0);

    /** @arg An empty key is rejected */
    size_t old_size = p.names().size();
    p.addKey("");
    CHECK_EQ(p.names().size(), old_size);

    /** @arg A duplicated key is rejected, and values() never writes out of bounds */
    p.addKey("My Test Key");
    CHECK_EQ(p.names().size(), old_size);
    PriceList values = p.values();
    CHECK_EQ(values.size(), p.names().size());

    /** @arg A chinese alias already bound to a different english key is rejected without side
     * effects on the process-wide mapping */
    p.addKey("Another Key", "我的测试项");
    CHECK_EQ(p.names().size(), old_size);
    CHECK_UNARY(!p.exist("Another Key"));

    /** @arg A legacy chinese key registered by another instance: get on this instance returns
     * Null, exist returns false (consistent with get), and setValue on the unregistered key is
     * rejected without creating a new slot */
    Performance q;
    CHECK_UNARY(std::isnan(q.get("我的测试项")));
    CHECK_UNARY(!q.exist("我的测试项"));
    size_t q_size = q.names().size();
    q.setValue("My Test Key", 9.0);
    CHECK_UNARY(std::isnan(q.get("My Test Key")));
    CHECK_EQ(q.names().size(), q_size);
}

/** @par Test points */
TEST_CASE("test_Performance_statistics_closed_trades") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeManagerPtr tm = crtTM(Datetime(199901010000LL), 1000000, TC_Zero(), "TEST");

    // W(17->18), W(18->19), W(19->20), L(20->22), W(23->24): the historical max winning streak
    // is 3, and the last streak of 1 must not overwrite it
    tm->buy(Datetime(199911170000LL), stock, 10.0, 100);
    tm->sell(Datetime(199911180000LL), stock, 11.0, MAX_DOUBLE);
    tm->buy(Datetime(199911180000LL), stock, 10.0, 100);
    tm->sell(Datetime(199911190000LL), stock, 11.0, MAX_DOUBLE);
    tm->buy(Datetime(199911190000LL), stock, 10.0, 100);
    tm->sell(Datetime(199911200000LL), stock, 11.0, MAX_DOUBLE);
    tm->buy(Datetime(199911200000LL), stock, 10.0, 100);
    tm->sell(Datetime(199911220000LL), stock, 9.0, MAX_DOUBLE);
    tm->buy(Datetime(199911230000LL), stock, 10.0, 100);
    tm->sell(Datetime(199911240000LL), stock, 11.0, MAX_DOUBLE);

    Performance p;
    p.statistics(tm, Datetime(199911240000LL));

    /** @arg The account base fields */
    CHECK_EQ(p.get("Account Initial Capital"), 1000000.0);
    CHECK_EQ(p.get("Total Closed Trades"), 5);
    CHECK_EQ(p.get("Total Net Profit of Closed Trades"), 300.0);
    CHECK_EQ(p.get("Total Profit of Winning Trades"), 400.0);
    CHECK_EQ(p.get("Total Loss of Losing Trades"), -100.0);

    /** @arg Win/loss counts and ratios */
    CHECK_EQ(p.get("Number of Winning Trades"), 4);
    CHECK_EQ(p.get("Number of Losing Trades"), 1);
    CHECK_EQ(p.get("Win Rate %"), 100.0 * 4 / 5);
    CHECK_EQ(p.get("Largest Single Win"), 100.0);
    CHECK_EQ(p.get("Largest Single Loss"), -100.0);
    CHECK_EQ(p.get("Avg Profit per Winning Trade"), 100.0);
    CHECK_EQ(p.get("Avg Loss per Losing Trade"), -100.0);
    CHECK_EQ(p.get("Avg Win / Avg Loss Ratio"), 1.0);
    CHECK_EQ(p.get("Profit Factor"), 4.0);

    /** @arg Max consecutive wins: the historical maximum 3 (not the last streak of 1), and the
     * amount of the longest winning streak is 300 */
    CHECK_EQ(p.get("Max Consecutive Wins"), 3);
    CHECK_EQ(p.get("Max Consecutive Win Amount"), 300.0);

    /** @arg Max consecutive losses stays at 1 with the amount of -100 */
    CHECK_EQ(p.get("Max Consecutive Losses"), 1);
    CHECK_EQ(p.get("Max Consecutive Loss Amount"), -100.0);

    /** @arg The R multiple uses the actual risk ((realPrice - stoploss) * number * unit):
     * stoploss defaults to 0, so the risk is 10*100*1 = 1000, giving 0.1 per trade (0.06 on
     * average over 5 trades) */
    CHECK_EQ(p.get("R-Multiple Expectancy"), 0.06);

    /** @arg Cash usage percent lies in (0, 1] */
    CHECK_UNARY(p.get("Max Cash Usage per Trade %") > 0.0);
    CHECK_UNARY(p.get("Max Cash Usage per Trade %") <= 100.0);
    CHECK_UNARY(p.get("Avg Cash Usage per Trade %") <= p.get("Max Cash Usage per Trade %"));
}

/** @par Test points */
TEST_CASE("test_Performance_statistics_streak_amount_tie") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeManagerPtr tm = crtTM(Datetime(199901010000LL), 1000000, TC_Zero(), "TEST");

    // Closing order L(-100), L(-100), W(+100), L(-50), L(-50): two equal-length (2) losing
    // streaks, the first summing to -200 and the second to -100. "Max Consecutive Loss Amount"
    // must report the larger loss (-200), not the last/smaller one
    tm->buy(Datetime(199911170000LL), stock, 10.0, 100);
    tm->sell(Datetime(199911180000LL), stock, 9.0, MAX_DOUBLE);
    tm->buy(Datetime(199911180000LL), stock, 10.0, 100);
    tm->sell(Datetime(199911190000LL), stock, 9.0, MAX_DOUBLE);
    tm->buy(Datetime(199911190000LL), stock, 10.0, 100);
    tm->sell(Datetime(199911200000LL), stock, 11.0, MAX_DOUBLE);
    tm->buy(Datetime(199911200000LL), stock, 10.0, 100);
    tm->sell(Datetime(199911210000LL), stock, 9.5, MAX_DOUBLE);
    tm->buy(Datetime(199911210000LL), stock, 10.0, 100);
    tm->sell(Datetime(199911220000LL), stock, 9.5, MAX_DOUBLE);

    Performance p;
    p.statistics(tm, Datetime(199911220000LL));

    /** @arg All 5 trades are closed in order (guards against silently rejected out-of-order
     * orders that would make the following assertions vacuous) */
    CHECK_EQ(p.get("Total Closed Trades"), 5);

    /** @arg The equal-length losing streaks resolve to the larger loss -200 (the first segment) */
    CHECK_EQ(p.get("Max Consecutive Losses"), 2);
    CHECK_EQ(p.get("Max Consecutive Loss Amount"), -200.0);

    /** @arg TM-204: the max consecutive loss R multiple is the mean over the selected segment
     * only (-0.1, -0.1 -> -0.1), not accumulated across segments */
    CHECK_EQ(p.get("Max Consecutive Loss R-Multiple"), -0.1);

    /** @arg The single winning trade is the max winning streak of 1 with the amount of +100 */
    CHECK_EQ(p.get("Max Consecutive Wins"), 1);
    CHECK_EQ(p.get("Max Consecutive Win Amount"), 100.0);
}

/** @par Test points */
TEST_CASE("test_Performance_statistics_streak_win_amount_tie") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeManagerPtr tm = crtTM(Datetime(199901010000LL), 1000000, TC_Zero(), "TEST");

    // Closing order W(+100), W(+100), L(-100), W(+100), W(+100): two equal-length (2) winning
    // streaks, the first summing to +200 and the second to +100. "Max Consecutive Win Amount"
    // must report the larger win (+200)
    tm->buy(Datetime(199911170000LL), stock, 10.0, 100);
    tm->sell(Datetime(199911180000LL), stock, 11.0, MAX_DOUBLE);
    tm->buy(Datetime(199911180000LL), stock, 10.0, 100);
    tm->sell(Datetime(199911190000LL), stock, 11.0, MAX_DOUBLE);
    tm->buy(Datetime(199911190000LL), stock, 10.0, 100);
    tm->sell(Datetime(199911200000LL), stock, 9.0, MAX_DOUBLE);
    tm->buy(Datetime(199911200000LL), stock, 10.0, 100);
    tm->sell(Datetime(199911210000LL), stock, 11.0, MAX_DOUBLE);
    tm->buy(Datetime(199911210000LL), stock, 10.0, 100);
    tm->sell(Datetime(199911220000LL), stock, 11.0, MAX_DOUBLE);

    Performance p;
    p.statistics(tm, Datetime(199911220000LL));

    /** @arg All 5 trades are closed in order (guards against silently rejected orders) */
    CHECK_EQ(p.get("Total Closed Trades"), 5);

    /** @arg The equal-length winning streaks resolve to the larger win +200 (the first segment) */
    CHECK_EQ(p.get("Max Consecutive Wins"), 2);
    CHECK_EQ(p.get("Max Consecutive Win Amount"), 200.0);

    /** @arg The max consecutive win R multiple is the mean over the selected segment only
     * (risk 1000, r 0.1 each -> 0.1), not accumulated across segments */
    CHECK_EQ(p.get("Max Consecutive Win R-Multiple"), 0.1);

    /** @arg The single losing trade is the max losing streak of 1 with the amount of -100 */
    CHECK_EQ(p.get("Max Consecutive Losses"), 1);
    CHECK_EQ(p.get("Max Consecutive Loss Amount"), -100.0);
}

/** @par Test points */
TEST_CASE("test_Performance_statistics_time_flat_with_open_position") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeManagerPtr tm = crtTM(Datetime(199901010000LL), 1000000, TC_Zero(), "TEST");

    /** @arg Buy once and never sell: every day is a holding day, so there is no flat time */
    tm->buy(Datetime(199911170000LL), stock, 10.0, 100);

    Performance p;
    p.statistics(tm, Datetime(199911180000LL));
    CHECK_EQ(p.get("Total Time Flat"), 0.0);
    CHECK_EQ(p.get("Time Flat / Total Time %"), 0.0);
    CHECK_EQ(p.get("Max Time Flat"), 0.0);
}

/** @par Test points */
TEST_CASE("test_Performance_statistics_time_flat_after_close") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeManagerPtr tm = crtTM(Datetime(199901010000LL), 1000000, TC_Zero(), "TEST");

    // Held only on Nov 17; flat for 2 days (Nov 18, 19) until the statistics moment Nov 19
    tm->buy(Datetime(199911170000LL), stock, 10.0, 100);
    tm->sell(Datetime(199911180000LL), stock, 11.0, MAX_DOUBLE);

    Performance p;
    p.statistics(tm, Datetime(199911190000LL));

    /** @arg The flat days cover the whole range except the holding day */
    CHECK_EQ(p.get("Total Time Flat"), 2.0);

    /** @arg A single flat streak: the max and the average both equal the total (the first day of
     * a streak must be counted), and no off-by-one contradiction between them */
    CHECK_EQ(p.get("Max Time Flat"), 2.0);
    CHECK_EQ(p.get("Avg Time Flat"), 2.0);

    /** @arg Time Flat % keeps the fraction instead of integer truncation */
    int64_t day_count = (Datetime(199911200000LL) - Datetime(199911170000LL)).days();
    CHECK_EQ(p.get("Time Flat / Total Time %"),
             doctest::Approx(100.0 * 2 / day_count).epsilon(0.001));
}

/** @par Test points */
TEST_CASE("test_Performance_statistics_time_flat_streaks") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    Stock stock2 = sm.getStock("sh600004");
    Stock stock3 = sm.getStock("sh600008");
    TradeManagerPtr tm = crtTM(Datetime(199901010000LL), 1000000, TC_Zero(), "TEST");

    // Three disjoint holding blocks -> two separate flat gaps (multi-streak path)
    tm->buy(Datetime(199911170000LL), stock, 10.0, 100);
    tm->sell(Datetime(199911180000LL), stock, 11.0, MAX_DOUBLE);
    tm->buy(Datetime(199911190000LL), stock2, 10.0, 100);
    tm->sell(Datetime(199911210000LL), stock2, 11.0, MAX_DOUBLE);
    tm->buy(Datetime(199911220000LL), stock3, 10.0, 100);

    Performance p;
    p.statistics(tm, Datetime(199911220000LL));
    CHECK_EQ(p.get("Total Closed Trades"), 2);
    assert_flat_matches_scan(p, tm, Datetime(199911220000LL));
}

/** @par Test points */
TEST_CASE("test_Performance_statistics_time_flat_merge_overlap") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    Stock stock2 = sm.getStock("sh600004");
    Stock stock3 = sm.getStock("sh600008");
    TradeManagerPtr tm = crtTM(Datetime(199901010000LL), 1000000, TC_Zero(), "TEST");

    // stock [17,19) and stock2 [18,20) overlap -> merged into one block; stock3 open from 22
    // leaves a trailing gap in between, covering the interval-merge (overlap) path
    tm->buy(Datetime(199911170000LL), stock, 10.0, 100);
    tm->buy(Datetime(199911180000LL), stock2, 10.0, 100);
    tm->sell(Datetime(199911190000LL), stock, 11.0, MAX_DOUBLE);
    tm->sell(Datetime(199911200000LL), stock2, 11.0, MAX_DOUBLE);
    tm->buy(Datetime(199911220000LL), stock3, 10.0, 100);

    Performance p;
    p.statistics(tm, Datetime(199911220000LL));
    CHECK_EQ(p.get("Total Closed Trades"), 2);
    assert_flat_matches_scan(p, tm, Datetime(199911220000LL));
}

/** @par Test points */
TEST_CASE("test_Performance_report") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeManagerPtr tm = crtTM(Datetime(199901010000LL), 1000000, TC_Zero(), "TEST");
    tm->buy(Datetime(199911170000LL), stock, 10.0, 100);
    tm->sell(Datetime(199911180000LL), stock, 11.0, MAX_DOUBLE);

    /** @arg report outputs one line per registered key and is not empty */
    Performance p;
    p.statistics(tm, Datetime(199911180000LL));
    string buf = p.report();
    CHECK_UNARY(!buf.empty());
}

/** @} */
