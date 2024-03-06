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

/** @par 检测点（基本操作）  */
TEST_CASE("test_SYS_Simple_for_base") {
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
    SYSPtr sys;

    /** @arg 未指定账户运行    */
    sys = SYS_Simple();
    CHECK_EQ(sys->readyForRun(), false);
    sys->run(stk, query);
    CHECK_UNARY(!sys->getTM());

    /** @arg 指定了账户，但未指定其他策略组件 */
    sys = SYS_Simple();
    sys->setTM(tm->clone());
    CHECK_EQ(sys->readyForRun(), false);
    sys->run(stk, query);
    CHECK_EQ(sys->getTM()->currentCash(), init_cash);
    tr_list = sys->getTM()->getTradeList();
    CHECK_EQ(tr_list.size(), 1);
    CHECK_EQ(tr_list[0].business, BUSINESS_INIT);

    /** @arg 指定了TM和SG，但未指定其他策略组件 */
    sys = SYS_Simple();
    sys->setTM(tm->clone());
    sys->setSG(sg->clone());
    CHECK_EQ(sys->readyForRun(), false);
    sys->run(stk, query);
    CHECK_EQ(sys->getTM()->currentCash(), init_cash);
    tr_list = sys->getTM()->getTradeList();
    CHECK_EQ(tr_list.size(), 1);
    CHECK_EQ(tr_list[0].business, BUSINESS_INIT);

    /** @arg 指定了TM、SG、MM，但未指定其他策略组件，非延迟操作 */
    sys = SYS_Simple();
    sys->setParam("buy_delay", false);
    sys->setParam("sell_delay", false);
    sys->setTM(tm->clone());
    sys->setSG(sg->clone());
    sys->setMM(mm->clone());
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

    /** @arg 指定了TM、SG、MM，但未指定其他策略组件，延迟操作 */
    sys = SYS_Simple();
    sys->setParam("buy_delay", true);
    sys->setParam("sell_delay", true);
    sys->setTM(tm->clone());
    sys->setSG(sg->clone());
    sys->setMM(mm->clone());
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

/** @} */
