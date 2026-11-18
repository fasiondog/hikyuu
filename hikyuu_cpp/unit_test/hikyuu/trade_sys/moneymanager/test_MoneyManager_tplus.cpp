/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-11-17
 *      Author: fasiondog
 *
 *  Tests for the MoneyManagerBase "t_plus_n" rule (the shares bought within the last n calendar
 *  days cannot be sold), driven on the real minute bars of the test database. The t+0 / t+1 / t+2
 *  combinations are covered on the minute KData and on the daily KData separately (the rule is
 *  skipped for a bar period of a day or above).
 */

#include "../../test_config.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/trade_manage/tradecost/crt/TC_Zero.h>
#include <hikyuu/trade_manage/crtTM.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include <hikyuu/trade_sys/signal/crt/SG_Cross.h>
#include <hikyuu/trade_sys/signal/crt/SG_Manual.h>
#include <hikyuu/trade_sys/moneymanager/MoneyManagerBase.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedCount.h>

using namespace hku;

/**
 * @defgroup test_MoneyManager_tplus test_MoneyManager_tplus
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

namespace {

// Three trading days of the real minute bars (sh600000 MIN5, 2000-01-04 / 05 / 06); the T+n rule
// applies to the sub-daily queries only, so the KType of the query matters
KData tplusMinuteKData() {
    Stock stock = getStock("sh600000");
    REQUIRE(!stock.isNull());
    KData kdata =
      stock.getKData(KQueryByDate(Datetime(20000104), Datetime(20000107), KQuery::MIN5));
    REQUIRE(kdata.size() > 10);
    return kdata;
}

// The index of the first bar of the given trading day (0-based)
size_t dayStart(const KData& kdata, size_t day) {
    size_t idx = Null<size_t>();
    size_t count = 0;
    Datetime prev;
    for (size_t i = 0; i < kdata.size(); ++i) {
        Datetime d = kdata[i].datetime.startOfDay();
        if (d != prev) {
            if (count == day) {
                idx = i;
                break;
            }
            ++count;
            prev = d;
        }
    }
    REQUIRE(idx != Null<size_t>());
    return idx;
}

// A synthetic minute fixture spanning a weekend: Fri 2000-01-07, Mon 2000-01-10, Tue 2000-01-11
KData tplusWeekendKData() {
    Stock stock("TEST", "TPLUSW", "T plus n weekend test");
    stock.setKRecordList({KRecord(Datetime(200001070935), 10, 10.5, 9.5, 10, 100, 100),
                          KRecord(Datetime(200001070940), 10, 10.5, 9.5, 10, 100, 100),
                          KRecord(Datetime(200001100935), 11, 11.5, 10.5, 11, 100, 100),
                          KRecord(Datetime(200001100940), 11, 11.5, 10.5, 11, 100, 100),
                          KRecord(Datetime(200001110935), 12, 12.5, 11.5, 12, 100, 100),
                          KRecord(Datetime(200001110940), 12, 12.5, 11.5, 12, 100, 100)},
                         KQuery::MIN5);
    return stock.getKData(KQuery(0, Null<int64_t>(), KQuery::MIN5));
}

SignalPtr manualSG(const std::vector<std::pair<Datetime, double>>& signals) {
    auto sg = SG_Manual();
    sg->setParam<bool>("alternate", false);
    for (const auto& [datetime, value] : signals) {
        sg->_addSignal(datetime, value);
    }
    return sg;
}

// Always asks for the "sell everything" sentinel, so the sentinel resolution of the cap is
// covered
class SellAllMM : public MoneyManagerBase {
public:
    SellAllMM() : MoneyManagerBase("SellAllMM") {}

    MoneyManagerPtr _clone() override {
        return std::make_shared<SellAllMM>();
    }

    double _getBuyNumber(const Datetime&, const Stock&, price_t, price_t, SystemPart) override {
        return 100;
    }

    double _getSellNumber(const Datetime&, const Stock&, price_t, price_t, SystemPart) override {
        return MAX_DOUBLE;
    }
};

// Exposes the short-side quantities, to verify the T+n lock never touches them
class ShortProbeMM : public MoneyManagerBase {
public:
    ShortProbeMM() : MoneyManagerBase("ShortProbeMM") {}

    MoneyManagerPtr _clone() override {
        return std::make_shared<ShortProbeMM>();
    }

    double _getBuyNumber(const Datetime&, const Stock&, price_t, price_t, SystemPart) override {
        return 100;
    }

    double _getSellShortNumber(const Datetime&, const Stock&, price_t, price_t,
                               SystemPart) override {
        return 100;
    }

    double _getBuyShortNumber(const Datetime&, const Stock&, price_t, price_t,
                              SystemPart) override {
        return 100;
    }
};

const TradeRecord* findRecord(const TradeRecordList& trs, BUSINESS business) {
    for (const auto& tr : trs) {
        if (tr.business == business) {
            return &tr;
        }
    }
    return nullptr;
}

}  // namespace

/** @par Check point (minute KData, the same-day sell): t+0 sells, t+1 and t+2 both block the
 * shares bought today */
