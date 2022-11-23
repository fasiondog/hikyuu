/*
 * test_TradeManager.cpp
 *
 *  Created on: 2013-2-21
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/crt/TC_TestStub.h>
#include <hikyuu/trade_manage/crt/TC_FixedA.h>
#include <hikyuu/trade_manage/crt/crtTM.h>

#include <fstream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

using namespace hku;

/**
 * @defgroup test_TradeManager test_TradeManager
 * @ingroup test_hikyuu_trade_manage_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_TradeManager_init") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    CostRecord result, expect;
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000,
                               TC_FixedA(0.0018, 5, 0.001, 0.001, 1.0), MRPtr(), "TEST");

    CHECK_EQ(tm->name(), "TEST");
    CHECK_EQ(tm->initCash(), 100000.0);
    // CHECK_EQ(tm->cash(), 100000.0);
    CHECK_EQ(tm->initDatetime(), Datetime(199901010000));
    CHECK_EQ(tm->firstDatetime(), Null<Datetime>());
    CHECK_EQ(tm->lastDatetime(), Datetime(199901010000));
    CHECK_EQ(tm->have(stock), false);
    CHECK_EQ(tm->getStockNumber(), 0);
    CHECK_EQ(tm->getHoldNumber(Datetime(199901010000), stock), 0);
    const TradeRecordList& tradeList = tm->getTradeList();
    CHECK_EQ(tradeList.size(), 1);
    CHECK_EQ(tradeList[0],
             TradeRecord(Null<Stock>(), Datetime(199901010000), BUSINESS_INIT, 100000.0, 100000.0,
                         0.0, 0, Null<CostRecord>(), 0.0, 100000.0, 1.0, PART_INVALID));
    CHECK_EQ(tm->getPositionList().empty(), true);
    CHECK_EQ(tm->getPosition(Datetime(199901010000), stock), Null<PositionRecord>());
}

/** @par 检测点 */
TEST_CASE("test_TradeManager_getBuyCost") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    CostRecord result, expect;
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000,
                               TC_FixedA(0.0018, 5, 0.001, 0.001, 1.0), MRPtr(), "TEST");

    /** @arg 调用CostFunc是否正常 */
    result = tm->getBuyCost(Datetime(200101010000), stock, 10.0, 1000);
    expect.commission = 18.0;
    expect.stamptax = 0.0;
    expect.transferfee = 1.0;
    expect.total = 19.0;
    CHECK_EQ(result, expect);
}

/** @par 检测点 */
TEST_CASE("test_TradeManager_getSellCost") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600004");
    CostRecord result, expect;
    TradeManagerPtr tm =
      crtTM(Datetime(199901010000), 100000, TC_FixedA(0.0018, 5, 0.001, 0.001, 1.0));

    /** @arg 调用CostFunc是否正常 */
    result = tm->getSellCost(Datetime(200101010000), stock, 10.0, 100);
    expect.commission = 5.0;
    expect.stamptax = 1.0;
    expect.transferfee = 1.0;
    expect.total = 7.0;
    CHECK_EQ(result, expect);
}

/** @par 检测点 */
TEST_CASE("test_TradeManager_getMarginRatio") {
    Stock stock = getStock("sh600004");
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000, TC_Zero());

    // 默认未指定获取保证金算法
    CHECK_EQ(tm->getMarginRatio(Datetime(200001020000), stock), 1.0);

    // 指定保证金算法
    tm->marginRatioFunc(MR_Fixed(0.02));
    CHECK_EQ(tm->getMarginRatio(Datetime(200001020000), stock), 0.02);
}

