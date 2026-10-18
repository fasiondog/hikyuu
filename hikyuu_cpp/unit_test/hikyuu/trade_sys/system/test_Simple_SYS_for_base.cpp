/*
 * test_SYS_Simple.cpp
 *
 *  Created on: 2013-4-20
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include "test_sys.h"
#include <map>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include <hikyuu/trade_sys/signal/crt/SG_Cross.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedCount.h>
#include <hikyuu/trade_sys/stoploss/crt/ST_FixedPercent.h>
#include <hikyuu/trade_sys/stoploss/crt/ST_Indicator.h>
#include <hikyuu/indicator/crt/KDATA.h>

using namespace hku;

/**
 * @defgroup test_SYS_Simple test_SYS_Simple
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test point (the basic operations) */
TEST_CASE("test_SYS_Simple_for_base") {
    TradeRecordList tr_list;
    price_t current_cash;

    StockManager& sm = StockManager::instance();

    // The initial parameters
    Datetime init_date(199001010000LL);   // Account initial date
    price_t init_cash = 100000;           // Account initial amount
    TradeCostPtr costfunc = TC_Zero();    // The zero cost function
    Stock stk = sm["sh600000"];           // The selected security
    Datetime start_date(199911100000LL);  // Test start date
    Datetime end_date(200002250000LL);    // Test end date

    KQuery query = KQueryByDate(start_date, end_date, KQuery::DAY);

    // Build the system parts
    TMPtr tm = crtTM(init_date, init_cash, costfunc, "TEST_TM");
    SGPtr sg = SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10));
    MMPtr mm = MM_FixedCount(100);
    SYSPtr sys;

    /** @arg Run without a given account */
    sys = SYS_Simple();
    CHECK_THROWS_AS(sys->readyForRun(), std::exception);

    /** @arg The account is given but the other strategy components are not */
    sys = SYS_Simple();
    sys->setTM(tm->clone());
    CHECK_THROWS_AS(sys->readyForRun(), std::exception);

    /** @arg TM and SG are given but the other components are not */
    sys = SYS_Simple();
    sys->setTM(tm->clone());
    sys->setSG(sg->clone());
    CHECK_THROWS_AS(sys->readyForRun(), std::exception);

    /** @arg TM, SG and MM are given but the others are not, a non-delayed operation */
    sys = SYS_Simple();
    sys->setParam("buy_delay", false);
    sys->setParam("sell_delay", false);
    sys->setTM(tm->clone());
    sys->setSG(sg->clone());
    sys->setMM(mm->clone());
    sys->run(stk, query);
    CHECK_NE(sys->getTM()->currentCash(), init_cash);
    tr_list = sys->getTM()->getTradeList();
    CHECK_NE(tr_list.size(), 1);
    CHECK_EQ(tr_list[0].business, BUSINESS_INIT);

    CHECK_EQ(tr_list[1].stock, stk);
    CHECK_EQ(tr_list[1].datetime, Datetime(199912150000LL));
    CHECK_EQ(tr_list[1].business, BUSINESS_BUY);
    CHECK_LT(std::fabs(tr_list[1].planPrice - 26.45), 0.00001);
    CHECK_LT(std::fabs(tr_list[1].realPrice - 26.45), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[1].goalPrice));
    CHECK_EQ(tr_list[1].number, 100);
    CHECK_LT(std::fabs(tr_list[1].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[1].stoploss - 0), 0.00001);
    current_cash = init_cash - 2645;
    CHECK_LT(std::fabs(tr_list[1].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[1].from, PART_SIGNAL);

    CHECK_EQ(tr_list[2].stock, stk);
    CHECK_EQ(tr_list[2].datetime, Datetime(199912220000LL));
    CHECK_EQ(tr_list[2].business, BUSINESS_SELL);
    CHECK_LT(std::fabs(tr_list[2].planPrice - 25.15), 0.00001);
    CHECK_LT(std::fabs(tr_list[2].realPrice - 25.15), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[2].goalPrice));
    CHECK_EQ(tr_list[2].number, 100);
    CHECK_LT(std::fabs(tr_list[2].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[2].stoploss - 0), 0.00001);
    current_cash += 2515;
    CHECK_LT(std::fabs(tr_list[2].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[2].from, PART_SIGNAL);

    CHECK_EQ(tr_list[3].stock, stk);
    CHECK_EQ(tr_list[3].datetime, Datetime(200001050000LL));
    CHECK_EQ(tr_list[3].business, BUSINESS_BUY);
    CHECK_LT(std::fabs(tr_list[3].planPrice - 25.28), 0.00001);
    CHECK_LT(std::fabs(tr_list[3].realPrice - 25.28), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[3].goalPrice));
    CHECK_EQ(tr_list[3].number, 100);
    CHECK_LT(std::fabs(tr_list[3].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[3].stoploss - 0), 0.00001);
    current_cash -= 2528;
    CHECK_LT(std::fabs(tr_list[3].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[3].from, PART_SIGNAL);

    /** @arg TM, SG and MM are given but the others are not, a delayed operation */
    sys = SYS_Simple();
    sys->setParam("buy_delay", true);
    sys->setParam("sell_delay", true);
    sys->setTM(tm->clone());
    sys->setSG(sg->clone());
    sys->setMM(mm->clone());
    sys->run(stk, query);
    CHECK_NE(sys->getTM()->currentCash(), init_cash);
    tr_list = sys->getTM()->getTradeList();
    CHECK_NE(tr_list.size(), 1);
    CHECK_EQ(tr_list[0].business, BUSINESS_INIT);

    CHECK_EQ(tr_list[1].stock, stk);
    CHECK_EQ(tr_list[1].datetime, Datetime(199912160000LL));
    CHECK_EQ(tr_list[1].business, BUSINESS_BUY);
    CHECK_LT(std::fabs(tr_list[1].planPrice - 26.50), 0.00001);
    CHECK_LT(std::fabs(tr_list[1].realPrice - 26.50), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[1].goalPrice));
    CHECK_EQ(tr_list[1].number, 100);
    CHECK_LT(std::fabs(tr_list[1].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[1].stoploss - 0), 0.00001);
    current_cash = init_cash - 2650;
    CHECK_LT(std::fabs(tr_list[1].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[1].from, PART_SIGNAL);

    CHECK_EQ(tr_list[2].stock, stk);
    CHECK_EQ(tr_list[2].datetime, Datetime(199912230000LL));
    CHECK_EQ(tr_list[2].business, BUSINESS_SELL);
    CHECK_LT(std::fabs(tr_list[2].planPrice - 25.20), 0.00001);
    CHECK_LT(std::fabs(tr_list[2].realPrice - 25.20), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[2].goalPrice));
    CHECK_EQ(tr_list[2].number, 100);
    CHECK_LT(std::fabs(tr_list[2].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[2].stoploss - 0), 0.00001);
    current_cash += 2520;
    CHECK_LT(std::fabs(tr_list[2].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[2].from, PART_SIGNAL);

    CHECK_EQ(tr_list[3].stock, stk);
    CHECK_EQ(tr_list[3].datetime, Datetime(200001060000LL));
    CHECK_EQ(tr_list[3].business, BUSINESS_BUY);
    CHECK_LT(std::fabs(tr_list[3].planPrice - 25.18), 0.00001);
    CHECK_LT(std::fabs(tr_list[3].realPrice - 25.18), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[3].goalPrice));
    CHECK_EQ(tr_list[3].number, 100);
    CHECK_LT(std::fabs(tr_list[3].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[3].stoploss - 0), 0.00001);
    current_cash -= 2518;
    CHECK_LT(std::fabs(tr_list[3].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[3].from, PART_SIGNAL);

    /*for (auto iter = tr_list.begin(); iter != tr_list.end(); ++iter) {
        std::cout ,, *iter ,, std::endl;
    }*/
}