TEST_CASE("test_MM_tplus_minute_same_day_sell") {
    KData kdata = tplusMinuteKData();
    Stock stock = kdata.getStock();

    auto run_sys = [&](int t_plus_n) {
        auto tm = crtTM(Datetime(199001010000), 100000.0, TC_Zero());
        auto mm = MM_FixedCount(100);
        mm->setParam<int>("t_plus_n", t_plus_n);
        auto sys = SYS_Simple(tm, mm, EnvironmentPtr(), ConditionPtr(),
                              manualSG({{kdata[0].datetime, 1.0}, {kdata[1].datetime, -1.0}}));
        sys->setParam<bool>("buy_delay", false);
        sys->setParam<bool>("sell_delay", false);
        sys->run(kdata);
        return tm;
    };

    /** @arg t_plus_n=0: the same-day sell fills */
    auto tm0 = run_sys(0);
    CHECK_EQ(tm0->getHoldNumber(kdata.back().datetime, stock), 0.0);

    /** @arg t_plus_n=1: the same-day sell is capped to 0 and dropped */
    auto tm1 = run_sys(1);
    CHECK_EQ(tm1->getHoldNumber(kdata.back().datetime, stock), 100.0);
    CHECK_UNARY(findRecord(tm1->getTradeList(), BUSINESS_SELL) == nullptr);

    /** @arg t_plus_n=2: the same-day sell is blocked as well */
    auto tm2 = run_sys(2);
    CHECK_EQ(tm2->getHoldNumber(kdata.back().datetime, stock), 100.0);
    CHECK_UNARY(findRecord(tm2->getTradeList(), BUSINESS_SELL) == nullptr);
}

/** @par Check point (minute KData, the unlock day, delayed mode): with t+1 the buy of day 1 is
 * sellable on day 2, with t+2 only on day 3; a sell blocked by the lock is dropped, not deferred */
TEST_CASE("test_MM_tplus_minute_unlock_day") {
    KData kdata = tplusMinuteKData();
    Stock stock = kdata.getStock();
    size_t day2 = dayStart(kdata, 1);
    size_t day3 = dayStart(kdata, 2);

    auto run_sys = [&](int t_plus_n) {
        auto tm = crtTM(Datetime(199001010000), 100000.0, TC_Zero());
        auto mm = MM_FixedCount(100);
        mm->setParam<int>("t_plus_n", t_plus_n);
        auto sys = SYS_Simple(tm, mm, EnvironmentPtr(), ConditionPtr(),
                              manualSG({{kdata[0].datetime, 1.0},
                                        {kdata[day2].datetime, -1.0},
                                        {kdata[day3].datetime, -1.0}}));
        sys->setParam<bool>("buy_delay", true);
        sys->setParam<bool>("sell_delay", true);
        sys->run(kdata);
        return tm;
    };

    /** @arg t_plus_n=1: the sell of day 2 fills on its next bar */
    auto tm1 = run_sys(1);
    TradeRecordList trs1 = tm1->getTradeList();
    const TradeRecord* sell1 = findRecord(trs1, BUSINESS_SELL);
    REQUIRE_UNARY(sell1 != nullptr);
    CHECK_EQ(sell1->datetime, kdata[day2 + 1].datetime);
    CHECK_EQ(tm1->getHoldNumber(kdata.back().datetime, stock), 0.0);

    /** @arg t_plus_n=2: the sell of day 2 is still locked (dropped), the sell of day 3 fills */
    auto tm2 = run_sys(2);
    TradeRecordList trs2 = tm2->getTradeList();
    const TradeRecord* sell2 = findRecord(trs2, BUSINESS_SELL);
    REQUIRE_UNARY(sell2 != nullptr);
    CHECK_EQ(sell2->datetime, kdata[day3 + 1].datetime);
    CHECK_EQ(tm2->getHoldNumber(kdata.back().datetime, stock), 0.0);
}