/** @par 检测点 */
TEST_CASE("test_TradeManager_can_not_buy") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeCostPtr costfunc = TC_FixedA(0.0018, 5, 0.001, 0.001, 1.0);
    TradeManagerPtr tm;
    TradeRecord result;
    CostRecord cost;
    TradeRecord trade;
    TradeRecordList trade_list;

    /** @arg 账户初始余额为0，未进行过交易，忽略权息信息 */
    tm = crtTM(Datetime(199901010000), 0, costfunc, MRPtr(), "SYS");
    result = tm->buy(Datetime(199911180000), stock, 27.2, 100, 0, 27.2, 27.2);
    CHECK_EQ(result, Null<TradeRecord>());

    /** @arg 账户初始余额为100000，试图对Null<Stock>进行操作 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, MRPtr(), "SYS");
    result = tm->buy(Datetime(199911180000), Null<Stock>(), 27.2, 100, 0, 27.2, 27.2);
    CHECK_EQ(result, Null<TradeRecord>());
    CHECK_EQ(tm->cash(Datetime(199911180000)), 100000);

    /** @arg 试图在初始建仓日之前买入 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, MRPtr(), "SYS");
    result = tm->buy(Datetime(199001010000), stock, 26.36, 100, 0, 26.36, 26.36);
    CHECK_EQ(result, Null<TradeRecord>());

#if 0  //取消了该限制
    /** @arg 账户初始余额为100000，未进行过交易，忽略权息信息，但指定日期该证券不能进行交易，如非交易日 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, MRPtr(), "SYS");
    result = tm->buy(Datetime(199911130000), stock, 27.2, 100, 0, 27.2, 27.2);
    CHECK_EQ(result, Null<TradeRecord>());

    /** @arg 账户初始余额为100000，未进行过交易，忽略权息信息，但买入价格超出当日最高价 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, MRPtr(), "SYS");
    result = tm->buy(Datetime(199911170000), stock, 27.2, 100, 0, 27.2, 27.2);
    CHECK_EQ(result, Null<TradeRecord>());
#endif

    /** @arg 账户初始余额为100000，未进行过交易，忽略权息信息，但买入价格等于当日最高价 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, MRPtr(), "SYS");
    result = tm->buy(Datetime(199911170000), stock, 27.18, 100, 0, 27.18, 27.18);
    cost = tm->getBuyCost(Datetime(199911170000), stock, 27.18, 100);
    trade = TradeRecord(stock, Datetime(199911170000), BUSINESS_BUY, 27.18, 27.18, 27.18, 100, cost,
                        0.0, 100000 - cost.total - 27.18 * 100, 1.0, PART_INVALID);
    CHECK_EQ(result, trade);
    CHECK_EQ(tm->cash(Datetime(199911170000)), trade.cash);
    trade_list = tm->getTradeList();
    CHECK_EQ(trade_list.size(), 2);
    CHECK_EQ(trade_list[1], trade);

#if 0
    /** @arg 账户初始余额为100000，未进行过交易，忽略权息信息，但买入价格低于当日最低价 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, MRPtr(), "SYS");
    result = tm->buy(Datetime(199911170000), stock, 26.36, 100, 0, 26.36, 26.36);
    CHECK_EQ(result, Null<TradeRecord>());
#endif

    /** @arg 账户初始余额为100000，未进行过交易，忽略权息信息，但买入价格等于当日最低价 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, MRPtr(), "SYS");
    result = tm->buy(Datetime(199911160000), stock, 26.48, 100, 0, 26.48, 26.48);
    cost = tm->getBuyCost(Datetime(199911160000), stock, 26.48, 100);
    CHECK_EQ(result,
             TradeRecord(stock, Datetime(199911160000), BUSINESS_BUY, 26.48, 26.48, 26.48, 100,
                         cost, 0.0, 100000 - cost.total - 26.48 * 100, 1.0, PART_INVALID));

    /** @arg 试图在最后一笔交易时间之前进行交易 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, MRPtr(), "SYS");
    result = tm->buy(Datetime(199911170000), stock, 26.48, 100, 0, 26.48, 26.48);
    CHECK_UNARY(!(result == Null<TradeRecord>()));
    result = tm->buy(Datetime(199911160000), stock, 26.48, 100, 0, 26.48, 26.48);
    CHECK_EQ(result, Null<TradeRecord>());

    /** @arg 试图买入数量为0的股票 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, MRPtr(), "SYS");
    result = tm->buy(Datetime(199911170000), stock, 26.48, 0, 0, 26.48, 26.48);
    CHECK_EQ(result, Null<TradeRecord>());

    /** @arg 买入数量小于该股票的最小交易量 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, MRPtr(), "SYS");
    result =
      tm->buy(Datetime(199911170000), stock, 26.48, stock.minTradeNumber() - 1, 0, 26.48, 26.48);
    CHECK_EQ(result, Null<TradeRecord>());

    /** @arg 买入数量大于该股票的最大交易量 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, MRPtr(), "SYS");
    result =
      tm->buy(Datetime(199911170000), stock, 26.48, stock.maxTradeNumber() - 1, 0, 26.48, 26.48);
    CHECK_EQ(result, Null<TradeRecord>());
}

/** @par 检测点 */
TEST_CASE("test_TradeManager_can_not_sell") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeCostPtr costfunc = TC_FixedA(0.0018, 5, 0.001, 0.001, 1.0);
    TradeManagerPtr tm;
    TradeRecord result;
    CostRecord cost;
    TradeRecord trade;
    TradeRecordList trade_list;

    /** @arg 账户初始余额为0，未进行过交易，忽略权息信息 */
    tm = crtTM(Datetime(199901010000), 0, costfunc, MRPtr(), "SYS");
    result = tm->sell(Datetime(199911180000), stock, 27.2, 100);
    CHECK_EQ(result, Null<TradeRecord>());

    /** @arg 账户初始余额为100000，试图对Null<Stock>进行操作 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, MRPtr(), "SYS");
    result = tm->sell(Datetime(199911180000), Null<Stock>(), 27.2, 100);
    CHECK_EQ(result, Null<TradeRecord>());
    CHECK_EQ(tm->cash(Datetime(199911180000)), 100000);

    /** @arg 试图在最后一个交易日之前卖出 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, MRPtr(), "SYS");
    result = tm->buy(Datetime(199911170000), stock, 27.18, 100, 0);
    CHECK_EQ(tm->getHoldNumber(Datetime(199911170000), stock), 100);
    result = tm->sell(Datetime(199801010000), stock, 26.36, 100);
    CHECK_EQ(result, Null<TradeRecord>());

    /** @arg 卖出的数量等于0 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, MRPtr(), "SYS");
    result = tm->buy(Datetime(199911170000), stock, 27.18, 100, 0);
    CHECK_EQ(tm->getHoldNumber(Datetime(199911170000), stock), 100);
    result = tm->sell(Datetime(199911180000), stock, 26.36, 0);
    CHECK_EQ(result, Null<TradeRecord>());

    /** @arg 卖出的数量小于最小交易数量 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, MRPtr(), "SYS");
    result = tm->buy(Datetime(199911170000), stock, 27.18, 100, 0);
    CHECK_EQ(tm->getHoldNumber(Datetime(199911170000), stock), 100);
    result = tm->sell(Datetime(199911180000), stock, 26.36, stock.minTradeNumber() - 1);
    CHECK_EQ(result, Null<TradeRecord>());

    /** @arg 卖出的数量大于最大交易数量 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, MRPtr(), "SYS");
    result = tm->buy(Datetime(199911170000), stock, 27.18, 100, 0);
    CHECK_EQ(tm->getHoldNumber(Datetime(199911170000), stock), 100);
    result = tm->sell(Datetime(199911180000), stock, 26.36, stock.maxTradeNumber() + 1);
    CHECK_EQ(result, Null<TradeRecord>());

    /** @arg 卖出未持仓的股票 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, MRPtr(), "SYS");
    result = tm->sell(Datetime(199901020000), stock, 26.36, 100);
    CHECK_EQ(result, Null<TradeRecord>());

    /** @arg 卖出的数量大于当前持仓数量 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, MRPtr(), "SYS");
    result = tm->buy(Datetime(199911170000), stock, 27.18, 100, 0);
    CHECK_EQ(tm->getHoldNumber(Datetime(199911170000), stock), 100);
    result = tm->sell(Datetime(199911180000), stock, 26.36, 101);
    CHECK_EQ(result, Null<TradeRecord>());

    /** @arg 忽略权息信息，将买入股票全部卖出 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, MRPtr(), "SYS");
    result = tm->buy(Datetime(199911170000), stock, 27.18, 100, 0, 27.18, 27.18);
    cost = tm->getBuyCost(Datetime(199911170000), stock, 27.18, 100);
    CHECK_EQ(result,
             TradeRecord(stock, Datetime(199911170000), BUSINESS_BUY, 27.18, 27.18, 27.18, 100,
                         cost, 0.0, 100000 - cost.total - 27.18 * 100, 1.0, PART_INVALID));
    CHECK_EQ(tm->getHoldNumber(Datetime(199911170000), stock), 100);
    CHECK_EQ(tm->getStockNumber(), 1);
    CHECK_EQ(tm->cash(Datetime(199911170000)), 97276.0);
    result = tm->sell(Datetime(199911180000), stock, 26.36, std::numeric_limits<double>::max(), 0,
                      0, 26.36);
    cost = tm->getSellCost(Datetime(199911180000), stock, 26.36, 100);
    CHECK_EQ(result, TradeRecord(stock, Datetime(199911180000), BUSINESS_SELL, 26.36, 26.36, 0.0,
                                 100, cost, 0.0, 99903.36, 1.0, PART_INVALID));
    CHECK_EQ(tm->getHoldNumber(Datetime(199911180000), stock), 0);
    CHECK_EQ(tm->getStockNumber(), 0);
    CHECK_EQ(tm->cash(Datetime(199911180000)), 99903.36);

    /** @arg 不忽略权息信息，对股票进行买卖操作，忽略买卖成本 */
    tm = crtTM(Datetime(199901010000), 1000000, TC_Zero(), MRPtr(), "SYS");
    tm->buy(Datetime(199911170000), stock, 27.18, 1000, 0);
    CHECK_EQ(tm->cash(Datetime(199911170000)), 972820);
    tm->sell(Datetime(200605150000), stock, 10.2, 100);
    CHECK_EQ(tm->cash(Datetime(200605150000)), 974685);  // 973840);
    CHECK_EQ(tm->getHoldNumber(Datetime(200605160000), stock), 1850);
    tm->sell(Datetime(200612010000), stock, 16.87, std::numeric_limits<double>::max());
    CHECK_EQ(tm->cash(Datetime(200612010000)), 1006135.0);  // 1005049.5);
}

