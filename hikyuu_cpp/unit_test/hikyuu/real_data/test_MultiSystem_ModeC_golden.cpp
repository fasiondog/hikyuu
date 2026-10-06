/*
 * test_MultiSystem_ModeC_golden.cpp
 *
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-10-07
 *      Author: fasiondog
 *
 *  The mode C golden alignment test (it needs the real market data, so it belongs to the real_data
 *  directory and runs with: xmake r real-test).
 *
 *  The fixture below is the per-trade detail produced by the Portfolio of the previous release (the
 *  official wheel hikyuu_2.8.3_20260930, old engine), running the 8-ETF trend bollinger portfolio:
 *      sg.趋势布林带 SG_Band(CLOSE, MA-0.5*STDEV, MA+0.5*STDEV) with n=10 / band=0.5,
 *      mm MM_FixedCapitalFunds(20), SE_Signal over the 8 ETFs, PF_WithoutAF with
 *      adjust_cycle=1, adjust_mode="query", delay_to_trading_day=true, trade_on_close=true,
 *      sys_use_self_tm=false, sell_at_not_selected=false, account 1000000 at 2015-01-01 with
 *      TC_FixedA2017.
 *  It is embedded as a constant on purpose: the tests must not depend on an installed legacy wheel
 *  (see the design 13).
 *
 *  @note CONFIG COUPLING: this fixture was captured with the weight (ex-rights / cash dividend)
 * data loaded, which is what the C++ test binaries get from ~/.hikyuu/hikyuu.ini (load_stock_weight
 *        defaults to true). The strategy side of hikyuu_star loads the data with load_weight=False,
 *        where the same engine yields the 129 buys / 62 sells set (archived next to this plan as
 *        golden_modeC_trades.csv). Both runs were verified against the legacy engine of their own
 *        configuration, so the mode C semantics do not depend on that switch.
 *
 *  @note The golden trade set is tied to the local market data snapshot used when it was captured
 *        (the daily data up to 2026-09-30). If the local data grows, new signals may appear after
 *        that date and the sets drift; regenerate the fixture then, following the procedure of the
 *        implementation plan P0.2 (run the old wheel in a throwaway virtual environment with
 *        PYTHONPATH cleared, dump the trade list, uninstall).
 */

#include "doctest/doctest.h"
#include <cmath>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/STDEV.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_manage/crt/TC_FixedA2017.h>
#include <hikyuu/trade_sys/signal/crt/SG_Band.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedCapitalFunds.h>
#include <hikyuu/trade_sys/selector/crt/SE_Signal.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include <hikyuu/trade_sys/portfolio/build_in.h>

using namespace hku;

/**
 * @defgroup test_MultiSystem_ModeC_golden test_MultiSystem_ModeC_golden
 * @ingroup test_hikyuu_real_data
 * @{
 */

/** One golden trade: the day (yyyymmdd), the instrument, the side (B=buy, S=sell), the quantity */
struct GoldenTrade {
    unsigned long long day;
    const char* code;
    char side;
    double number;
};

