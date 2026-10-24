/*
 * test_SYS_Simple.cpp
 *
 *  Created on: 2013-4-20
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include "test_sys.h"
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

/** @par Test point (the system valid condition strategy) */
TEST_CASE("test_SYS_Simple_for_cn") {
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
    STPtr st = ST_FixedPercent(0.01);
    TPPtr tp = ST_Indicator(MA(CLOSE(), 5));
    SYSPtr sys;

    EVPtr ev1 = make_shared<TestEV1>();
    ev1->setQuery(query);

    EVPtr ev2 = make_shared<TestEV2>();
    ev2->setQuery(query);

    EVPtr ev3 = make_shared<TestEV3>();
    ev3->setQuery(query);

    CNPtr cn1 = make_shared<TestCN1>();
    CNPtr cn2 = make_shared<TestCN2>();
    CNPtr cn3 = make_shared<TestCN3>();

    /** @arg TM, SG, MM, ST, TP, EV and CN are given; the CN valid dates overlap those of EV */
    sys = SYS_Simple();
    sys->setParam<bool>("buy_delay", false);
    sys->setParam<bool>("sell_delay", false);
    sys->setParam<bool>("ev_open_position", true);
    sys->setTM(tm->clone());
    sys->setSG(sg->clone());
    sys->setMM(mm->clone());
    sys->setST(st->clone());
    sys->setTP(tp->clone());
    sys->setEV(ev1);
    sys->setCN(cn1);

    sys->run(stk, query);
    CHECK_NE(sys->getTM()->currentCash(), init_cash);
    tr_list = sys->getTM()->getTradeList();
    CHECK_EQ(tr_list.size(), 4);
    CHECK_EQ(tr_list[0].business, BUSINESS_INIT);

    CHECK_EQ(tr_list[1].stock, stk);
    CHECK_EQ(tr_list[1].datetime, Datetime(200001040000LL));
    CHECK_EQ(tr_list[1].business, BUSINESS_BUY);
    CHECK_LT(std::fabs(tr_list[1].planPrice - 25.57), 0.00001);
    CHECK_LT(std::fabs(tr_list[1].realPrice - 25.57), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[1].goalPrice));
    CHECK_EQ(tr_list[1].number, 100);
    CHECK_LT(std::fabs(tr_list[1].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[1].stoploss - 25.31), 0.00001);
    current_cash = init_cash - 2557;
    CHECK_LT(std::fabs(tr_list[1].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[1].from, PART_ENVIRONMENT);

    CHECK_EQ(tr_list[2].stock, stk);
    CHECK_EQ(tr_list[2].datetime, Datetime(200001050000LL));
    CHECK_EQ(tr_list[2].business, BUSINESS_BUY);
    CHECK_LT(std::fabs(tr_list[2].planPrice - 25.28), 0.00001);
    CHECK_LT(std::fabs(tr_list[2].realPrice - 25.28), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[2].goalPrice));
    CHECK_EQ(tr_list[2].number, 100);
    CHECK_LT(std::fabs(tr_list[2].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[2].stoploss - 25.03), 0.00001);
    current_cash -= 2528;
    CHECK_LT(std::fabs(tr_list[2].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[2].from, PART_SIGNAL);

    CHECK_EQ(tr_list[3].stock, stk);
    CHECK_EQ(tr_list[3].datetime, Datetime(200001100000LL));
    CHECK_EQ(tr_list[3].business, BUSINESS_SELL);
    CHECK_LT(std::fabs(tr_list[3].planPrice - 27.25), 0.00001);
    CHECK_LT(std::fabs(tr_list[3].realPrice - 27.25), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[3].goalPrice));
    CHECK_EQ(tr_list[3].number, 200);
    CHECK_LT(std::fabs(tr_list[3].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[3].stoploss - 26.98), 0.00001);
    current_cash += 2725 * 2;
    CHECK_LT(std::fabs(tr_list[3].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[3].from, PART_ENVIRONMENT);

    /** @arg TM, SG, MM, ST, TP, EV and CN are given; the CN valid range fully covers and exceeds EV
     */
    sys = SYS_Simple();
    sys->setParam<bool>("buy_delay", false);
    sys->setParam<bool>("sell_delay", false);
    sys->setParam<bool>("ev_open_position", true);
    sys->setTM(tm->clone());
    sys->setSG(sg->clone());
    sys->setMM(mm->clone());
    sys->setST(st->clone());
    sys->setTP(tp->clone());
    sys->setEV(ev1);
    sys->setCN(cn2);

    sys->run(stk, query);
    CHECK_NE(sys->getTM()->currentCash(), init_cash);
    tr_list = sys->getTM()->getTradeList();
    CHECK_EQ(tr_list.size(), 4);
    CHECK_EQ(tr_list[0].business, BUSINESS_INIT);

    CHECK_EQ(tr_list[1].stock, stk);
    CHECK_EQ(tr_list[1].datetime, Datetime(200001040000LL));
    CHECK_EQ(tr_list[1].business, BUSINESS_BUY);
    CHECK_LT(std::fabs(tr_list[1].planPrice - 25.57), 0.00001);
    CHECK_LT(std::fabs(tr_list[1].realPrice - 25.57), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[1].goalPrice));
    CHECK_EQ(tr_list[1].number, 100);
    CHECK_LT(std::fabs(tr_list[1].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[1].stoploss - 25.31), 0.00001);
    current_cash = init_cash - 2557;
    CHECK_LT(std::fabs(tr_list[1].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[1].from, PART_ENVIRONMENT);

    CHECK_EQ(tr_list[2].stock, stk);
    CHECK_EQ(tr_list[2].datetime, Datetime(200001050000LL));
    CHECK_EQ(tr_list[2].business, BUSINESS_BUY);
    CHECK_LT(std::fabs(tr_list[2].planPrice - 25.28), 0.00001);
    CHECK_LT(std::fabs(tr_list[2].realPrice - 25.28), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[2].goalPrice));
    CHECK_EQ(tr_list[2].number, 100);
    CHECK_LT(std::fabs(tr_list[2].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[2].stoploss - 25.03), 0.00001);
    current_cash -= 2528;
    CHECK_LT(std::fabs(tr_list[2].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[2].from, PART_SIGNAL);

    CHECK_EQ(tr_list[3].stock, stk);
    CHECK_EQ(tr_list[3].datetime, Datetime(200001100000LL));
    CHECK_EQ(tr_list[3].business, BUSINESS_SELL);
    CHECK_LT(std::fabs(tr_list[3].planPrice - 27.25), 0.00001);
    CHECK_LT(std::fabs(tr_list[3].realPrice - 27.25), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[3].goalPrice));
    CHECK_EQ(tr_list[3].number, 200);
    CHECK_LT(std::fabs(tr_list[3].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[3].stoploss - 26.98), 0.00001);
    current_cash += 2725 * 2;
    CHECK_LT(std::fabs(tr_list[3].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[3].from, PART_ENVIRONMENT);

    /** @arg TM, SG, MM, EV and CN are given (no position building); the CN range is inside EV */
    sys = SYS_Simple();
    sys->setParam<bool>("buy_delay", false);
    sys->setParam<bool>("sell_delay", false);
    sys->setParam<bool>("ev_open_position", true);
    sys->setParam<bool>("cn_open_position", false);
    sys->setTM(tm->clone());
    sys->setSG(sg->clone());
    sys->setMM(mm->clone());
    sys->setEV(ev1);
    sys->setCN(cn3);

    sys->run(stk, query);
    CHECK_NE(sys->getTM()->currentCash(), init_cash);
    tr_list = sys->getTM()->getTradeList();
    CHECK_EQ(tr_list.size(), 4);
    CHECK_EQ(tr_list[0].business, BUSINESS_INIT);

    CHECK_EQ(tr_list[1].stock, stk);
    CHECK_EQ(tr_list[1].datetime, Datetime(200001040000LL));
    CHECK_EQ(tr_list[1].business, BUSINESS_BUY);
    CHECK_LT(std::fabs(tr_list[1].planPrice - 25.57), 0.00001);
    CHECK_LT(std::fabs(tr_list[1].realPrice - 25.57), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[1].goalPrice));
    CHECK_EQ(tr_list[1].number, 100);
    CHECK_LT(std::fabs(tr_list[1].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[1].stoploss - 0.0), 0.00001);
    current_cash = init_cash - 2557;
    CHECK_LT(std::fabs(tr_list[1].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[1].from, PART_ENVIRONMENT);

    CHECK_EQ(tr_list[2].stock, stk);
    CHECK_EQ(tr_list[2].datetime, Datetime(200001050000LL));
    CHECK_EQ(tr_list[2].business, BUSINESS_BUY);
    CHECK_LT(std::fabs(tr_list[2].planPrice - 25.28), 0.00001);
    CHECK_LT(std::fabs(tr_list[2].realPrice - 25.28), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[2].goalPrice));
    CHECK_EQ(tr_list[2].number, 100);
    CHECK_LT(std::fabs(tr_list[2].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[2].stoploss - 0), 0.00001);
    current_cash -= 2528;
    CHECK_LT(std::fabs(tr_list[2].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[2].from, PART_SIGNAL);

    CHECK_EQ(tr_list[3].stock, stk);
    CHECK_EQ(tr_list[3].datetime, Datetime(200001070000LL));
    CHECK_EQ(tr_list[3].business, BUSINESS_SELL);
    CHECK_LT(std::fabs(tr_list[3].planPrice - 26.90), 0.00001);
    CHECK_LT(std::fabs(tr_list[3].realPrice - 26.90), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[3].goalPrice));
    CHECK_EQ(tr_list[3].number, 200);
    CHECK_LT(std::fabs(tr_list[3].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[3].stoploss - 0), 0.00001);
    current_cash += 2690 * 2;
    CHECK_LT(std::fabs(tr_list[3].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[3].from, PART_CONDITION);

    /** @arg TM, SG, MM, EV and CN are given (position building); the CN range is inside EV */
    sys = SYS_Simple();
    sys->setParam<bool>("buy_delay", false);
    sys->setParam<bool>("sell_delay", false);
    sys->setParam<bool>("ev_open_position", true);
    sys->setParam<bool>("cn_open_position", true);
    sys->setTM(tm->clone());
    sys->setSG(sg->clone());
    sys->setMM(mm->clone());
    sys->setEV(ev1);
    sys->setCN(cn3);

    sys->run(stk, query);
    CHECK_NE(sys->getTM()->currentCash(), init_cash);
    tr_list = sys->getTM()->getTradeList();
    CHECK_EQ(tr_list.size(), 4);
    CHECK_EQ(tr_list[0].business, BUSINESS_INIT);

    CHECK_EQ(tr_list[1].stock, stk);
    CHECK_EQ(tr_list[1].datetime, Datetime(200001040000LL));
    CHECK_EQ(tr_list[1].business, BUSINESS_BUY);
    CHECK_LT(std::fabs(tr_list[1].planPrice - 25.57), 0.00001);
    CHECK_LT(std::fabs(tr_list[1].realPrice - 25.57), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[1].goalPrice));
    CHECK_EQ(tr_list[1].number, 100);
    CHECK_LT(std::fabs(tr_list[1].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[1].stoploss - 0), 0.00001);
    current_cash = init_cash - 2557;
    CHECK_LT(std::fabs(tr_list[1].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[1].from, PART_ENVIRONMENT);

    CHECK_EQ(tr_list[2].stock, stk);
    CHECK_EQ(tr_list[2].datetime, Datetime(200001050000LL));
    CHECK_EQ(tr_list[2].business, BUSINESS_BUY);
    CHECK_LT(std::fabs(tr_list[2].planPrice - 25.28), 0.00001);
    CHECK_LT(std::fabs(tr_list[2].realPrice - 25.28), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[2].goalPrice));
    CHECK_EQ(tr_list[2].number, 100);
    CHECK_LT(std::fabs(tr_list[2].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[2].stoploss - 0), 0.00001);
    current_cash -= 2528;
    CHECK_LT(std::fabs(tr_list[2].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[2].from, PART_CONDITION);

    CHECK_EQ(tr_list[3].stock, stk);
    CHECK_EQ(tr_list[3].datetime, Datetime(200001070000LL));
    CHECK_EQ(tr_list[3].business, BUSINESS_SELL);
    CHECK_LT(std::fabs(tr_list[3].planPrice - 26.90), 0.00001);
    CHECK_LT(std::fabs(tr_list[3].realPrice - 26.90), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[3].goalPrice));
    CHECK_EQ(tr_list[3].number, 200);
    CHECK_LT(std::fabs(tr_list[3].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[3].stoploss - 0), 0.00001);
    current_cash += 2690 * 2;
    CHECK_LT(std::fabs(tr_list[3].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[3].from, PART_CONDITION);

    /*for (auto iter = tr_list.begin(); iter , tr_list.end(); ++iter) {
        std::cout ,, *iter ,, std::endl;
    }*/
}