/** @par Check point (minute KData, t+2 partial): the sell closes only the lots unlocked by the
 * calendar-day distance, the lots of the last two days stay locked */
TEST_CASE("test_MM_tplus_minute_partial_sell_clamped") {
    KData kdata = tplusMinuteKData();
    Stock stock = kdata.getStock();
    size_t day2 = dayStart(kdata, 1);
    size_t day3 = dayStart(kdata, 2);

    // Buy 100 on each of the three days (each fills at its close), then sell at day 3: with
    // t_plus_n=2 only the day 1 lot is unlocked
    auto tm = crtTM(Datetime(199001010000), 1000000.0, TC_Zero());
    auto mm = MM_FixedCount(100);
    mm->setParam<int>("t_plus_n", 2);
    auto sys = SYS_Simple(tm, mm, EnvironmentPtr(), ConditionPtr(),
                          manualSG({{kdata[0].datetime, 1.0},
                                    {kdata[day2].datetime, 1.0},
                                    {kdata[day3].datetime, 1.0},
                                    {kdata[day3 + 1].datetime, -1.0}}));
    sys->setParam<bool>("buy_delay", false);
    sys->setParam<bool>("sell_delay", false);
    sys->run(kdata);

    /** @arg the sell closes only the unlocked 100, the day 2 / day 3 lots stay locked */
    CHECK_EQ(tm->getHoldNumber(kdata.back().datetime, stock), 200.0);
    TradeRecordList trs = tm->getTradeList();
    const TradeRecord* sell = findRecord(trs, BUSINESS_SELL);
    REQUIRE_UNARY(sell != nullptr);
    CHECK_EQ(sell->number, 100.0);
}

/** @par Check point (minute KData): the stop-loss liquidation (risk <= 0 without the multi-trade
 * support, resolved to the MAX_DOUBLE sentinel before the subclass is consulted) is capped by the
 * T+n lock, and the lock ages exactly by the calendar-day distance */