/** @par 检测点 */
TEST_CASE("test_TradeManager_can_not_checkin") {
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000);

    /** @arg 试图存入的金额 <= 0 */
    CHECK_EQ(tm->checkin(Datetime(199901020000), 0), false);
    CHECK_EQ(tm->checkin(Datetime(199901020000), -0.01), false);
    CHECK_EQ(tm->checkin(Datetime(199901020000), 0.01), true);

    /** @arg 试图在最后交易日期前存入 */
    tm->checkin(Datetime(200001020000), 10000);
    CHECK_EQ(tm->checkin(Datetime(200001010000), 200), false);
}

/** @par 检测点 */
TEST_CASE("test_TradeManager_can_not_checkout") {
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000);

    /** @arg 试图取出的金额 <= 0 */
    CHECK_EQ(tm->checkout(Datetime(199901020000), 0), false);
    CHECK_EQ(tm->checkout(Datetime(199901020000), -0.01), false);
    CHECK_EQ(tm->checkout(Datetime(199901020000), 0.01), true);

    /** @arg 试图在最后交易日期前取出 */
    tm->checkin(Datetime(200001020000), 0.01);
    CHECK_EQ(tm->checkout(Datetime(200001010000), 200), false);

    /** @arg 试图取出的金额，大于当前账户余额 */
    CHECK_EQ(tm->currentCash(), 100000);
    CHECK_EQ(tm->checkout(Datetime(200001030000), 100000.01), false);
    CHECK_EQ(tm->checkout(Datetime(200001030000), 100000), true);
}

