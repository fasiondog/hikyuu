/*
 * test_MM_FixedCountTps.cpp
 *
 *  Created on: 2013-4-19
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <limits>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_manage/crt/TC_FixedA.h>
#include <hikyuu/trade_manage/crt/TC_FixedAPre2015.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedCountTps.h>

using namespace hku;

/**
 * @defgroup test_MM_FixedCountTps test_MM_FixedCountTps
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_MM_FixedCountTpsTps") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeManagerPtr tm = crtTM(Datetime(199001010000LL), 0.0, TC_FixedAPre2015());

    /** @arg The buy / sell quantity contains a value less than 0 */
    CHECK_THROWS_AS(MM_FixedCountTps({100., -1}, {100., 100.}), std::exception);
    CHECK_THROWS_AS(MM_FixedCountTps({100., 100.}, {100., -100.}), std::exception);

    /** @arg The total buy and sell quantities differ (a warning is printed only) */
    auto _ = MM_FixedCountTps({100., 100.}, {200., 200.});

    /** @arg n = {100, 200}; the query deposits nothing and the tier sizes ignore the cash; the
     * account is funded explicitly for the tier-flow assertions */
    tm = crtTM(Datetime(199001010000LL), 0.0, TC_FixedAPre2015());
    CHECK_EQ(tm->initCash(), 0.0);
    auto mm = MM_FixedCountTps({100., 200.}, {200., 100.});
    mm->setTM(tm);
    mm->setParam<bool>("auto_checkin", true);
    CHECK_EQ(mm->currentBuyCount(stock), 0);
    double buy_num = mm->getBuyNumber(Datetime(200001200000), stock, 24.11, 24.11, PART_SIGNAL);
    CHECK_EQ(buy_num, 100);
    CHECK_EQ(tm->cash(Datetime(200001200000)), 0.0);
    tm->checkin(Datetime(200001200000), 1000000.0);
    auto tr = tm->buy(Datetime(200001200000), stock, 24.11, 100, 0, 0, 24.11, PART_SIGNAL);
    mm->buyNotify(tr);
    CHECK_EQ(mm->currentBuyCount(stock), 1);

    buy_num = mm->getBuyNumber(Datetime(200001200000), stock, 24.11, 24.11, PART_SIGNAL);
    CHECK_EQ(buy_num, 200);
    tr = tm->buy(Datetime(200001200000), stock, 24.11, 100, 0, 0, 24.11, PART_SIGNAL);
    mm->buyNotify(tr);
    CHECK_EQ(mm->currentBuyCount(stock), 2);

    buy_num = mm->getBuyNumber(Datetime(200001200000), stock, 24.11, 24.11, PART_SIGNAL);
    CHECK_EQ(buy_num, 0);

    double sell_num = mm->getSellNumber(Datetime(200001210000), stock, 24.11, 24.11, PART_SIGNAL);
    CHECK_EQ(sell_num, 200);
    tr = tm->sell(Datetime(200001200000), stock, 24.11, 100, 0, 0, 24.11, PART_SIGNAL);
    mm->sellNotify(tr);
    CHECK_EQ(mm->currentBuyCount(stock), 0);
    CHECK_EQ(mm->currentSellCount(stock), 1);

    sell_num = mm->getSellNumber(Datetime(200001210000), stock, 24.11, 24.11, PART_SIGNAL);
    CHECK_EQ(sell_num, 100);
    tr = tm->sell(Datetime(200001200000), stock, 24.11, 100, 0, 0, 24.11, PART_SIGNAL);
    mm->sellNotify(tr);
    CHECK_EQ(mm->currentBuyCount(stock), 0);
    CHECK_EQ(mm->currentSellCount(stock), 2);

    buy_num = mm->getBuyNumber(Datetime(200001202000), stock, 24.11, 24.11, PART_SIGNAL);
    CHECK_EQ(buy_num, 100);
    tr = tm->buy(Datetime(200001202000), stock, 24.11, 100, 0, 0, 24.11, PART_SIGNAL);
    mm->buyNotify(tr);
    CHECK_EQ(mm->currentBuyCount(stock), 1);
    CHECK_EQ(mm->currentSellCount(stock), 0);
}

