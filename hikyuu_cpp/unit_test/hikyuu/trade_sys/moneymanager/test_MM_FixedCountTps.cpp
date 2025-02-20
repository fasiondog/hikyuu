/*
 * test_MM_FixedCountTps.cpp
 *
 *  Created on: 2013-4-19
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_manage/crt/TC_FixedA.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedCountTps.h>

using namespace hku;

/**
 * @defgroup test_MM_FixedCountTps test_MM_FixedCountTps
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_MM_FixedCountTpsTps") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeManagerPtr tm = crtTM(Datetime(199001010000LL), 0.0, TC_FixedA());

    /** @arg 买入/卖出数量中含有小于0的值 */
    CHECK_THROWS_AS(MM_FixedCountTps({100., -1}, {100., 100.}), std::exception);
    CHECK_THROWS_AS(MM_FixedCountTps({100., 100.}, {100., -100.}), std::exception);

    /** @arg 买入卖出总量不等（仅打印警告）*/
    auto _ = MM_FixedCountTps({100., 100.}, {200., 200.});

    /** @arg n = 100, 一个初始资金为0的交易账户自动补充资金，能够执行买入操作 */
    tm = crtTM(Datetime(199001010000LL), 0.0, TC_FixedA());
    CHECK_EQ(tm->initCash(), 0.0);
    auto mm = MM_FixedCountTps({100., 200.}, {200., 100.});
    mm->setTM(tm);
    mm->setParam<bool>("auto-checkin", true);
    CHECK_EQ(mm->currentBuyCount(stock), 0);
    double buy_num = mm->getBuyNumber(Datetime(200001200000), stock, 24.11, 24.11, PART_SIGNAL);
    CHECK_EQ(buy_num, 100);
    CHECK_EQ(tm->cash(Datetime(200001200000)), 2417.02);
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

/** @} */