/** The golden fixture: 225 trades (158 buys / 67 sells) of the legacy Portfolio */
static const GoldenTrade GOLDEN[] = {
  {20150116, "SH510300", 'B', 50000},  {20150116, "SH510500", 'B', 49900},
  {20150116, "SZ159915", 'B', 49900},  {20150116, "SH512010", 'B', 49900},
  {20150116, "SH518880", 'B', 49900},  {20150119, "SH510300", 'S', 50000},
  {20150119, "SH510500", 'S', 49900},  {20150119, "SH512010", 'S', 49900},
  {20150120, "SH510500", 'B', 49000},  {20150121, "SH512010", 'B', 49200},
  {20150126, "SH510300", 'B', 49500},  {20150203, "SZ159915", 'B', 48900},
  {20150210, "SZ159915", 'B', 48400},  {20150211, "SH510300", 'B', 48700},
  {20150212, "SH510500", 'B', 48800},  {20150213, "SH512010", 'B', 49400},
  {20150312, "SH510300", 'B', 1000},   {20150511, "SH512010", 'B', 100},
  {20151029, "SH518880", 'S', 49900},  {20151104, "SH510300", 'B', 31900},
  {20151104, "SZ159915", 'B', 100},    {20160120, "SZ159915", 'B', 3200},
  {20160121, "SZ159915", 'S', 150500}, {20160203, "SZ159915", 'B', 47800},
  {20160204, "SH512010", 'B', 48400},  {20160215, "SH510500", 'B', 25500},
  {20160216, "SH510300", 'B', 100},    {20160524, "SH510300", 'S', 131200},
  {20160531, "SH510300", 'B', 49700},  {20160531, "SH512010", 'B', 49700},
  {20160606, "SH518880", 'B', 50000},  {20160622, "SH510500", 'B', 8800},
  {20160727, "SH510300", 'S', 49700},  {20160727, "SH510500", 'S', 61716},
  {20160727, "SZ159915", 'S', 47800},  {20160728, "SH518880", 'B', 52500},
  {20160808, "SH510300", 'B', 52500},  {20160808, "SH510500", 'B', 50900},
  {20160809, "SZ159915", 'B', 200},    {20160822, "SH518880", 'S', 102500},
  {20160826, "SH512010", 'B', 53200},  {20160905, "SH518880", 'B', 53100},
  {20160906, "SH510300", 'B', 21000},  {20160906, "SZ159915", 'B', 100},
  {20161026, "SZ159915", 'S', 300},    {20161103, "SH510300", 'B', 100},
  {20161103, "SH512010", 'B', 200},    {20161109, "SH510500", 'S', 50900},
  {20161110, "SH510500", 'B', 49900},  {20161114, "SZ159915", 'B', 200},
  {20161118, "SH512010", 'B', 100},    {20161215, "SH518880", 'S', 53100},
  {20161219, "SH512010", 'B', 52600},  {20161229, "SH518880", 'B', 25900},
  {20170103, "SH512010", 'B', 100},    {20170123, "SH510300", 'S', 73600},
  {20170125, "SH510500", 'B', 38700},  {20170125, "SH512010", 'B', 100},
  {20170327, "SZ159915", 'S', 200},    {20170405, "SH510300", 'B', 100},
  {20170914, "SH510300", 'S', 100},    {20170918, "SH510300", 'B', 100},
  {20170919, "SH510300", 'S', 100},    {20170920, "SH510300", 'B', 100},
  {20180515, "SH518880", 'S', 25900},  {20180521, "SH510500", 'B', 10800},
  {20180601, "SH512010", 'S', 303100}, {20180605, "SH510300", 'B', 61100},
  {20180606, "SH512010", 'B', 61000},  {20180612, "SH510300", 'B', 60300},
  {20180613, "SH510300", 'S', 121500}, {20180614, "SH518880", 'B', 59400},
  {20180621, "SH511260", 'B', 3100},   {20180629, "SZ159915", 'B', 3200},
  {20180711, "SH510500", 'S', 99400},  {20180712, "SH510500", 'B', 56500},
  {20180716, "SH511260", 'B', 2100},   {20180723, "SH518880", 'B', 1000},
  {20180813, "SZ159915", 'B', 100},    {20181130, "SH511260", 'S', 5200},
  {20181203, "SH510300", 'B', 54000},  {20181203, "SH510500", 'B', 54000},
  {20181203, "SZ159915", 'B', 54000},  {20181203, "SH512010", 'B', 19600},
  {20181214, "SH510300", 'S', 54000},  {20181221, "SH511260", 'B', 1600},
  {20190104, "SH510300", 'B', 1600},   {20190118, "SZ159915", 'B', 100},
  {20190422, "SH510500", 'S', 110500}, {20190424, "SZ159915", 'B', 62300},
  {20190425, "SZ159915", 'S', 119700}, {20190430, "SH518880", 'B', 62000},
  {20190430, "SH511260", 'B', 5600},   {20190516, "SH510500", 'B', 500},
  {20190516, "SH512010", 'B', 200},    {20191225, "SZ159915", 'B', 100},
  {20200527, "SH512010", 'S', 80800},  {20200601, "SH510300", 'B', 43900},
  {20200601, "SH515050", 'B', 100},    {20201030, "SH510300", 'S', 45500},
  {20201102, "SZ159915", 'B', 74700},  {20201103, "SH510300", 'B', 4100},
  {20201103, "SH515050", 'B', 300},    {20210118, "SZ159915", 'B', 100},
  {20210119, "SZ159915", 'S', 74900},  {20210120, "SH510500", 'B', 30700},
  {20210210, "SH515050", 'B', 100},    {20210319, "SH510300", 'S', 4100},
  {20210324, "SH511260", 'B', 100},    {20210326, "SH510500", 'B', 1300},
  {20210326, "SZ159915", 'B', 100},    {20210513, "SH510500", 'S', 32500},
  {20210514, "SH510300", 'B', 45200},  {20210518, "SH515050", 'B', 100},
  {20211103, "SH518880", 'S', 122400}, {20211104, "SH510500", 'B', 55100},
  {20211104, "SH512690", 'B', 200},    {20211105, "SH510500", 'S', 55100},
  {20211109, "SH510500", 'B', 54300},  {20211110, "SH512010", 'B', 1200},
  {20211213, "SH511260", 'S', 7300},   {20211214, "SH518880", 'B', 76000},
  {20211214, "SH511260", 'B', 4800},   {20211215, "SH518880", 'S', 76000},
  {20211217, "SH518880", 'B', 75500},  {20211222, "SH515050", 'B', 4800},
  {20211228, "SH512010", 'B', 100},    {20211229, "SH512690", 'S', 200},
  {20211229, "SH512010", 'S', 1300},   {20211230, "SH510500", 'B', 100},
  {20220118, "SH515050", 'B', 200},    {20220121, "SH512690", 'B', 3700},
  {20220511, "SH512010", 'B', 100},    {20220513, "SH518880", 'S', 75500},
  {20220520, "SH512690", 'B', 70600},  {20220520, "SH512010", 'B', 70600},
  {20220526, "SH510500", 'B', 30400},  {20220526, "SH518880", 'B', 100},
  {20220530, "SH512690", 'B', 300},    {20220614, "SH518880", 'S', 100},
  {20220620, "SH515050", 'B', 400},    {20220715, "SZ159915", 'S', 100},
  {20220719, "SH515050", 'B', 300},    {20220727, "SH512690", 'S', 74600},
  {20220728, "SH510500", 'B', 8700},   {20220728, "SH515050", 'B', 600},
  {20220913, "SH512010", 'S', 70700},  {20220920, "SH512690", 'B', 40400},
  {20220929, "SH512010", 'B', 100},    {20221216, "SH515050", 'S', 6900},
  {20221223, "SH512690", 'B', 6500},   {20230209, "SH510500", 'B', 400},
  {20230209, "SH512690", 'B', 300},    {20230214, "SH512010", 'B', 100},
  {20230215, "SH512010", 'S', 200},    {20230301, "SH515050", 'B', 100},
  {20230330, "SH511260", 'S', 4800},   {20230403, "SH511260", 'B', 4700},
  {20230404, "SH518880", 'B', 2200},   {20230407, "SH512010", 'B', 300},
  {20230428, "SH518880", 'S', 2200},   {20230504, "SH518880", 'B', 2100},
  {20230515, "SH512010", 'B', 400},    {20231011, "SH511260", 'S', 4700},
  {20231013, "SH511260", 'B', 4600},   {20231017, "SH518880", 'B', 2000},
  {20231026, "SH512690", 'B', 500},    {20231120, "SH518880", 'S', 4100},
  {20231121, "SH512690", 'B', 24400},  {20240125, "SH510300", 'B', 900},
  {20240125, "SH515050", 'B', 100},    {20240126, "SH515050", 'S', 200},
  {20240129, "SH512690", 'B', 200},    {20240130, "SH512690", 'S', 72300},
  {20240206, "SH510300", 'B', 13300},  {20240206, "SH512690", 'B', 100},
  {20240207, "SH512010", 'B', 100},    {20240517, "SH510300", 'B', 2500},
  {20240517, "SH512690", 'B', 100},    {20240605, "SH512010", 'B', 100},
  {20240606, "SH512010", 'S', 900},    {20240709, "SH515050", 'B', 300},
  {20240902, "SH512690", 'S', 200},    {20240902, "SH515050", 'S', 300},
  {20240903, "SH512010", 'B', 1100},   {20250116, "SH510300", 'B', 2500},
  {20250117, "SH512690", 'B', 100},    {20250306, "SH511260", 'S', 4600},
  {20250310, "SH518880", 'B', 76200},  {20250314, "SZ159915", 'B', 55200},
  {20250407, "SH512690", 'S', 100},    {20250410, "SH512690", 'B', 100},
  {20250624, "SH510500", 'B', 900},    {20250624, "SZ159915", 'B', 200},
  {20250926, "SH512010", 'S', 1100},   {20250930, "SH512010", 'B', 1100},
  {20251028, "SH512010", 'S', 1100},   {20251029, "SH512010", 'B', 1000},
  {20251030, "SH512010", 'S', 1000},   {20251110, "SH512690", 'B', 700},
  {20251114, "SH510300", 'S', 64400},  {20251114, "SH510500", 'S', 108393},
  {20251114, "SZ159915", 'S', 55400},  {20251117, "SH511260", 'B', 9300},
  {20251126, "SH515050", 'B', 4700},   {20251210, "SH512010", 'B', 100},
  {20251226, "SH512690", 'S', 800},    {20260105, "SH512010", 'B', 20900},
  {20260202, "SH515050", 'S', 4700},   {20260203, "SH515050", 'B', 4600},
  {20260204, "SH515050", 'S', 4600},   {20260205, "SH512010", 'B', 27500},
  {20260325, "SH515050", 'B', 2500},   {20260326, "SH515050", 'S', 2500},
  {20260330, "SH512010", 'B', 16500},  {20260630, "SH510300", 'B', 2300},
  {20260706, "SH512690", 'B', 600},    {20260707, "SH512690", 'S', 600},
  {20260710, "SH512690", 'B', 600},    {20260902, "SH510300", 'S', 2300},
  {20260907, "SH515050", 'B', 10000},  {20260918, "SH510500", 'B', 1500},
  {20260921, "SH512010", 'B', 400},    {20260924, "SH512690", 'S', 600},
  {20260930, "SH512690", 'B', 600},
};

