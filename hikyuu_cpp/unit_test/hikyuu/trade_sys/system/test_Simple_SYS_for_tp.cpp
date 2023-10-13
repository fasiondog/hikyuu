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

/** @par 检测点（止盈策略）  */
TEST_CASE("test_SYS_Simple_for_tp") {
    TradeRecordList tr_list;
    price_t current_cash;

    StockManager& sm = StockManager::instance();

    // 初始参数
    Datetime init_date(199001010000LL);   // 账户初始日期
    price_t init_cash = 100000;           // 账户初始金额
    TradeCostPtr costfunc = TC_Zero();    // 零成本函数
    Stock stk = sm["sh600000"];           // 选定标的
    Datetime start_date(199911100000LL);  // 测试起始日期
    Datetime end_date(200002250000LL);    // 测试结束日期

    KQuery query = KQueryByDate(start_date, end_date, KQuery::DAY);

    // 构建系统部件
    TMPtr tm = crtTM(init_date, init_cash, costfunc, "TEST_TM");
    SGPtr sg = SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10));
    MMPtr mm = MM_FixedCount(100);
    STPtr st = ST_FixedPercent(0.01);
    TPPtr tp = ST_Indicator(MA(CLOSE(), 5));
    SYSPtr sys;

    /** @arg 指定了TM、SG、MM、ST、TP，但未指定其他策略组件，非延迟操作 */
    sys = SYS_Simple();
    sys->setParam("buy_delay", false);
    sys->setParam("sell_delay", false);
    sys->setTM(tm->clone());
    sys->setSG(sg->clone());
    sys->setMM(mm->clone());
    sys->setST(st->clone());
    sys->setTP(tp->clone());
    CHECK_EQ(sys->readyForRun(), true);
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
    CHECK_LT(std::fabs(tr_list[1].stoploss - 26.19), 0.00001);
    current_cash = init_cash - 2645;
    CHECK_LT(std::fabs(tr_list[1].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[1].from, PART_SIGNAL);

    CHECK_EQ(tr_list[2].stock, stk);
    // CHECK_EQ(tr_list[2].datetime, Datetime(199912160000LL));
    // CHECK_EQ(tr_list[2].business, BUSINESS_SELL);
    // CHECK_LT(std::fabs(tr_list[2].planPrice - 26.00), 0.00001);
    // CHECK_LT(std::fabs(tr_list[2].realPrice - 26.00), 0.00001);
    // CHECK_UNARY(std::isnan(tr_list[2].goalPrice));
    // CHECK_EQ(tr_list[2].number, 100);
    // CHECK_LT(std::fabs(tr_list[2].cost.total - 0), 0.00001);
    // CHECK_LT(std::fabs(tr_list[2].stoploss - 25.74), 0.00001);
    // current_cash += 2600;
    // CHECK_LT(std::fabs(tr_list[2].cash - current_cash), 0.00001);
    // CHECK_EQ(tr_list[2].from, PART_STOPLOSS);

    // CHECK_EQ(tr_list[3].stock, stk);
    // CHECK_EQ(tr_list[3].datetime, Datetime(200001050000LL));
    // CHECK_EQ(tr_list[3].business, BUSINESS_BUY);
    // CHECK_LT(std::fabs(tr_list[3].planPrice - 25.28), 0.00001);
    // CHECK_LT(std::fabs(tr_list[3].realPrice - 25.28), 0.00001);
    // CHECK_UNARY(std::isnan(tr_list[3].goalPrice));
    // CHECK_EQ(tr_list[3].number, 100);
    // CHECK_LT(std::fabs(tr_list[3].cost.total - 0), 0.00001);
    // CHECK_LT(std::fabs(tr_list[3].stoploss - 25.03), 0.00001);
    // current_cash -= 2528;
    // CHECK_LT(std::fabs(tr_list[3].cash - current_cash), 0.00001);
    // CHECK_EQ(tr_list[3].from, PART_SIGNAL);

    // CHECK_EQ(tr_list[4].stock, stk);
    // CHECK_EQ(tr_list[4].datetime, Datetime(200001110000LL));
    // CHECK_EQ(tr_list[4].business, BUSINESS_SELL);
    // CHECK_LT(std::fabs(tr_list[4].planPrice - 26.20), 0.00001);
    // CHECK_LT(std::fabs(tr_list[4].realPrice - 26.20), 0.00001);
    // CHECK_UNARY(std::isnan(tr_list[4].goalPrice));
    // CHECK_EQ(tr_list[4].number, 100);
    // CHECK_LT(std::fabs(tr_list[4].cost.total - 0), 0.00001);
    // CHECK_LT(std::fabs(tr_list[4].stoploss - 25.94), 0.00001);
    // current_cash += 2620;
    // CHECK_LT(std::fabs(tr_list[4].cash - current_cash), 0.00001);
    // CHECK_EQ(tr_list[4].from, PART_TAKEPROFIT);

    /** @arg 指定了TM、SG、MM、ST、TP，但未指定其他策略组件，延迟操作 */
    sys = SYS_Simple();
    sys->setParam("buy_delay", true);
    sys->setParam("sell_delay", true);
    sys->setTM(tm->clone());
    sys->setSG(sg->clone());
    sys->setMM(mm->clone());
    sys->setST(st->clone());
    sys->setTP(tp->clone());
    CHECK_EQ(sys->readyForRun(), true);
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
    CHECK_LT(std::fabs(tr_list[1].stoploss - 26.24), 0.00001);
    current_cash = init_cash - 2650;
    CHECK_LT(std::fabs(tr_list[1].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[1].from, PART_SIGNAL);

    CHECK_EQ(tr_list[2].stock, stk);
    // CHECK_EQ(tr_list[2].datetime, Datetime(199912170000LL));
    // CHECK_EQ(tr_list[2].business, BUSINESS_SELL);
    // CHECK_LT(std::fabs(tr_list[2].planPrice - 26.00), 0.00001);
    // CHECK_LT(std::fabs(tr_list[2].realPrice - 26.00), 0.00001);
    // CHECK_UNARY(std::isnan(tr_list[2].goalPrice));
    // CHECK_EQ(tr_list[2].number, 100);
    // CHECK_LT(std::fabs(tr_list[2].cost.total - 0), 0.00001);
    // CHECK_LT(std::fabs(tr_list[2].stoploss - 25.74), 0.00001);
    // current_cash += 2600;
    // CHECK_LT(std::fabs(tr_list[2].cash - current_cash), 0.00001);
    // CHECK_EQ(tr_list[2].from, PART_STOPLOSS);

    // CHECK_EQ(tr_list[3].stock, stk);
    // CHECK_EQ(tr_list[3].datetime, Datetime(200001060000LL));
    // CHECK_EQ(tr_list[3].business, BUSINESS_BUY);
    // CHECK_LT(std::fabs(tr_list[3].planPrice - 25.18), 0.00001);
    // CHECK_LT(std::fabs(tr_list[3].realPrice - 25.18), 0.00001);
    // CHECK_UNARY(std::isnan(tr_list[3].goalPrice));
    // CHECK_EQ(tr_list[3].number, 100);
    // CHECK_LT(std::fabs(tr_list[3].cost.total - 0), 0.00001);
    // CHECK_LT(std::fabs(tr_list[3].stoploss - 24.93), 0.00001);
    // current_cash -= 2518;
    // CHECK_LT(std::fabs(tr_list[3].cash - current_cash), 0.00001);
    // CHECK_EQ(tr_list[3].from, PART_SIGNAL);

    // CHECK_EQ(tr_list[4].stock, stk);
    // CHECK_EQ(tr_list[4].datetime, Datetime(200001120000LL));
    // CHECK_EQ(tr_list[4].business, BUSINESS_SELL);
    // CHECK_LT(std::fabs(tr_list[4].planPrice - 26.00), 0.00001);
    // CHECK_LT(std::fabs(tr_list[4].realPrice - 26.00), 0.00001);
    // CHECK_UNARY(std::isnan(tr_list[4].goalPrice));
    // CHECK_EQ(tr_list[4].number, 100);
    // CHECK_LT(std::fabs(tr_list[4].cost.total - 0), 0.00001);
    // CHECK_LT(std::fabs(tr_list[4].stoploss - 25.74), 0.00001);
    // current_cash += 2600;
    // CHECK_LT(std::fabs(tr_list[4].cash - current_cash), 0.00001);
    // CHECK_EQ(tr_list[4].from, PART_TAKEPROFIT);
}

/** @} */
