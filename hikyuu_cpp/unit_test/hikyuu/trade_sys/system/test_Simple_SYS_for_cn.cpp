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

/** @par 检测点（系统有效性策略）  */
TEST_CASE("test_SYS_Simple_for_cn") {
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

    EVPtr ev1 = make_shared<TestEV1>();
    ev1->setQuery(query);

    EVPtr ev2 = make_shared<TestEV2>();
    ev2->setQuery(query);

    EVPtr ev3 = make_shared<TestEV3>();
    ev3->setQuery(query);

    CNPtr cn1 = make_shared<TestCN1>();
    CNPtr cn2 = make_shared<TestCN2>();
    CNPtr cn3 = make_shared<TestCN3>();

    /** @arg 指定了TM、SG、MM、ST、TP、EV、CN，CN有效日期和EV重合 */
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

    CHECK_EQ(sys->readyForRun(), true);
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

    /** @arg 指定了TM、SG、MM、ST、TP、EV、CN，CN的有效日期范围完全覆盖并大于EV的范围 */
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

    CHECK_EQ(sys->readyForRun(), true);
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

    /** @arg 指定了TM、SG、MM、EV、CN（不触发建仓），CN的有效日期范围在EV的范围之内 */
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

    CHECK_EQ(sys->readyForRun(), true);
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

    /** @arg 指定了TM、SG、MM、EV、CN（触发建仓），CN的有效日期范围在EV的范围之内 */
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

    CHECK_EQ(sys->readyForRun(), true);
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

/** @} */