/** @par 检测点 */
TEST_CASE("test_TradeManager_normal_buy_and_sell_no_margin_by_day") {
    Stock stk = getStock("sz000001");
    TradeManagerPtr tm = crtTM(Datetime(199305010000LL), 100000);
    TradeRecord tr;
    CostRecord cost;

    /** @arg 检测初始建立的 TradeManager, 针对所有接口 */
    CHECK_EQ(tm->initDatetime(), Datetime(199305010000LL));
    CHECK_EQ(tm->firstDatetime(), Datetime());
    CHECK_EQ(tm->lastDatetime(), tm->initDatetime());
    CHECK_EQ(tm->currentCash(), 100000.0);
    CHECK_EQ(tm->cash(Datetime(199305010000LL), KQuery::DAY), tm->currentCash());
    CHECK_EQ(tm->cash(Datetime(199305010000LL), KQuery::DAY), tm->currentCash());
    CHECK_UNARY(!tm->have(stk));
    CHECK_EQ(tm->getStockNumber(), 0);
    CHECK_EQ(tm->getHoldNumber(Datetime(199305100000LL), stk), 0);
    auto tr_list = tm->getTradeList();
    CHECK_EQ(tr_list.size(), 1);
    CHECK_EQ(tr_list[0],
             TradeRecord(Null<Stock>(), Datetime(199305010000LL), BUSINESS_INIT, 100000., 100000.,
                         0., 0., cost, 0., 100000., 1.0, SystemPart::PART_INVALID));
    auto position_list = tm->getPositionList();
    CHECK_EQ(position_list.size(), 0);
    auto history_position_list = tm->getHistoryPositionList();
    CHECK_EQ(history_position_list.size(), 0);
    auto position = tm->getPosition(Datetime(199305020000LL), stk);
    CHECK_EQ(position, Null<PositionRecord>());
    CHECK_EQ(tm->getFunds(Null<Datetime>(), KQuery::DAY), tm->getFunds(tm->lastDatetime()));
    CHECK_EQ(tm->getFunds(Datetime(199304300000LL), KQuery::DAY), FundsRecord(0., 0., 0., 0.));
    CHECK_EQ(tm->getFunds(Datetime(199305010900LL), KQuery::DAY),
             FundsRecord(100000., 0., 100000.0, 0.));
    CHECK_EQ(tm->getFunds(Datetime(199305100000LL), KQuery::DAY),
             FundsRecord(100000., 0., 100000.0, 0.));
    auto date_list = getDateRange(Datetime(199304300000LL), Datetime(199305030000LL));
    auto funds_curve = tm->getFundsCurve(date_list, KQuery::DAY);
    CHECK_EQ(funds_curve.size(), 3);
    CHECK_EQ(funds_curve[0], 0.);
    CHECK_EQ(funds_curve[1], 100000.);
    CHECK_EQ(funds_curve[2], 100000.);
    auto profit_curve = tm->getProfitCurve(date_list, KQuery::DAY);
    CHECK_EQ(profit_curve.size(), 3);
    CHECK_EQ(profit_curve[0], 0.);
    CHECK_EQ(profit_curve[1], 0.);
    CHECK_EQ(profit_curve[2], 0.);

    /** @arg 买入两次后，一次全买 */
    tr = tm->buy(Datetime(199305200000L), stk, 55.8, 100);
    CHECK_EQ(tr, TradeRecord(stk, Datetime(199305200000LL), BUSINESS_BUY, 0.0, 55.8, 0.0, 100, cost,
                             0.0, 94420.00, 1.0, PART_INVALID));
    CHECK_EQ(tm->currentCash(), 94420.0);

    tr = tm->buy(Datetime(199305200000LL), stk, 56.1, 200);
    CHECK_EQ(tr, TradeRecord(stk, Datetime(199305200000L), BUSINESS_BUY, 0.0, 56.1, 0.0, 200, cost,
                             0.0, 83200.00, 1.0, PART_INVALID));
    CHECK_EQ(tm->currentCash(), 83200.00);

    tr = tm->sell(Datetime(199305210000L), stk, 55.15, 300);
    CHECK_EQ(tm->currentCash(), 99745.00);
    CHECK(tm->getPositionList().empty());
    CHECK_EQ(tr, TradeRecord(stk, Datetime(199305210000L), BUSINESS_SELL, 0.0, 55.15, 0.0, 300,
                             cost, 0.0, 99745.00, 1.0, PART_INVALID));

    /** @arg 第一次买入100，第二次买入200，第三次卖出200 */
    tm = crtTM(Datetime(199305010000), 100000);
    tr = tm->buy(Datetime(199305200000L), stk, 55.8, 100);
    tr = tm->buy(Datetime(199305200000L), stk, 56.1, 200);
    CHECK_EQ(tm->getFunds(Datetime(199305200000L)), FundsRecord(83200.0, 16710.0, 100000., 0.));
    tr = tm->sell(Datetime(199305210000L), stk, 55.15, 200);
    CHECK_EQ(tm->getFunds(Datetime(199305200000L)), FundsRecord(83200.0, 16710.0, 100000., 0.));
    CHECK_EQ(tm->currentCash(), 94230.00);
    CHECK_EQ(tr, TradeRecord(stk, Datetime(199305210000L), BUSINESS_SELL, 0.0, 55.15, 0.0, 200,
                             cost, 0.0, 94230.00, 1.0, PART_INVALID));
    auto positionList = tm->getPositionList();
    CHECK_EQ(positionList.size(), 1);
    position = positionList[0];
    CHECK_EQ(position.number, 100.0);
    CHECK_EQ(position.totalNumber, 300.0);
    CHECK_EQ(position.contracts.size(), 1);
    const auto& contract = position.contracts.front();
    CHECK_EQ(contract.datetime, Datetime(199305200000L));
    CHECK_EQ(contract.number, 100);
    CHECK_EQ(contract.price, 56.1);
    CHECK_EQ(contract.marginRatio, 1.0);
    CHECK_EQ(tm->getFunds(Datetime(199305200000L)), FundsRecord(83200.0, 16710.0, 100000., 0.));
    CHECK_EQ(tm->getFunds(Datetime(199305210000L)), FundsRecord(94230.0, 5440.0, 100000., 0.));

    /** @arg 5月26日卖出剩余股票，中间经历权息变化, 检测 tm 所有接口正确性 */
    auto current_num = tm->getHoldNumber(Datetime(199305260000LL), stk);
    CHECK_EQ(current_num, 185);  // 24日送转85股, 红利30元
    CHECK_EQ(tm->getFunds(Datetime(199305260000ULL)), FundsRecord(94260., 4995.0, 100000., 0.));
    tr = tm->sell(Datetime(199305260000LL), stk, 28.1, MAX_DOUBLE);

    CHECK_EQ(tm->currentCash(), 99458.50);
    CHECK_EQ(tr, TradeRecord(stk, Datetime(199305260000LL), BUSINESS_SELL, 0., 28.1, 0., 185., cost,
                             0., 99458.5, 1., PART_INVALID));
    CHECK(tm->getPositionList().empty());
    CHECK_EQ(tm->getFunds(Datetime(199304300000LL), KQuery::DAY), FundsRecord(0., 0., 0., 0.));
    CHECK_EQ(tm->getFunds(Datetime(199305010900LL), KQuery::DAY),
             FundsRecord(100000., 0., 100000.0, 0.));
    CHECK_EQ(tm->getFunds(Datetime(199305100000LL), KQuery::DAY),
             FundsRecord(100000., 0., 100000.0, 0.));
    CHECK_EQ(tm->getFunds(Datetime(199305260000ULL)), FundsRecord(99458.5, 0., 100000., 0.));
}