static const size_t GOLDEN_COUNT = sizeof(GOLDEN) / sizeof(GoldenTrade);

/** @par Check point: mode C reproduces the Portfolio of the previous release trade by trade (the
 * design 13 golden acceptance) */
TEST_CASE("test_MultiSystem_ModeC_golden") {
    static const char* ETFS[] = {"SH510300", "SH510500", "SZ159915", "SH512690",
                                 "SH515050", "SH512010", "SH518880", "SH511260"};
    static const size_t ETF_COUNT = sizeof(ETFS) / sizeof(ETFS[0]);

    StockList stks;
    for (size_t i = 0; i < ETF_COUNT; ++i) {
        Stock stk = getStock(ETFS[i]);
        if (stk.isNull()) {
            MESSAGE("skip the golden test: " << ETFS[i] << " is not in the local data");
            return;
        }
        stks.emplace_back(stk);
    }
    KData probe = stks[0].getKData(KQuery(Datetime(20150101), Null<Datetime>()));
    if (probe.size() < 2000) {
        MESSAGE("skip the golden test: the daily data of " << ETFS[0] << " is too short ("
                                                           << probe.size() << " bars)");
        return;
    }

    // The strategy chain of the golden run, exactly as the hub parts assemble it
    int n = 10;
    double band = 0.5;
    Indicator ma = MA(CLOSE(), n);
    Indicator sd = STDEV(CLOSE(), n);
    auto sg = SG_Band(CLOSE(), ma - band * sd, ma + band * sd);
    auto proto =
      SYS_Simple(crtTM(), MM_FixedCapitalFunds(20), EnvironmentPtr(), ConditionPtr(), sg);
    auto se = SE_Signal(stks, proto);
    auto tm = crtTM(Datetime(20150101), 1000000.0, TC_FixedA2017());

    // PF_WithoutAF is the mode C preset; the sub-systems come from the SE prototypes (the legacy
    // call style, no explicit add)
    auto pf = PF_WithoutAF(tm, se, 1, "query", true, true, false, false);
    REQUIRE_EQ(pf->getMode(), "C");
    pf->run(KQuery(Datetime(20150101), Null<Datetime>()));

    // Collect the trades of the shared real account
    std::set<std::pair<unsigned long long, std::string>> got_buy, got_sell;
    double buy_number_sum = 0.0;
    for (auto& t : tm->getTradeList()) {
        if (t.business != BUSINESS_BUY && t.business != BUSINESS_SELL) {
            continue;
        }
        auto key = std::make_pair(t.datetime.ymd(), t.stock.market_code());
        if (t.business == BUSINESS_BUY) {
            got_buy.insert(key);
            buy_number_sum += t.number;
        } else {
            got_sell.insert(key);
        }
    }

    // The expected sets, derived from the fixture
    std::set<std::pair<unsigned long long, std::string>> exp_buy, exp_sell;
    double exp_buy_number_sum = 0.0;
    for (size_t i = 0; i < GOLDEN_COUNT; ++i) {
        auto key = std::make_pair(GOLDEN[i].day, std::string(GOLDEN[i].code));
        if (GOLDEN[i].side == 'B') {
            exp_buy.insert(key);
            exp_buy_number_sum += GOLDEN[i].number;
        } else {
            exp_sell.insert(key);
        }
    }

    /** @arg the buy and the sell sets match the legacy engine exactly (the design 13: 100% overlap
     * on both sides) */
    CHECK_EQ(got_buy.size(), exp_buy.size());
    CHECK_EQ(got_sell.size(), exp_sell.size());
    CHECK(got_buy == exp_buy);
    CHECK(got_sell == exp_sell);

    /** @arg the average buy size is the same as the legacy engine (19308) */
    if (!got_buy.empty()) {
        double got_avg = buy_number_sum / got_buy.size();
        double exp_avg = exp_buy_number_sum / exp_buy.size();
        CHECK_LT(std::fabs(got_avg - exp_avg), exp_avg * 0.01);
    }

    /** @arg the cash of the shared account ends at the same value (the legacy engine leaves
     * 15.87 there, the geometric decay of MM_FixedCapitalFunds nearly empties it) */
    FundsRecord funds = tm->getFunds(tm->lastDatetime());
    CHECK_LT(std::fabs(funds.cash - 15.87), 1.0);
}

/** @} */