TEST_CASE("test_MM_tplus_minute_stoploss_liquidation_capped") {
    KData kdata = tplusMinuteKData();
    Stock stock = kdata.getStock();
    size_t day2 = dayStart(kdata, 1);
    size_t day3 = dayStart(kdata, 2);

    // A fresh MM (with a simulated buy fill via the notification channel) per n, for the cache of
    // one n must not leak into the checks of another n
    auto make_mm = [&](int t_plus_n) {
        auto tm = crtTM(Datetime(199001010000), 100000.0, TC_Zero());
        auto mm = std::make_shared<SellAllMM>();
        mm->setTM(tm);
        mm->setQuery(KQuery(0, Null<int64_t>(), KQuery::MIN5));
        mm->setParam<int>("t_plus_n", t_plus_n);
        TradeRecord tr = tm->buy(kdata[0].datetime, stock, kdata[0].closePrice, 100);
        REQUIRE_EQ(tr.business, BUSINESS_BUY);
        mm->buyNotify(tr);
        return mm;
    };

    /** @arg t_plus_n=1: locked on the buy day, free from the next day */
    auto mm1 = make_mm(1);
    CHECK_EQ(mm1->getSellNumber(kdata[1].datetime, stock, kdata[1].closePrice, 0.0, PART_SYSTEM),
             0.0);
    CHECK_EQ(
      mm1->getSellNumber(kdata[day2].datetime, stock, kdata[day2].closePrice, 0.0, PART_SYSTEM),
      MAX_DOUBLE);

    /** @arg t_plus_n=2: locked on the buy day and the next day, free from the second day after */
    auto mm2 = make_mm(2);
    CHECK_EQ(mm2->getSellNumber(kdata[1].datetime, stock, kdata[1].closePrice, 0.0, PART_SYSTEM),
             0.0);
    CHECK_EQ(
      mm2->getSellNumber(kdata[day2].datetime, stock, kdata[day2].closePrice, 0.0, PART_SYSTEM),
      0.0);
    CHECK_EQ(
      mm2->getSellNumber(kdata[day3].datetime, stock, kdata[day3].closePrice, 0.0, PART_SYSTEM),
      MAX_DOUBLE);

    /** @arg t_plus_n=0: always free */
    auto mm0 = make_mm(0);
    CHECK_EQ(mm0->getSellNumber(kdata[1].datetime, stock, kdata[1].closePrice, 0.0, PART_SYSTEM),
             MAX_DOUBLE);
}

/** @par Check point: the T+n lock ages by the trading days, not by the calendar days: with t+2
 * the Friday buy is still locked on the following Monday and unlocks on Tuesday, while t+1
 * unlocks on Monday */
TEST_CASE("test_MM_tplus_minute_weekend_t2") {
    KData kdata = tplusWeekendKData();
    Stock stock = kdata.getStock();

    auto run_sys = [&](int t_plus_n) {
        auto tm = crtTM(Datetime(199001010000), 100000.0, TC_Zero());
        auto mm = MM_FixedCount(100);
        mm->setParam<int>("t_plus_n", t_plus_n);
        auto sys = SYS_Simple(
          tm, mm, EnvironmentPtr(), ConditionPtr(),
          manualSG(
            {{kdata[0].datetime, 1.0}, {kdata[2].datetime, -1.0}, {kdata[4].datetime, -1.0}}));
        sys->setParam<bool>("buy_delay", false);
        sys->setParam<bool>("sell_delay", false);
        sys->run(kdata);
        return tm;
    };

    /** @arg t_plus_n=1: bought Friday, sellable on Monday */
    auto tm1 = run_sys(1);
    TradeRecordList trs1 = tm1->getTradeList();
    const TradeRecord* sell1 = findRecord(trs1, BUSINESS_SELL);
    REQUIRE_UNARY(sell1 != nullptr);
    CHECK_EQ(sell1->datetime, Datetime(200001100935));
    CHECK_EQ(tm1->getHoldNumber(kdata.back().datetime, stock), 0.0);

    /** @arg t_plus_n=2: still locked on Monday (one trading day later), sellable on Tuesday */
    auto tm2 = run_sys(2);
    TradeRecordList trs2 = tm2->getTradeList();
    const TradeRecord* sell2 = findRecord(trs2, BUSINESS_SELL);
    REQUIRE_UNARY(sell2 != nullptr);
    CHECK_EQ(sell2->datetime, Datetime(200001110935));
    CHECK_EQ(tm2->getHoldNumber(kdata.back().datetime, stock), 0.0);
}

/** @par Check point (minute KData): the "sell everything" sentinel (MAX_DOUBLE) is resolved by
 * the T+n cap */