/** @par 检测点，正常买卖 */
TEST_CASE("test_TradeManager_normal_buy_and_sell_with_margin") {
    Stock stk = getStock("sz000001");

    // 创建使用合约交易的账户
    TradeManagerPtr tm = crtTM(Datetime(199305010000), 1000., TC_Zero(), MR_Fixed(0.1));
    tm->setParam<bool>("use_contract", true);

    TradeRecord tr;
    CostRecord cost;

    //
    tr = tm->buy(Datetime(199305200000L), stk, 55.8, 100);
    CHECK_EQ(tr, TradeRecord(stk, Datetime(199305200000L), BUSINESS_BUY, 0.0, 55.8, 0.0, 100, cost,
                             0.0, 442.00, 0.1, PART_INVALID));
    CHECK_EQ(tm->currentCash(), 442.0);

    // HKU_INFO("{}", tm->getFunds(KQuery::DAY));
    // HKU_INFO("{}", stk.getKRecord(stk.getCount() - 1));
    // HKU_INFO("{}", tr);
    // HKU_INFO("{}", stk.getKRecord(Datetime(199305240000L)));
    // HKU_INFO("{}", stk.getKRecord(Datetime(199305250000L)));
    // HKU_INFO("{}", tm);
}

/** @par 检测点，测试 getTradeList */
TEST_CASE("test_TradeManager_getTradeList") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm.getStock("sz000001");

    CostRecord cost;

    TradeManagerPtr tm = crtTM(Datetime(199305010000), 100000);

    tm->buy(Datetime(199305200000L), stk, 55.7, 100);
    tm->buy(Datetime(199305250000L), stk, 27.5, 100);
    tm->buy(Datetime(199407110000L), stk, 8.55, 200);

    /** @arg 获取全部交易记录 */
    TradeRecordList tr_list = tm->getTradeList();
    CHECK_EQ(tr_list.size(), 8);
    CHECK_EQ(tr_list[0], TradeRecord(Stock(), Datetime(199305010000L), BUSINESS_INIT, 100000,
                                     100000, 0, 0, cost, 0, 100000, 1.0, PART_INVALID));
    CHECK_EQ(tr_list[7], TradeRecord(stk, Datetime(199407110000L), BUSINESS_BUY, 0, 8.55, 0, 200,
                                     cost, 0, 90142.50, 1.0, PART_INVALID));

    /** @arg 指定日期范围获取交易记录 start 等于账户建立日期，end 为  Null<Datetime>() */
    tr_list = tm->getTradeList(Datetime(199305010000), Null<Datetime>());

    CHECK_EQ(tr_list.size(), 8);
    CHECK_EQ(tr_list[0], TradeRecord(Stock(), Datetime(199305010000L), BUSINESS_INIT, 100000,
                                     100000, 0, 0, cost, 0, 100000, 1.0, PART_INVALID));
    CHECK_EQ(tr_list[7], TradeRecord(stk, Datetime(199407110000L), BUSINESS_BUY, 0, 8.55, 0, 200,
                                     cost, 0, 90142.50, 1.0, PART_INVALID));

    /** @arg 指定日期范围获取交易记录 start 等于第一条买入记录日期，end为Null */
    tr_list = tm->getTradeList(Datetime(199305200000L), Null<Datetime>());

    CHECK_EQ(tr_list.size(), 7);
    CHECK_EQ(tr_list[0], TradeRecord(stk, Datetime(199305200000L), BUSINESS_BUY, 0, 55.70, 0, 100,
                                     cost, 0, 94430, 1.0, PART_INVALID));
    CHECK_EQ(tr_list[6], TradeRecord(stk, Datetime(199407110000L), BUSINESS_BUY, 0, 8.55, 0, 200,
                                     cost, 0, 90142.50, 1.0, PART_INVALID));

    /** @arg 指定日期范围获取交易记录 start 介于两个交易记录日期之间，end为Null */
    tr_list = tm->getTradeList(Datetime(199305210000L), Null<Datetime>());

    CHECK_EQ(tr_list.size(), 6);
    CHECK_EQ(tr_list[0], TradeRecord(stk, Datetime(199305240000L), BUSINESS_BONUS, 30, 30, 0, 0,
                                     cost, 0, 94490, 1.0, PART_INVALID));
    CHECK_EQ(tr_list[5], TradeRecord(stk, Datetime(199407110000L), BUSINESS_BUY, 0, 8.55, 0, 200,
                                     cost, 0, 90142.50, 1.0, PART_INVALID));

    /** @arg 指定日期范围获取交易记录 start 大于 end */
    tr_list = tm->getTradeList(Null<Datetime>(), Datetime(199305210000L));
    CHECK_EQ(tr_list.size(), 0);

    /** @arg 指定日期范围获取交易记录 start 等于 end */
    tr_list = tm->getTradeList(Datetime(199305210000L), Datetime(199305210000L));
    CHECK_EQ(tr_list.size(), 0);

    /** @arg 指定日期范围获取交易记录 start等于某交易记录日期，end大于最后一条交易记录日期 */
    tr_list = tm->getTradeList(Datetime(199305200000L), Datetime(199407120000L));

    CHECK_EQ(tr_list.size(), 7);
    CHECK_EQ(tr_list[0], TradeRecord(stk, Datetime(199305200000L), BUSINESS_BUY, 0, 55.70, 0, 100,
                                     cost, 0, 94430, 1.0, PART_INVALID));
    CHECK_EQ(tr_list[6], TradeRecord(stk, Datetime(199407110000L), BUSINESS_BUY, 0, 8.55, 0, 200,
                                     cost, 0, 90142.50, 1.0, PART_INVALID));

    /** @arg 指定日期范围获取交易记录 start等于某交易记录日期，end等于最后一条交易记录日期 */
    tr_list = tm->getTradeList(Datetime(199305200000L), Datetime(199407110000L));

    CHECK_EQ(tr_list.size(), 4);
    CHECK_EQ(tr_list[0], TradeRecord(stk, Datetime(199305200000L), BUSINESS_BUY, 0, 55.70, 0, 100,
                                     cost, 0, 94430, 1.0, PART_INVALID));
    CHECK_EQ(tr_list[3], TradeRecord(stk, Datetime(199305250000L), BUSINESS_BUY, 0, 27.5, 0, 100,
                                     cost, 0, 91710, 1.0, PART_INVALID));
}