/** @par Test points */
TEST_CASE("test_MM_FixedCountTps_mult_sell_guard") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");

    /** @arg risk <= 0 liquidates the whole position (returns MAX_DOUBLE) */
    auto mm = MM_FixedCountTps({100., 200.}, {200., 100.});
    mm->setTM(crtTM());
    CHECK_UNARY(mm->getSellNumber(Datetime(200001210000), stock, 24.11, 0.0, PART_SIGNAL) ==
                MAX_DOUBLE);
    CHECK_UNARY(mm->getSellNumber(Datetime(200001210000), stock, 24.11, -1.0, PART_SIGNAL) ==
                MAX_DOUBLE);

    /** @arg tier exhausted also returns MAX_DOUBLE to avoid orphan shares */
    auto mm1 = MM_FixedCountTps({100.}, {100.});
    auto tm1 = crtTM(Datetime(199001010000LL), 1000000.0);
    mm1->setTM(tm1);
    auto buy_tr = tm1->buy(Datetime(200001200000), stock, 24.11, 100, 0, 0, 24.11, PART_SIGNAL);
    mm1->buyNotify(buy_tr);
    auto sell_tr = tm1->sell(Datetime(200001210000), stock, 24.11, 100, 0, 0, 24.11, PART_SIGNAL);
    mm1->sellNotify(sell_tr);  // Advances sell count to 1, exhausting sell_counts.size()==1
    CHECK_UNARY(mm1->getSellNumber(Datetime(200001210000), stock, 24.11, 24.11, PART_SIGNAL) ==
                MAX_DOUBLE);

    /** @arg tier == holdNumber returns tier (strict `>` in the clamp, not `>=`) */
    auto mm2 = MM_FixedCountTps({100.}, {100.});
    auto tm2 = crtTM(Datetime(199001010000LL), 1000000.0);
    mm2->setTM(tm2);
    auto buy_tr2 = tm2->buy(Datetime(200001200000), stock, 24.11, 100, 0, 0, 24.11, PART_SIGNAL);
    mm2->buyNotify(buy_tr2);
    CHECK_EQ(mm2->getSellNumber(Datetime(200001210000), stock, 24.11, 24.11, PART_SIGNAL), 100.0);

    /** @arg Advancing through every sell tier to exhaustion still returns MAX_DOUBLE on the next
     *  signal (multi-tier counterpart of the single-tier case above) */
    auto mm3 = MM_FixedCountTps({100., 200.}, {100., 200.});
    auto tm3 = crtTM(Datetime(199001010000LL), 1000000.0);
    mm3->setTM(tm3);
    auto b1 = tm3->buy(Datetime(200001200000), stock, 24.11, 100, 0, 0, 24.11, PART_SIGNAL);
    mm3->buyNotify(b1);
    auto b2 = tm3->buy(Datetime(200001200000), stock, 24.11, 200, 0, 0, 24.11, PART_SIGNAL);
    mm3->buyNotify(b2);
    auto s1 = tm3->sell(Datetime(200001210000), stock, 24.11, 100, 0, 0, 24.11, PART_SIGNAL);
    mm3->sellNotify(s1);
    auto s2 = tm3->sell(Datetime(200001210000), stock, 24.11, 200, 0, 0, 24.11, PART_SIGNAL);
    mm3->sellNotify(s2);  // Advances sell count to 2, exhausting sell_counts.size()==2
    CHECK_UNARY(mm3->getSellNumber(Datetime(200001220000), stock, 24.11, 24.11, PART_SIGNAL) ==
                MAX_DOUBLE);
}