TEST_CASE("test_MM_tplus_minute_sell_all_sentinel_capped") {
    KData kdata = tplusMinuteKData();
    Stock stock = kdata.getStock();
    size_t day2 = dayStart(kdata, 1);

    auto run_sys = [&](const std::vector<std::pair<Datetime, double>>& signals) {
        auto tm = crtTM(Datetime(199001010000), 100000.0, TC_Zero());
        auto sys = SYS_Simple(tm, std::make_shared<SellAllMM>(), EnvironmentPtr(), ConditionPtr(),
                              manualSG(signals));
        sys->setParam<bool>("buy_delay", false);
        sys->setParam<bool>("sell_delay", false);
        sys->run(kdata);
        return tm;
    };

    /** @arg the same-day sell-all is capped to 0 */
    auto tm1 = run_sys({{kdata[0].datetime, 1.0}, {kdata[1].datetime, -1.0}});
    CHECK_EQ(tm1->getHoldNumber(kdata.back().datetime, stock), 100.0);

    /** @arg the next-day sell-all sells the whole position */
    auto tm2 = run_sys({{kdata[0].datetime, 1.0}, {kdata[day2].datetime, -1.0}});
    CHECK_EQ(tm2->getHoldNumber(kdata.back().datetime, stock), 0.0);
}

/** @par Check point (minute KData): the lock restricts the long sells only; the short-side
 * quantities (opening a short position / covering it) are never capped, and the short-side trade
 * notifications do not enter the long lock cache */
TEST_CASE("test_MM_tplus_short_side_unaffected") {
    KData kdata = tplusMinuteKData();
    Stock stock = kdata.getStock();
    size_t day2 = dayStart(kdata, 1);
    auto tm = crtTM(Datetime(199001010000), 100000.0, TC_Zero());
    tm->setParam<bool>("support_borrow_stock", true);

    auto mm = std::make_shared<ShortProbeMM>();
    mm->setTM(tm);
    mm->setQuery(KQuery(0, Null<int64_t>(), KQuery::MIN5));

    // A long buy filled today
    TradeRecord buy = tm->buy(kdata[0].datetime, stock, kdata[0].closePrice, 100);
    REQUIRE_EQ(buy.business, BUSINESS_BUY);
    mm->buyNotify(buy);

    // A short position opened and covered on the same day
    TradeRecord short_sell = tm->sellShort(kdata[0].datetime, stock, kdata[0].closePrice, 100);
    REQUIRE_EQ(short_sell.business, BUSINESS_SELL_SHORT);
    mm->sellNotify(short_sell);
    TradeRecord cover = tm->buyShort(kdata[1].datetime, stock, kdata[1].closePrice, 100);
    REQUIRE_EQ(cover.business, BUSINESS_BUY_SHORT);
    mm->buyNotify(cover);

    price_t p = kdata[1].closePrice;

    /** @arg the long sell is still locked on the same day */
    CHECK_EQ(mm->getSellNumber(kdata[1].datetime, stock, p, p - 0.01, PART_SYSTEM), 0.0);

    /** @arg the short open / cover of the same day are not capped */
    CHECK_EQ(mm->getSellShortNumber(kdata[1].datetime, stock, p, p + 0.01, PART_SYSTEM), 100.0);
    CHECK_EQ(mm->getBuyShortNumber(kdata[1].datetime, stock, p, p + 0.01, PART_SYSTEM), 100.0);

    /** @arg the long lock is released on the next day (the short-side notifications add no lock) */
    CHECK_EQ(mm->getSellNumber(kdata[day2].datetime, stock, kdata[day2].closePrice,
                               kdata[day2].closePrice - 0.01, PART_SYSTEM),
             MAX_DOUBLE);
}

/** @par Check point (minute KData): a sellable part below minTradeNumber (e.g. the odd lot left
 * by a dividend) is treated as nothing sellable, so no rejected sell reaches the trade manager */
TEST_CASE("test_MM_tplus_sub_lot_sellable") {
    KData kdata = tplusMinuteKData();
    Stock stock = kdata.getStock();
    auto tm = crtTM(Datetime(199001010000), 100000.0, TC_Zero());

    auto mm = MM_FixedCount(100);
    mm->setTM(tm);
    mm->setQuery(KQuery(0, Null<int64_t>(), KQuery::MIN5));

    // An odd lot of 150 held from earlier, plus a lock of 100 bought today: only 50 would remain
    // sellable, which is below minTradeNumber
    TradeRecord odd_buy = tm->buy(kdata[0].datetime, stock, kdata[0].closePrice, 150);
    REQUIRE_EQ(odd_buy.business, BUSINESS_BUY);
    TradeRecord today_buy(stock, kdata[1].datetime, BUSINESS_BUY, kdata[1].closePrice,
                          kdata[1].closePrice, 0.0, 100, CostRecord(), 0.0, 0.0, PART_SYSTEM);
    mm->buyNotify(today_buy);

    /** @arg the sub-lot sellable part is capped to 0 instead of a rejected size */
    CHECK_EQ(mm->getSellNumber(kdata[1].datetime, stock, kdata[1].closePrice,
                               kdata[1].closePrice - 0.01, PART_SYSTEM),
             0.0);
}