/** @par Test point (the auto_checkin deposits only at the actual trade of a delayed buy) */
TEST_CASE("test_SYS_Simple_for_base_auto_checkin") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm["sh600000"];
    REQUIRE_UNARY(!stk.isNull());
    Datetime start_date(199911100000LL);
    Datetime end_date(200002250000LL);
    KQuery query = KQueryByDate(start_date, end_date, KQuery::DAY);

    // A quantity far beyond the initial cash forces a top-up on every buy; a deposit on a date
    // without a trade is the regression signature of the old query-side deposit
    SYSPtr sys = SYS_Simple();
    sys->setParam("buy_delay", true);
    sys->setParam("sell_delay", true);
    sys->setTM(crtTM(Datetime(199001010000LL), 100000, TC_Zero(), "TEST_TM"));
    sys->setSG(SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10)));
    MMPtr mm = MM_FixedCount(100000);
    mm->setParam<bool>("auto_checkin", true);
    sys->setMM(mm);

    sys->run(stk, query);

    TradeRecordList tr_list = sys->getTM()->getTradeList();
    std::map<Datetime, size_t> buy_counts, checkin_counts;
    for (const auto& tr : tr_list) {
        if (BUSINESS_BUY == tr.business) {
            buy_counts[tr.datetime]++;
        } else if (BUSINESS_CHECKIN == tr.business) {
            checkin_counts[tr.datetime]++;
        }
    }

    /** @arg the scenario really exercised the top-up path */
    CHECK_UNARY(buy_counts.size() >= 1);
    CHECK_UNARY(!checkin_counts.empty());

    /** @arg every deposit coincides with an actual buy */
    for (const auto& kv : checkin_counts) {
        CHECK_UNARY(buy_counts.count(kv.first) > 0);
    }

    /** @arg at most one deposit per buy */
    for (const auto& kv : checkin_counts) {
        CHECK_LE(kv.second, buy_counts[kv.first]);
    }
}

/** @} */