/** @par 检测点, 测试addTradeRecord */
TEST_CASE("test_TradeManager_addTradeRecord") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm.getStock("sz000001");

    CostRecord cost;

    TradeManagerPtr tm = crtTM(Datetime(199305010000), 100000);

    tm->buy(Datetime(199305200000L), stk, 55.7, 100);
    tm->buy(Datetime(199305250000L), stk, 27.5, 100);
    tm->buy(Datetime(199407110000L), stk, 8.55, 200);

    /** @arg 加入账户初始化交易记录 */
    TradeRecordList tr_list = tm->getTradeList();
    CHECK_EQ(tr_list.size(), 8);
    CHECK_EQ(tr_list[0], TradeRecord(Stock(), Datetime(199305010000L), BUSINESS_INIT, 100000,
                                     100000, 0, 0, cost, 0, 100000, 1.0, PART_INVALID));

    TradeRecord tr(Stock(), Datetime(199201010000L), BUSINESS_INIT, 200000, 200000, 0, 0, cost, 0,
                   200000, 1.0, PART_INVALID);
    tm->addTradeRecord(tr);

    tr_list = tm->getTradeList();
    CHECK_EQ(tr_list.size(), 1);
    CHECK_EQ(tr_list[0], tr);

    /** @arg 复制一个tm的交易记录至另一个tm */
    tm = crtTM(Datetime(199305010000), 100000);
    tm->buy(Datetime(199305200000L), stk, 55.7, 100);
    tm->buy(Datetime(199305250000L), stk, 27.5, 100);
    tm->buy(Datetime(199407110000L), stk, 8.55, 200);
    tr_list = tm->getTradeList();

    TMPtr tm2 = crtTM(Datetime(199101010000), 100000, TC_Zero(), MRPtr(), "TM2");
    CHECK_NE(tm->initDatetime(), tm2->initDatetime());
    for (auto iter = tr_list.begin(); iter != tr_list.end(); ++iter) {
        tm2->addTradeRecord(*iter);
    }

    CHECK_NE(tm2->name(), tm->name());
    CHECK_EQ(tm2->initDatetime(), tm->initDatetime());
    CHECK_EQ(tm2->lastDatetime(), tm->lastDatetime());
    CHECK_EQ(tm2->currentCash(), tm->currentCash());

    tr_list = tm2->getTradeList();
    CHECK_EQ(tr_list.size(), 8);
    CHECK_EQ(tr_list[0], TradeRecord(Stock(), Datetime(199305010000L), BUSINESS_INIT, 100000,
                                     100000, 0, 0, cost, 0, 100000, 1.0, PART_INVALID));
    CHECK_EQ(tr_list[7], TradeRecord(stk, Datetime(199407110000L), BUSINESS_BUY, 0, 8.55, 0, 200,
                                     cost, 0, 90142.50, 1.0, PART_INVALID));
}

/** @} */