/** @par Check point (minute KData): the lock cache is carried over by clone() and dropped by
 * reset() (a replay without reset keeps the position and its lock together) */
TEST_CASE("test_MM_tplus_reset_and_clone") {
    KData kdata = tplusMinuteKData();
    Stock stock = kdata.getStock();
    auto tm = crtTM(Datetime(199001010000), 100000.0, TC_Zero());

    auto mm = std::make_shared<SellAllMM>();
    mm->setTM(tm);
    mm->setQuery(KQuery(0, Null<int64_t>(), KQuery::MIN5));

    TradeRecord buy = tm->buy(kdata[0].datetime, stock, kdata[0].closePrice, 100);
    mm->buyNotify(buy);

    /** @arg the same-day sell is locked */
    CHECK_EQ(mm->getSellNumber(kdata[1].datetime, stock, kdata[1].closePrice,
                               kdata[1].closePrice - 0.01, PART_SYSTEM),
             0.0);

    /** @arg clone() carries the lock over with the position */
    auto copy = mm->clone();
    copy->setTM(tm);
    copy->setQuery(KQuery(0, Null<int64_t>(), KQuery::MIN5));
    CHECK_EQ(copy->getSellNumber(kdata[1].datetime, stock, kdata[1].closePrice,
                                 kdata[1].closePrice - 0.01, PART_SYSTEM),
             0.0);

    /** @arg reset() drops the cache */
    mm->reset();
    mm->setTM(tm);
    mm->setQuery(KQuery(0, Null<int64_t>(), KQuery::MIN5));
    CHECK_EQ(mm->getSellNumber(kdata[1].datetime, stock, kdata[1].closePrice,
                               kdata[1].closePrice - 0.01, PART_SYSTEM),
             MAX_DOUBLE);
}

/** @par Check point (daily KData): the rule is skipped for a bar period of a day or above, so
 * t+0 / t+1 / t+2 produce identical results */
TEST_CASE("test_MM_tplus_daily_result_unchanged") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData kdata = stk.getKData(query);
    REQUIRE(kdata.size() > 10);

    auto run_sys = [&](int t_plus_n) {
        auto tm = crtTM(Datetime(199001010000), 100000.0);
        auto mm = MM_FixedCount(100);
        mm->setParam<int>("t_plus_n", t_plus_n);
        auto sys = SYS_Simple(tm, mm, EnvironmentPtr(), ConditionPtr(),
                              SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10)));
        sys->run(kdata);
        return tm->getTradeList();
    };

    /** @arg the trade records of t_plus_n=0 / 1 / 2 are identical */
    TradeRecordList trs0 = run_sys(0);
    TradeRecordList trs1 = run_sys(1);
    TradeRecordList trs2 = run_sys(2);
    REQUIRE_EQ(trs0.size(), trs1.size());
    REQUIRE_EQ(trs0.size(), trs2.size());
    for (size_t i = 0; i < trs0.size(); ++i) {
        CHECK_EQ(trs1[i].business, trs0[i].business);
        CHECK_EQ(trs1[i].datetime, trs0[i].datetime);
        CHECK_EQ(trs1[i].number, trs0[i].number);
        CHECK_EQ(trs2[i].business, trs0[i].business);
        CHECK_EQ(trs2[i].datetime, trs0[i].datetime);
        CHECK_EQ(trs2[i].number, trs0[i].number);
    }
}

/** @} */