//-----------------------------------------------------------------------------
// The invalid→valid transition open must not misfire on a run(reset=false) re-entry
//-----------------------------------------------------------------------------

/** @par Check points (the cn_open_position transition on a run(reset=false) re-entry):
 *  the initial position is built once; the re-entry run must not buy again */
TEST_CASE("test_SYS_Simple_cn_open_position_no_reopen_on_rerun") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm["sh600000"];
    KQuery query = KQueryByDate(Datetime(199911100000LL), Datetime(200002250000LL), KQuery::DAY);
    KData k = stk.getKData(query);
    REQUIRE_GE(k.size(), 10);

    // The CN is invalid on the first bar only, valid on every bar afterwards
    DatetimeList valid(k.size() - 1);
    for (size_t i = 1; i < k.size(); ++i) {
        valid[i - 1] = k[i].datetime;
    }

    // The re-entry window starts well after the initial position was built
    KQuery query2 = KQueryByDate(k[8].datetime, Datetime(200002250000LL), KQuery::DAY);
    KData k2 = stk.getKData(query2);
    REQUIRE_GE(k2.size(), 5);

    auto sys = SYS_Simple();
    sys->setParam<bool>("cn_open_position", true);
    // buy_delay default true: the transition buy fills long before the re-entry window
    sys->setTM(crtTM(Datetime(199001010000LL), 100000, TC_Zero(), "TEST_TM"));
    sys->setSG(make_shared<NoSignalSG>());
    sys->setMM(MM_FixedCount(100));
    sys->setCN(make_shared<RangeCN>(valid));

    sys->run(stk, query);

    /** @arg the invalid→valid transition at the second bar builds the initial position once */
    CHECK_EQ(countBusiness(sys->getTM()->getTradeList(), BUSINESS_BUY), 1);
    CHECK_EQ(sys->getTM()->getPosition(k.back().datetime, stk).number, 100.0);

    sys->run(stk, query2, false, false);

    /** @arg the re-entry's invalid→valid transition must not buy again on top of the holding */
    CHECK_EQ(countBusiness(sys->getTM()->getTradeList(), BUSINESS_BUY), 1);
    CHECK_EQ(sys->getTM()->getPosition(k.back().datetime, stk).number, 100.0);

    sys->run(stk, query2, false, true);

    /** @arg the forced full reset re-forces the pre-flags: the transition builds again */
    CHECK_EQ(countBusiness(sys->getTM()->getTradeList(), BUSINESS_BUY), 1);
    CHECK_EQ(sys->getTM()->getPosition(k.back().datetime, stk).number, 100.0);
    CHECK_GE(sys->getTM()->getTradeList().back().datetime, k2.front().datetime);
}

