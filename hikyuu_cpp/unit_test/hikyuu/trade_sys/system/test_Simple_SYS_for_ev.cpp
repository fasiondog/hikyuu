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

/** @par Test point (the market environment strategy) */
TEST_CASE("test_SYS_Simple_for_ev") {
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

    /** @arg TM, SG, MM, ST, TP and EV are given (no position building) but the others are not */
    sys = SYS_Simple();
    sys->setParam<bool>("buy_delay", false);
    sys->setParam<bool>("sell_delay", false);
    sys->setParam<bool>("ev_open_position", false);
    sys->setTM(tm->clone());
    sys->setSG(sg->clone());
    sys->setMM(mm->clone());
    sys->setST(st->clone());
    sys->setTP(tp->clone());
    sys->setEV(ev1);

    sys->run(stk, query);
    CHECK_NE(sys->getTM()->currentCash(), init_cash);
    tr_list = sys->getTM()->getTradeList();
    CHECK_EQ(tr_list.size(), 3);
    CHECK_EQ(tr_list[0].business, BUSINESS_INIT);

    CHECK_EQ(tr_list[1].stock, stk);
    CHECK_EQ(tr_list[1].datetime, Datetime(200001050000LL));
    CHECK_EQ(tr_list[1].business, BUSINESS_BUY);
    CHECK_LT(std::fabs(tr_list[1].planPrice - 25.28), 0.00001);
    CHECK_LT(std::fabs(tr_list[1].realPrice - 25.28), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[1].goalPrice));
    CHECK_EQ(tr_list[1].number, 100);
    CHECK_LT(std::fabs(tr_list[1].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[1].stoploss - 25.03), 0.00001);
    current_cash = init_cash - 2528;
    CHECK_LT(std::fabs(tr_list[1].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[1].from, PART_SIGNAL);

    CHECK_EQ(tr_list[2].stock, stk);
    CHECK_EQ(tr_list[2].datetime, Datetime(200001100000LL));
    CHECK_EQ(tr_list[2].business, BUSINESS_SELL);
    CHECK_LT(std::fabs(tr_list[2].planPrice - 27.25), 0.00001);
    CHECK_LT(std::fabs(tr_list[2].realPrice - 27.25), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[2].goalPrice));
    CHECK_EQ(tr_list[2].number, 100);
    CHECK_LT(std::fabs(tr_list[2].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[2].stoploss - 26.98), 0.00001);
    current_cash += 2725;
    CHECK_LT(std::fabs(tr_list[2].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[2].from, PART_ENVIRONMENT);

    /** @arg TM, SG, MM, ST, TP and EV are given (position building) but the others are not */
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

    /** @arg TM, SG, MM, ST, TP, EV given (no position building), the others are not, delayed */
    sys = SYS_Simple();
    sys->setParam<bool>("buy_delay", true);
    sys->setParam<bool>("sell_delay", true);
    sys->setParam<bool>("ev_open_position", false);
    sys->setTM(tm->clone());
    sys->setSG(sg->clone());
    sys->setMM(mm->clone());
    sys->setST(st->clone());
    sys->setTP(tp->clone());
    sys->setEV(ev1);
    sys->run(stk, query);
    CHECK_NE(sys->getTM()->currentCash(), init_cash);
    tr_list = sys->getTM()->getTradeList();
    CHECK_EQ(tr_list.size(), 3);
    CHECK_EQ(tr_list[0].business, BUSINESS_INIT);

    CHECK_EQ(tr_list[1].stock, stk);
    CHECK_EQ(tr_list[1].datetime, Datetime(200001060000LL));
    CHECK_EQ(tr_list[1].business, BUSINESS_BUY);
    CHECK_LT(std::fabs(tr_list[1].planPrice - 25.18), 0.00001);
    CHECK_LT(std::fabs(tr_list[1].realPrice - 25.18), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[1].goalPrice));
    CHECK_EQ(tr_list[1].number, 100);
    CHECK_LT(std::fabs(tr_list[1].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[1].stoploss - 24.93), 0.00001);
    current_cash = init_cash - 2518;
    CHECK_LT(std::fabs(tr_list[1].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[1].from, PART_SIGNAL);

    CHECK_EQ(tr_list[2].stock, stk);
    CHECK_EQ(tr_list[2].datetime, Datetime(200001110000LL));
    CHECK_EQ(tr_list[2].business, BUSINESS_SELL);
    CHECK_LT(std::fabs(tr_list[2].planPrice - 27.25), 0.00001);
    CHECK_LT(std::fabs(tr_list[2].realPrice - 27.25), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[2].goalPrice));
    CHECK_EQ(tr_list[2].number, 100);
    CHECK_LT(std::fabs(tr_list[2].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[2].stoploss - 26.98), 0.00001);
    current_cash += 2725;
    CHECK_LT(std::fabs(tr_list[2].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[2].from, PART_ENVIRONMENT);

    /** @arg TM, SG, MM, ST, TP and EV are given (position building), the others are not, delayed */
    sys = SYS_Simple();
    sys->setParam<bool>("buy_delay", true);
    sys->setParam<bool>("sell_delay", true);
    sys->setParam<bool>("ev_open_position", true);
    sys->setTM(tm->clone());
    sys->setSG(sg->clone());
    sys->setMM(mm->clone());
    sys->setST(st->clone());
    sys->setTP(tp->clone());
    sys->setEV(ev1);
    sys->run(stk, query);
    CHECK_NE(sys->getTM()->currentCash(), init_cash);
    tr_list = sys->getTM()->getTradeList();
    CHECK_EQ(tr_list.size(), 4);
    CHECK_EQ(tr_list[0].business, BUSINESS_INIT);

    CHECK_EQ(tr_list[1].stock, stk);
    CHECK_EQ(tr_list[1].datetime, Datetime(200001050000LL));
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
    CHECK_EQ(tr_list[2].datetime, Datetime(200001060000LL));
    CHECK_EQ(tr_list[2].business, BUSINESS_BUY);
    CHECK_LT(std::fabs(tr_list[2].planPrice - 25.18), 0.00001);
    CHECK_LT(std::fabs(tr_list[2].realPrice - 25.18), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[2].goalPrice));
    CHECK_EQ(tr_list[2].number, 100);
    CHECK_LT(std::fabs(tr_list[2].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[2].stoploss - 24.93), 0.00001);
    current_cash -= 2518;
    CHECK_LT(std::fabs(tr_list[2].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[2].from, PART_SIGNAL);

    CHECK_EQ(tr_list[3].stock, stk);
    CHECK_EQ(tr_list[3].datetime, Datetime(200001110000LL));
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

    /** @arg
     * TM, SG, MM and EV are given (covering one buy / sell pair exactly, no position building) but
     * the others are not, a non-delayed operation
     */
    sys = SYS_Simple();
    sys->setParam<bool>("buy_delay", false);
    sys->setParam<bool>("sell_delay", false);
    sys->setParam<bool>("ev_open_position", false);
    sys->setTM(tm->clone());
    sys->setSG(sg->clone());
    sys->setMM(mm->clone());
    sys->setEV(ev2);
    sys->run(stk, query);
    CHECK_NE(sys->getTM()->currentCash(), init_cash);
    tr_list = sys->getTM()->getTradeList();
    CHECK_EQ(tr_list.size(), 3);
    CHECK_EQ(tr_list[0].business, BUSINESS_INIT);

    CHECK_EQ(tr_list[1].stock, stk);
    CHECK_EQ(tr_list[1].datetime, Datetime(200001050000LL));
    CHECK_EQ(tr_list[1].business, BUSINESS_BUY);
    CHECK_LT(std::fabs(tr_list[1].planPrice - 25.28), 0.00001);
    CHECK_LT(std::fabs(tr_list[1].realPrice - 25.28), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[1].goalPrice));
    CHECK_EQ(tr_list[1].number, 100);
    CHECK_LT(std::fabs(tr_list[1].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[1].stoploss - 0), 0.00001);
    current_cash = init_cash - 2528;
    CHECK_LT(std::fabs(tr_list[1].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[1].from, PART_SIGNAL);

    CHECK_EQ(tr_list[2].stock, stk);
    CHECK_EQ(tr_list[2].datetime, Datetime(200001140000LL));
    CHECK_EQ(tr_list[2].business, BUSINESS_SELL);
    CHECK_LT(std::fabs(tr_list[2].planPrice - 24.20), 0.00001);
    CHECK_LT(std::fabs(tr_list[2].realPrice - 24.20), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[2].goalPrice));
    CHECK_EQ(tr_list[2].number, 100);
    CHECK_LT(std::fabs(tr_list[2].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[2].stoploss - 0), 0.00001);
    current_cash += 2420;
    CHECK_LT(std::fabs(tr_list[2].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[2].from, PART_SIGNAL);

    /** @arg
     * TM, SG, MM and EV are given (covering one buy / sell pair exactly, position building) but the
     * others are not, a non-delayed operation
     */
    sys = SYS_Simple();
    sys->setParam<bool>("buy_delay", false);
    sys->setParam<bool>("sell_delay", false);
    sys->setParam<bool>("ev_open_position", true);
    sys->setTM(tm->clone());
    sys->setSG(sg->clone());
    sys->setMM(mm->clone());
    sys->setEV(ev2);
    sys->run(stk, query);
    CHECK_NE(sys->getTM()->currentCash(), init_cash);
    tr_list = sys->getTM()->getTradeList();
    CHECK_EQ(tr_list.size(), 3);
    CHECK_EQ(tr_list[0].business, BUSINESS_INIT);

    CHECK_EQ(tr_list[1].stock, stk);
    CHECK_EQ(tr_list[1].datetime, Datetime(200001050000LL));
    CHECK_EQ(tr_list[1].business, BUSINESS_BUY);
    CHECK_LT(std::fabs(tr_list[1].planPrice - 25.28), 0.00001);
    CHECK_LT(std::fabs(tr_list[1].realPrice - 25.28), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[1].goalPrice));
    CHECK_EQ(tr_list[1].number, 100);
    CHECK_LT(std::fabs(tr_list[1].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[1].stoploss - 0), 0.00001);
    current_cash = init_cash - 2528;
    CHECK_LT(std::fabs(tr_list[1].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[1].from, PART_ENVIRONMENT);

    CHECK_EQ(tr_list[2].stock, stk);
    CHECK_EQ(tr_list[2].datetime, Datetime(200001140000LL));
    CHECK_EQ(tr_list[2].business, BUSINESS_SELL);
    CHECK_LT(std::fabs(tr_list[2].planPrice - 24.20), 0.00001);
    CHECK_LT(std::fabs(tr_list[2].realPrice - 24.20), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[2].goalPrice));
    CHECK_EQ(tr_list[2].number, 100);
    CHECK_LT(std::fabs(tr_list[2].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[2].stoploss - 0), 0.00001);
    current_cash += 2420;
    CHECK_LT(std::fabs(tr_list[2].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[2].from, PART_SIGNAL);

    /** @arg TM, SG, MM and EV are given (position building); the EV start date is the buy date */
    sys = SYS_Simple();
    sys->setParam<bool>("buy_delay", false);
    sys->setParam<bool>("sell_delay", false);
    sys->setParam<bool>("ev_open_position", true);
    sys->setTM(tm->clone());
    sys->setSG(sg->clone());
    sys->setMM(mm->clone());
    sys->setEV(ev3);

    sys->run(stk, query);
    CHECK_NE(sys->getTM()->currentCash(), init_cash);
    tr_list = sys->getTM()->getTradeList();
    CHECK_EQ(tr_list.size(), 3);
    CHECK_EQ(tr_list[0].business, BUSINESS_INIT);

    CHECK_EQ(tr_list[1].stock, stk);
    CHECK_EQ(tr_list[1].datetime, Datetime(200001050000LL));
    CHECK_EQ(tr_list[1].business, BUSINESS_BUY);
    CHECK_LT(std::fabs(tr_list[1].planPrice - 25.28), 0.00001);
    CHECK_LT(std::fabs(tr_list[1].realPrice - 25.28), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[1].goalPrice));
    CHECK_EQ(tr_list[1].number, 100);
    CHECK_LT(std::fabs(tr_list[1].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[1].stoploss - 0), 0.00001);
    current_cash = init_cash - 2528;
    CHECK_LT(std::fabs(tr_list[1].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[1].from, PART_ENVIRONMENT);

    CHECK_EQ(tr_list[2].stock, stk);
    CHECK_EQ(tr_list[2].datetime, Datetime(200001070000LL));
    CHECK_EQ(tr_list[2].business, BUSINESS_SELL);
    CHECK_LT(std::fabs(tr_list[2].planPrice - 26.90), 0.00001);
    CHECK_LT(std::fabs(tr_list[2].realPrice - 26.90), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[2].goalPrice));
    CHECK_EQ(tr_list[2].number, 100);
    CHECK_LT(std::fabs(tr_list[2].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[2].stoploss - 0), 0.00001);
    current_cash += 2690;
    CHECK_LT(std::fabs(tr_list[2].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[2].from, PART_ENVIRONMENT);

    /*for (auto iter = tr_list.begin(); iter , tr_list.end(); ++iter) {
        std::cout << *iter << std::endl;
    }*/
}