/** @par Test points */
TEST_CASE("test_MM_FixedCountTps_short_trade_immunity") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    auto tm = crtTM(Datetime(199001010000LL), 100000.0, TC_FixedAPre2015());
    tm->setParam<bool>("support_borrow_stock", true);
    auto mm = MM_FixedCountTps({100., 200.}, {200., 100.});
    mm->setTM(tm);

    /** @arg The short-side trades do not pollute the long tranche counters (ISS-093) */
    auto tr = tm->buy(Datetime(200001200000), stock, 24.11, 100, 0, 0, 24.11, PART_SIGNAL);
    mm->buyNotify(tr);
    tr = tm->buy(Datetime(200001200000), stock, 24.11, 100, 0, 0, 24.11, PART_SIGNAL);
    mm->buyNotify(tr);
    CHECK_EQ(mm->currentBuyCount(stock), 2);
    CHECK_EQ(mm->currentSellCount(stock), 0);

    /** @arg Opening a short neither resets the long buy counter nor counts as a long sell */
    tr = tm->sellShort(Datetime(200001210000), stock, 24.11, 100, 0, 0, 24.11, PART_SIGNAL);
    CHECK_UNARY(!tr.isNull());
    mm->sellNotify(tr);
    CHECK_EQ(mm->currentBuyCount(stock), 2);
    CHECK_EQ(mm->currentSellCount(stock), 0);

    /** @arg Covering the short neither advances the long buy counter nor counts as a long buy */
    tr = tm->buyShort(Datetime(200001220000), stock, 24.11, 100, 0, 0, 24.11, PART_SIGNAL);
    CHECK_UNARY(!tr.isNull());
    mm->buyNotify(tr);
    CHECK_EQ(mm->currentBuyCount(stock), 2);
    CHECK_EQ(mm->currentSellCount(stock), 0);

    /** @arg The long-side flows keep driving the tranches normally after the short-side trades */
    tr = tm->sell(Datetime(200001230000), stock, 24.11, 100, 0, 0, 24.11, PART_SIGNAL);
    CHECK_UNARY(!tr.isNull());
    mm->sellNotify(tr);
    CHECK_EQ(mm->currentBuyCount(stock), 0);
    CHECK_EQ(mm->currentSellCount(stock), 1);
}

/** @par Test points */
TEST_CASE("test_MM_FixedCountTps_param_check") {
    /** @arg A negative tier is rejected */
    CHECK_THROWS_AS(MM_FixedCountTps({100., -1}, {100., 100.}), std::exception);
    CHECK_THROWS_AS(MM_FixedCountTps({100., 100.}, {100., -100.}), std::exception);

    /** @arg A zero tier is rejected: it would stall the tranche counter forever */
    CHECK_THROWS_AS(MM_FixedCountTps({0., 100.}, {100., 100.}), std::exception);
    CHECK_THROWS_AS(MM_FixedCountTps({100., 100.}, {0., 100.}), std::exception);
    CHECK_THROWS_AS(MM_FixedCountTps({100., 0.}, {100., 100.}), std::exception);
    CHECK_THROWS_AS(MM_FixedCountTps({100., 100.}, {0., 200.}), std::exception);

    /** @arg A non-finite tier (NaN / +-Inf) is rejected before it can poison the order */
    CHECK_THROWS_AS(MM_FixedCountTps({std::numeric_limits<double>::quiet_NaN()}, {100.}),
                    std::exception);
    CHECK_THROWS_AS(MM_FixedCountTps({100.}, {std::numeric_limits<double>::quiet_NaN()}),
                    std::exception);
    CHECK_THROWS_AS(MM_FixedCountTps({std::numeric_limits<double>::infinity()}, {100.}),
                    std::exception);
    CHECK_THROWS_AS(MM_FixedCountTps({100.}, {-std::numeric_limits<double>::infinity()}),
                    std::exception);

    /** @arg An empty buy list is rejected: it would never open a position */
    CHECK_THROWS_AS(MM_FixedCountTps({}, {100.}), std::exception);

    /** @arg Finite tiers whose sum overflows to +inf are rejected before the comparison NaNs out */
    CHECK_THROWS_AS(MM_FixedCountTps({1e308, 1e308}, {1e308, 1e308}), std::exception);
    CHECK_THROWS_AS(MM_FixedCountTps({1e308}, {1e308, 1e308}), std::exception);

    /** @arg The total comparison uses a tolerance, so a reordered sum of the same tiers is fine */
    CHECK_NOTHROW(MM_FixedCountTps({0.1, 0.2, 0.3}, {0.3, 0.3}));

    /** @arg A genuinely inconsistent total only warns */
    CHECK_NOTHROW(MM_FixedCountTps({100., 100.}, {200., 200.}));
}

/** @par Test points */
TEST_CASE("test_MM_FixedCountTps_no_short_entry") {
    // The tranche counters are long-only (ISS-093), so opening a short stays forbidden
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");

    auto mm = MM_FixedCountTps({100., 200.}, {200., 100.});
    mm->setTM(crtTM(Datetime(199001010000LL), 1000000.0));
    CHECK_EQ(mm->getSellShortNumber(Datetime(200001200000), stock, 24.11, 10.0, PART_SIGNAL), 0.0);

    /** @arg the short cover liquidates the entire position */
    CHECK_UNARY((mm->getBuyShortNumber(Datetime(200001200000), stock, 24.11, 10.0, PART_SIGNAL) ==
                 MAX_DOUBLE));
}

/** @} */