/** @par Check points (the CN pre-flag while the EV stays invalid): the CN validity is tracked on
 *  every bar, the early return of an invalid EV included, so no stale transition build happens */
TEST_CASE("test_SYS_Simple_cn_transition_not_replayed_after_ev_invalid") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm["sh600000"];
    KQuery query = KQueryByDate(Datetime(199911100000LL), Datetime(200002250000LL), KQuery::DAY);
    KData k = stk.getKData(query);
    REQUIRE_GE(k.size(), 10);

    // The EV is invalid on the first three bars, the CN on the first bar only: the CN becomes
    // valid while the EV still blocks every trade
    DatetimeList ev_valid;
    for (size_t i = 3; i < k.size(); ++i) {
        ev_valid.push_back(k[i].datetime);
    }

    DatetimeList cn_valid;
    for (size_t i = 1; i < k.size(); ++i) {
        cn_valid.push_back(k[i].datetime);
    }

    auto sys = SYS_Simple();
    sys->setParam<bool>("cn_open_position", true);
    sys->setTM(crtTM(Datetime(199001010000LL), 100000, TC_Zero(), "TEST_TM"));
    sys->setSG(make_shared<NoSignalSG>());
    sys->setMM(MM_FixedCount(100));
    sys->setEV(make_shared<RangeEV>(ev_valid));
    sys->setCN(make_shared<RangeCN>(cn_valid));

    sys->run(stk, query);

    /** @arg the CN transition is consumed while the EV is invalid: no build on the EV recovery */
    CHECK_EQ(countBusiness(sys->getTM()->getTradeList(), BUSINESS_BUY), 0);
    CHECK_EQ(sys->getTM()->getPosition(k.back().datetime, stk).number, 0.0);
}

/** @} */