//-----------------------------------------------------------------------------
// The invalid→valid transition open must not misfire on a run(reset=false) re-entry
//-----------------------------------------------------------------------------

/** @par Check points (the ev_open_position transition on a run(reset=false) re-entry):
 *  the initial position is built once; the re-entry run must not buy again */
TEST_CASE("test_SYS_Simple_ev_open_position_no_reopen_on_rerun") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm["sh600000"];
    KQuery query = KQueryByDate(Datetime(199911100000LL), Datetime(200002250000LL), KQuery::DAY);
    KData k = stk.getKData(query);
    REQUIRE_GE(k.size(), 10);

    // The EV is invalid on the first bar only, valid on every bar afterwards
    DatetimeList valid(k.size() - 1);
    for (size_t i = 1; i < k.size(); ++i) {
        valid[i - 1] = k[i].datetime;
    }

    // The re-entry window starts well after the initial position was built
    KQuery query2 = KQueryByDate(k[8].datetime, Datetime(200002250000LL), KQuery::DAY);
    KData k2 = stk.getKData(query2);
    REQUIRE_GE(k2.size(), 5);

    auto sys = SYS_Simple();
    sys->setParam<bool>("ev_open_position", true);
    // buy_delay default true: the transition buy fills long before the re-entry window
    sys->setTM(crtTM(Datetime(199001010000LL), 100000, TC_Zero(), "TEST_TM"));
    sys->setSG(make_shared<NoSignalSG>());
    sys->setMM(MM_FixedCount(100));
    EVPtr ev = make_shared<RangeEV>(valid);
    ev->setQuery(query);
    sys->setEV(ev);

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

/** @par Check points (the *_open_position parameters without the matching part): the transition
 *  flag alone must not build a position when no EV or CN is attached */
TEST_CASE("test_SYS_Simple_open_position_without_part_no_build") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm["sh600000"];
    KQuery query = KQueryByDate(Datetime(199911100000LL), Datetime(200002250000LL), KQuery::DAY);

    auto sys = SYS_Simple();
    sys->setParam<bool>("ev_open_position", true);
    sys->setParam<bool>("cn_open_position", true);
    sys->setTM(crtTM(Datetime(199001010000LL), 100000, TC_Zero(), "TEST_TM"));
    sys->setSG(make_shared<NoSignalSG>());
    sys->setMM(MM_FixedCount(100));

    // An incremental run on a fresh system: the pre-flags are still the constructor defaults
    sys->run(stk, query, false, false);

    /** @arg no EV/CN attached: the first bar must not count as an invalid→valid transition */
    CHECK_EQ(countBusiness(sys->getTM()->getTradeList(), BUSINESS_BUY), 0);
}

/** @} */
