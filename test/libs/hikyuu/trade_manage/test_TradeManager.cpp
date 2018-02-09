/*
 * test_TradeManager.cpp
 *
 *  Created on: 2013-2-21
 *      Author: fasiondog
 */

#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_trade_manage_suite
    #include <boost/test/unit_test.hpp>
#endif

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
BOOST_AUTO_TEST_CASE( test_TradeManager_init ) {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    CostRecord result, expect;
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000,
            TC_FixedA(0.0018, 5, 0.001, 0.001, 1.0), "TEST");

    BOOST_CHECK(tm->name() == "TEST");
    BOOST_CHECK(tm->initCash() == 100000.0);
    //BOOST_CHECK(tm->cash() == 100000.0);
    BOOST_CHECK(tm->initDatetime() == Datetime(199901010000));
    BOOST_CHECK(tm->firstDatetime() == Null<Datetime>());
    BOOST_CHECK(tm->lastDatetime() == Datetime(199901010000));
    BOOST_CHECK(tm->have(stock) == false);
    BOOST_CHECK(tm->getStockNumber() == 0);
    BOOST_CHECK(tm->getHoldNumber(Datetime(199901010000), stock) == 0);
    const TradeRecordList& tradeList = tm->getTradeList();
    BOOST_CHECK(tradeList.size() == 1);
    BOOST_CHECK(tradeList[0] == TradeRecord(Null<Stock>(),
            Datetime(199901010000), BUSINESS_INIT, 100000.0, 100000.0, 0.0, 0,
            Null<CostRecord>(), 0.0, 100000.0, PART_INVALID));
    BOOST_CHECK(tm->getPositionList().empty() == true);
    BOOST_CHECK(tm->getPosition(stock) == Null<PositionRecord>());

    BOOST_CHECK(tm->getShortPositionList().empty() == true);
    BOOST_CHECK(tm->getShortPosition(stock) == Null<PositionRecord>());

}


/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_TradeManager_getBuyCost ) {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    CostRecord result, expect;
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000,
            TC_FixedA(0.0018, 5, 0.001, 0.001, 1.0), "TEST");

    /** @arg 调用CostFunc是否正常 */
    result = tm->getBuyCost(Datetime(200101010000), stock, 10.0, 1000);
    expect.commission = 18.0;
    expect.stamptax = 0.0;
    expect.transferfee = 1.0;
    expect.total = 19.0;
    BOOST_CHECK(result == expect);
}


/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_TradeManager_getSellCost ) {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600004");
    CostRecord result, expect;
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000,
            TC_FixedA(0.0018, 5, 0.001, 0.001, 1.0));

    /** @arg 调用CostFunc是否正常 */
    result = tm->getSellCost(Datetime(200101010000), stock, 10.0, 100);
    expect.commission = 5.0;
    expect.stamptax = 1.0;
    expect.transferfee = 1.0;
    expect.total = 7.0;
    BOOST_CHECK(result == expect);
}


/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_TradeManager_can_not_buy ) {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeCostPtr costfunc = TC_FixedA(0.0018, 5, 0.001, 0.001, 1.0);
    TradeManagerPtr tm;
    TradeRecord result;
    CostRecord cost;
    TradeRecord trade;
    TradeRecordList trade_list;

    /** @arg 账户初始余额为0，未进行过交易，忽略权息信息 */
    tm = crtTM(Datetime(199901010000), 0, costfunc, "SYS");
    result = tm->buy(Datetime(199911180000), stock, 27.2, 100, 0, 27.2, 27.2);
    BOOST_CHECK(result == Null<TradeRecord>());

    /** @arg 账户初始余额为100000，试图对Null<Stock>进行操作 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->buy(Datetime(199911180000), Null<Stock>(), 27.2, 100, 0, 27.2, 27.2);
    BOOST_CHECK(result == Null<TradeRecord>());
    BOOST_CHECK(tm->cash(Datetime(199911180000)) == 100000);

    /** @arg 试图在初始建仓日之前买入 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->buy(Datetime(199001010000), stock, 26.36, 100, 0, 26.36, 26.36);
    BOOST_CHECK(result == Null<TradeRecord>());

#if 0  //取消了该限制
    /** @arg 账户初始余额为100000，未进行过交易，忽略权息信息，但指定日期该证券不能进行交易，如非交易日 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->buy(Datetime(199911130000), stock, 27.2, 100, 0, 27.2, 27.2);
    BOOST_CHECK(result == Null<TradeRecord>());

    /** @arg 账户初始余额为100000，未进行过交易，忽略权息信息，但买入价格超出当日最高价 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->buy(Datetime(199911170000), stock, 27.2, 100, 0, 27.2, 27.2);
    BOOST_CHECK(result == Null<TradeRecord>());
#endif

    /** @arg 账户初始余额为100000，未进行过交易，忽略权息信息，但买入价格等于当日最高价 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->buy(Datetime(199911170000), stock, 27.18, 100, 0, 27.18, 27.18);
    cost = tm->getBuyCost(Datetime(199911170000), stock, 27.18, 100);
    trade = TradeRecord(stock, Datetime(199911170000),
            BUSINESS_BUY, 27.18, 27.18, 27.18, 100, cost, 0.0,
            100000 - cost.total - 27.18 * 100, PART_INVALID);
    BOOST_CHECK(result == trade);
    BOOST_CHECK(tm->cash(Datetime(199911170000)) == trade.cash);
    trade_list = tm->getTradeList();
    BOOST_CHECK(trade_list.size() == 2);
    BOOST_CHECK(trade_list[1] == trade);

#if 0
    /** @arg 账户初始余额为100000，未进行过交易，忽略权息信息，但买入价格低于当日最低价 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->buy(Datetime(199911170000), stock, 26.36, 100, 0, 26.36, 26.36);
    BOOST_CHECK(result == Null<TradeRecord>());
#endif

    /** @arg 账户初始余额为100000，未进行过交易，忽略权息信息，但买入价格等于当日最低价 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->buy(Datetime(199911160000), stock, 26.48, 100, 0, 26.48, 26.48);
    cost = tm->getBuyCost(Datetime(199911160000), stock, 26.48, 100);
    BOOST_CHECK(result == TradeRecord(stock, Datetime(199911160000),
            BUSINESS_BUY, 26.48, 26.48, 26.48, 100, cost, 0.0,
            100000 - cost.total - 26.48 * 100, PART_INVALID));

    /** @arg 试图在最后一笔交易时间之前进行交易 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->buy(Datetime(199911170000), stock, 26.48, 100, 0, 26.48, 26.48);
    BOOST_CHECK(!(result == Null<TradeRecord>()));
    result = tm->buy(Datetime(199911160000), stock, 26.48, 100, 0, 26.48, 26.48);
    BOOST_CHECK(result == Null<TradeRecord>());

    /** @arg 试图买入数量为0的股票 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->buy(Datetime(199911170000), stock, 26.48, 0, 0, 26.48, 26.48);
    BOOST_CHECK(result == Null<TradeRecord>());

    /** @arg 买入数量小于该股票的最小交易量 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->buy(Datetime(199911170000), stock, 26.48, stock.minTradeNumber() - 1, 0, 26.48, 26.48);
    BOOST_CHECK(result == Null<TradeRecord>());

    /** @arg 买入数量大于该股票的最大交易量 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->buy(Datetime(199911170000), stock, 26.48, stock.maxTradeNumber() - 1, 0, 26.48, 26.48);
    BOOST_CHECK(result == Null<TradeRecord>());
}

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_TradeManager_can_not_sell ) {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeCostPtr costfunc = TC_FixedA(0.0018, 5, 0.001, 0.001, 1.0);
    TradeManagerPtr tm;
    TradeRecord result;
    CostRecord cost;
    TradeRecord trade;
    TradeRecordList trade_list;

    /** @arg 账户初始余额为0，未进行过交易，忽略权息信息 */
    tm = crtTM(Datetime(199901010000), 0, costfunc, "SYS");
    result = tm->sell(Datetime(199911180000), stock, 27.2, 100);
    BOOST_CHECK(result == Null<TradeRecord>());

    /** @arg 账户初始余额为100000，试图对Null<Stock>进行操作 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->sell(Datetime(199911180000), Null<Stock>(), 27.2, 100);
    BOOST_CHECK(result == Null<TradeRecord>());
    BOOST_CHECK(tm->cash(Datetime(199911180000)) == 100000);

    /** @arg 试图在最后一个交易日之前卖出 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->buy(Datetime(199911170000), stock, 27.18, 100, 0);
    BOOST_CHECK(tm->getHoldNumber(Datetime(199911170000), stock) == 100);
    result = tm->sell(Datetime(199801010000), stock, 26.36, 100);
    BOOST_CHECK(result == Null<TradeRecord>());

    /** @arg 卖出的数量等于0 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->buy(Datetime(199911170000), stock, 27.18, 100, 0);
    BOOST_CHECK(tm->getHoldNumber(Datetime(199911170000), stock) == 100);
    result = tm->sell(Datetime(199911180000), stock, 26.36, 0);
    BOOST_CHECK(result == Null<TradeRecord>());

    /** @arg 卖出的数量小于最小交易数量 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->buy(Datetime(199911170000), stock, 27.18, 100, 0);
    BOOST_CHECK(tm->getHoldNumber(Datetime(199911170000), stock) == 100);
    result = tm->sell(Datetime(199911180000), stock, 26.36, stock.minTradeNumber() - 1);
    BOOST_CHECK(result == Null<TradeRecord>());

    /** @arg 卖出的数量大于最大交易数量 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->buy(Datetime(199911170000), stock, 27.18, 100, 0);
    BOOST_CHECK(tm->getHoldNumber(Datetime(199911170000), stock) == 100);
    result = tm->sell(Datetime(199911180000), stock, 26.36, stock.maxTradeNumber() + 1);
    BOOST_CHECK(result == Null<TradeRecord>());

    /** @arg 卖出未持仓的股票 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->sell(Datetime(199901020000), stock, 26.36, 100);
    BOOST_CHECK(result == Null<TradeRecord>());

    /** @arg 卖出的数量大于当前持仓数量 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->buy(Datetime(199911170000), stock, 27.18, 100, 0);
    BOOST_CHECK(tm->getHoldNumber(Datetime(199911170000), stock) == 100);
    result = tm->sell(Datetime(199911180000), stock, 26.36, 101);
    BOOST_CHECK(result == Null<TradeRecord>());

    /** @arg 忽略权息信息，将买入股票全部卖出 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->buy(Datetime(199911170000), stock, 27.18, 100, 0, 27.18, 27.18);
    cost = tm->getBuyCost(Datetime(199911170000), stock, 27.18, 100);
    BOOST_CHECK(result == TradeRecord(stock, Datetime(199911170000),
            BUSINESS_BUY, 27.18, 27.18, 27.18, 100, cost, 0.0,
            100000 - cost.total - 27.18 * 100, PART_INVALID));
    BOOST_CHECK(tm->getHoldNumber(Datetime(199911170000), stock) == 100);
    BOOST_CHECK(tm->getStockNumber() == 1);
    BOOST_CHECK(tm->cash(Datetime(199911170000)) == 97276.0);
    result = tm->sell(Datetime(199911180000), stock, 26.36, Null<size_t>(), 0, 0, 26.36);
    cost = tm->getSellCost(Datetime(199911180000), stock, 26.36, 100);
    BOOST_CHECK(result == TradeRecord(stock, Datetime(199911180000),
            BUSINESS_SELL, 26.36, 26.36, 0.0, 100, cost, 0.0, 99903.36, PART_INVALID));
    BOOST_CHECK(tm->getHoldNumber(Datetime(199911180000), stock) == 0);
    BOOST_CHECK(tm->getStockNumber() == 0);
    BOOST_CHECK(tm->cash(Datetime(199911180000)) == 99903.36);

    /** @arg 不忽略权息信息，对股票进行买卖操作，忽略买卖成本 */
    tm = crtTM(Datetime(199901010000), 1000000, TC_Zero(), "SYS");
    tm->setParam<bool>("reinvest", true);
    tm->buy(Datetime(199911170000), stock, 27.18, 1000, 0);
    BOOST_CHECK(tm->cash(Datetime(199911170000)) == 972820);
    tm->sell(Datetime(200605150000), stock, 10.2, 100);
    BOOST_CHECK(tm->cash(Datetime(200605150000)) == 974685); //973840);
    BOOST_CHECK(tm->getHoldNumber(Datetime(200605160000), stock) == 1850);
    tm->sell(Datetime(200612010000), stock, 16.87, Null<size_t>());
    BOOST_CHECK(tm->cash(Datetime(200612010000)) == 1006135.0); //1005049.5);
}


/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_TradeManager_can_not_checkin ) {
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000);

    /** @arg 试图存入的金额 <= 0 */
    BOOST_CHECK(tm->checkin(Datetime(199901020000), 0) == false);
    BOOST_CHECK(tm->checkin(Datetime(199901020000), -0.01) == false);
    BOOST_CHECK(tm->checkin(Datetime(199901020000), 0.01) == true);

    /** @arg 试图在最后交易日期前存入 */
    tm->checkin(Datetime(200001020000), 10000);
    BOOST_CHECK(tm->checkin(Datetime(200001010000), 200) == false);
}


/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_TradeManager_can_not_checkout ) {
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000);

    /** @arg 试图取出的金额 <= 0 */
    BOOST_CHECK(tm->checkout(Datetime(199901020000), 0) == false);
    BOOST_CHECK(tm->checkout(Datetime(199901020000), -0.01) == false);
    BOOST_CHECK(tm->checkout(Datetime(199901020000), 0.01) == true);

    /** @arg 试图在最后交易日期前取出 */
    tm->checkin(Datetime(200001020000), 0.01);
    BOOST_CHECK(tm->checkout(Datetime(200001010000), 200) == false);

    /** @arg 试图取出的金额，大于当前账户余额 */
    BOOST_CHECK(tm->currentCash() == 100000);
    BOOST_CHECK(tm->checkout(Datetime(200001030000), 100000.01) == false);
    BOOST_CHECK(tm->checkout(Datetime(200001030000), 100000) == true);
}


/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_TradeManager_can_not_borrowCash ) {
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000);

    /** @arg 试图存入的金额 <= 0 */
    BOOST_CHECK(tm->borrowCash(Datetime(199901020000), 0) == false);
    BOOST_CHECK(tm->borrowCash(Datetime(199901020000), -0.01) == false);
    BOOST_CHECK(tm->borrowCash(Datetime(199901020000), 0.01) == true);

    /** @arg 试图在最后交易日期前存入 */
    tm->checkin(Datetime(200001020000), 10000);
    BOOST_CHECK(tm->borrowCash(Datetime(200001010000), 200) == false);
}


/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_TradeManager_can_not_returnCash ) {
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000);

    /** @arg 试图在最后交易日期前操作 */
    tm->borrowCash(Datetime(200001020000), 50000);
    BOOST_CHECK(tm->returnCash(Datetime(200001010000), 200) == false);

    /** @arg 试图归还的金额 <= 0 */
    BOOST_CHECK(tm->returnCash(Datetime(200001020000), 0) == false);
    BOOST_CHECK(tm->returnCash(Datetime(200001020000), -0.01) == false);
    BOOST_CHECK(tm->returnCash(Datetime(200001020000), 0.01) == true);
    BOOST_CHECK(tm->borrowCash(Datetime(200001020000), 0.01) == true);

    /** @arg 试图归还的金额，大于当前的欠款额 */
    BOOST_CHECK(tm->getDebtCash(Datetime(200001030000)) == 50000);
    BOOST_CHECK(tm->returnCash(Datetime(200001030000), 50000.01) == false);
    BOOST_CHECK(tm->returnCash(Datetime(200001030000), 50000) == true);

    /** @arg 试图归还的金额，大于当前的账户余额 */
    tm->borrowCash(Datetime(200001040000), 50000);
    tm->checkout(Datetime(200001040000), 120000);
    BOOST_CHECK(tm->currentCash() == 30000);
    BOOST_CHECK(tm->getDebtCash(Datetime(200001040000)) == 50000);
    BOOST_CHECK(tm->returnCash(Datetime(200001040000), 50000) == false);
}


/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_TradeManager_can_not_checkinStock ) {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000);

    /** @arg 试图存入的stock is null */
    BOOST_CHECK(tm->checkinStock(Datetime(199901020000), Stock(), 10, 100) == false);

    /** @arg 试图存入的数量为0 */
    BOOST_CHECK(tm->checkinStock(Datetime(199901020000), stock, 10, 0) == false);

    /** @arg 试图存入的金额小于等于0 */
    BOOST_CHECK(tm->checkinStock(Datetime(199901020000), stock, 0, 100) == false);
    BOOST_CHECK(tm->checkinStock(Datetime(199901020000), stock, -0.01, 100) == false);

    /** @arg 试图在最后交易日期前存入 */
    tm->checkin(Datetime(200001020000), 10000);
    BOOST_CHECK(tm->checkinStock(Datetime(200001010000), stock, 10.0, 200) == false);
}

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_TradeManager_can_not_checkoutStock ) {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000);

    /** @arg 试图取出的stock is null */
    BOOST_CHECK(tm->checkoutStock(Datetime(199901020000), Stock(), 10, 100) == false);

    /** @arg 试图取出的数量为0 */
    BOOST_CHECK(tm->checkinStock(Datetime(199901020000), stock, 10, 100) == true);
    BOOST_CHECK(tm->checkoutStock(Datetime(199901020000), stock, 10, 0) == false);

    /** @arg 试图取出的金额小于等于0 */
    BOOST_CHECK(tm->checkoutStock(Datetime(199901020000), stock, 0, 100) == false);
    BOOST_CHECK(tm->checkoutStock(Datetime(199901020000), stock, -0.01, 100) == false);

    /** @arg 试图在最后交易日期前取出 */
    BOOST_CHECK(tm->checkinStock(Datetime(199901010000), stock, 10.0, 200) == false);
}


/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_TradeManager_can_not_borrowStock ) {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000);

    /** @arg 试图借入的stock is null */
    BOOST_CHECK(tm->borrowStock(Datetime(199901020000), Stock(), 10, 100) == false);

    /** @arg 试图借入的数量为0 */
    BOOST_CHECK(tm->borrowStock(Datetime(199901020000), stock, 10, 0) == false);

    /** @arg 试图借入的金额小于等于0 */
    BOOST_CHECK(tm->borrowStock(Datetime(199901020000), stock, 0, 100) == false);
    BOOST_CHECK(tm->borrowStock(Datetime(199901020000), stock, -0.01, 100) == false);

    /** @arg 试图在最后交易日期前借入 */
    tm->checkin(Datetime(200001020000), 10000);
    BOOST_CHECK(tm->borrowStock(Datetime(200001010000), stock, 10.0, 200) == false);
}


/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_TradeManager_can_not_returnStock ) {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000);

    BOOST_CHECK(tm->borrowStock(Datetime(199901020000), stock, 10, 100) == true);

    /** @arg 试图归还的stock is null */
    BOOST_CHECK(tm->returnStock(Datetime(199901020000), Stock(), 10, 100) == false);

    /** @arg 试图归还的数量为0 */
    BOOST_CHECK(tm->returnStock(Datetime(199901020000), stock, 10, 0) == false);

    /** @arg 试图归还的金额小于等于0 */
    BOOST_CHECK(tm->returnStock(Datetime(199901020000), stock, 0, 100) == false);
    BOOST_CHECK(tm->returnStock(Datetime(199901020000), stock, -0.01, 100) == false);

    /** @arg 试图在最后交易日期前归还 */
    BOOST_CHECK(tm->returnStock(Datetime(199901010000), stock, 10.0, 100) == false);

    /** @arg 试图归还的数量大于借入的数量 */
    BOOST_CHECK(tm->returnStock(Datetime(199901030000), stock, 10.0, 101) == false);
    BOOST_CHECK(tm->returnStock(Datetime(199901030000), stock, 10.0, 100) == true);
}


/** @par 检测点, 多次借入、归还现金 */
BOOST_AUTO_TEST_CASE( test_TradeManager_trade_multi_borrow_cash_by_day ) {
    FundsRecord funds;
    TradeCostPtr tc = TC_TestStub();
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000, tc);
    tm->setParam<bool>("reinvest", false); //忽略权息

    Datetime cur_date, pre_date, next_date;

    /** @arg 19991117 借入5000, 分2次归还 */
    cur_date = Datetime(199911170000);
    pre_date = Datetime(199911160000);
    next_date = Datetime(199911180000);
    BOOST_CHECK(tm->borrowCash(cur_date, 5000) == true);
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(104970, 0, 0, 100000, 0, 5000, 0));
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(100000, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(104970, 0, 0, 100000, 0, 5000, 0));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(104970, 0, 0, 100000, 0, 5000, 0));

    cur_date = Datetime(199911180000);
    pre_date = Datetime(199911170000);
    next_date = Datetime(199911190000);
    BOOST_CHECK(tm->returnCash(cur_date, 3000) == true);
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(101930, 0, 0, 100000, 0, 2000, 0));
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(104970, 0, 0, 100000, 0, 5000, 0));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(101930, 0, 0, 100000, 0, 2000, 0));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(101930, 0, 0, 100000, 0, 2000, 0));

    BOOST_CHECK(tm->returnCash(cur_date, 2000.01) == false);
    BOOST_CHECK(tm->returnCash(cur_date, 2000) == true);
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(99890, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(104970, 0, 0, 100000, 0, 5000, 0));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(99890, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(99890, 0, 0, 100000, 0, 0, 0));

    /** @arg 分两次借入5000元，一次归还 */
    cur_date = Datetime(199911190000);
    pre_date = Datetime(199911180000);
    next_date = Datetime(199911200000);
    BOOST_CHECK(tm->borrowCash(cur_date, 3000) == true);
    BOOST_CHECK(tm->borrowCash(cur_date, 2000) == true);
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(104830, 0, 0, 100000, 0, 5000, 0));
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(99890, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(104830, 0, 0, 100000, 0, 5000, 0));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(104830, 0, 0, 100000, 0, 5000, 0));

    BOOST_CHECK(tm->returnCash(cur_date, 5000.01) == false);
    BOOST_CHECK(tm->returnCash(cur_date, 5000) == true);
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(99750, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(99890, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(99750, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(99750, 0, 0, 100000, 0, 0, 0));

    /** @arg 分两次借入5000元，跨记录两次归还 */
    tm->reset();
    cur_date = Datetime(199911200000);
    pre_date = Datetime(199911190000);
    next_date = Datetime(199911210000);
    BOOST_CHECK(tm->borrowCash(cur_date, 3000) == true);
    BOOST_CHECK(tm->borrowCash(cur_date, 2000) == true);
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(104940, 0, 0, 100000, 0, 5000, 0));
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(100000, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(104940, 0, 0, 100000, 0, 5000, 0));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(104940, 0, 0, 100000, 0, 5000, 0));

    BOOST_CHECK(tm->returnCash(cur_date, 4000) == true);
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(100860, 0, 0, 100000, 0, 1000, 0));
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(100000, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(100860, 0, 0, 100000, 0, 1000, 0));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(100860, 0, 0, 100000, 0, 1000, 0));

    BOOST_CHECK(tm->returnCash(cur_date, 1000.01) == false);
    BOOST_CHECK(tm->returnCash(cur_date, 1000) == true);
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(99820, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(100000, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(99820, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(99820, 0, 0, 100000, 0, 0, 0));
}


/** @par 检测点, 多次借入、归还股票 */
BOOST_AUTO_TEST_CASE( test_TradeManager_trade_multi_borrow_stock_by_day ) {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    Stock stock2 = sm.getStock("sz000001");

    CostRecord cost;
    TradeRecord trade;
    FundsRecord funds;
    TradeCostPtr tc = TC_TestStub();
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000, tc);
    tm->setParam<bool>("reinvest", false); //忽略权息
    tm->setParam<bool>("support_borrow_cash", false);
    tm->setParam<bool>("support_borrow_stock", false);

    Datetime cur_date, pre_date, next_date;

    /** @arg 19991117 一次性买入1000股, 分两笔归还 */
    cur_date = Datetime(199911170000);
    pre_date = Datetime(199911160000);
    next_date = Datetime(199911180000);
    BOOST_CHECK(tm->borrowStock(cur_date, stock, 27.18, 1000) == true);
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(99950, 0, 0, 100000, 0, 0, 27180));
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(100000, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(99950, 0, 0, 100000, 0, 0, 27180));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(99950, 0, 0, 100000, 0, 0, 27180));

    cur_date = Datetime(199911180000);
    pre_date = Datetime(199911170000);
    next_date = Datetime(199911190000);
    BOOST_CHECK(tm->returnStock(cur_date, stock, 27.1, 800) == true);
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(99890, 0, 0, 100000, 0, 0, 5436));
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(99950, 0, 0, 100000, 0, 0, 27180));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(99890, 0, 0, 100000, 0, 0, 5436));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(99890, 0, 0, 100000, 0, 0, 5436));

    BOOST_CHECK(tm->returnStock(cur_date, stock, 26.8, 200) == true);
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(99830, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(99950, 0, 0, 100000, 0, 0, 27180));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(99830, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(99830, 0, 0, 100000, 0, 0, 0));

    /** @arg 19991123 分两次买入共1000股，一次性归还 */
    cur_date = Datetime(199911230000);
    pre_date = Datetime(199911220000);
    next_date = Datetime(199911240000);
    BOOST_CHECK(tm->borrowStock(cur_date, stock, 26.21, 200) == true);
    BOOST_CHECK(tm->borrowStock(cur_date, stock, 26.43, 800) == true);
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(99730, 0, 0, 100000, 0, 0, 26386));
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(99830, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(99730, 0, 0, 100000, 0, 0, 26386));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(99730, 0, 0, 100000, 0, 0, 26386));

    cur_date = Datetime(199911240000);
    pre_date = Datetime(199911230000);
    next_date = Datetime(199911250000);
    BOOST_CHECK(tm->returnStock(cur_date, stock, 26.20, 1000) == true);
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(99610, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(99730, 0, 0, 100000, 0, 0, 26386));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(99610, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(99610, 0, 0, 100000, 0, 0, 0));

    /** @arg 19991123 分两次买入1000股，跨记录两次归还 */
    cur_date = Datetime(199911300000);
    pre_date = Datetime(199911290000);
    next_date = Datetime(199912010000);
    BOOST_CHECK(tm->borrowStock(cur_date, stock, 26.28, 200) == true);
    BOOST_CHECK(tm->borrowStock(cur_date, stock, 26.42, 800) == true);
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(99510, 0, 0, 100000, 0, 0, 26392));
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(99610, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(99510, 0, 0, 100000, 0, 0, 26392));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(99510, 0, 0, 100000, 0, 0, 26392));

    cur_date = Datetime(199912010000);
    pre_date = Datetime(199911300000);
    next_date = Datetime(199912020000);
    BOOST_CHECK(tm->returnStock(cur_date, stock, 26.30, 500) == true);
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(99390, 0, 0, 100000, 0, 0, 13210));
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(99510, 0, 0, 100000, 0, 0, 26392));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(99390, 0, 0, 100000, 0, 0, 13210));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(99390, 0, 0, 100000, 0, 0, 13210));

    BOOST_CHECK(tm->returnStock(cur_date, stock, 26.30, 500) == true);
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(99330, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(99510, 0, 0, 100000, 0, 0, 26392));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(99330, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(99330, 0, 0, 100000, 0, 0, 0));

    /** @arg 19991207 分两次买入1000股，不跨记录，三次归还 */
    cur_date = Datetime(199912070000);
    pre_date = Datetime(199912060000);
    next_date = Datetime(199912080000);
    BOOST_CHECK(tm->borrowStock(cur_date, stock, 25.8, 600) == true);
    BOOST_CHECK(tm->borrowStock(cur_date, stock, 25.83, 400) == true);
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(99230, 0, 0, 100000, 0, 0, 25812));
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(99330, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(99230, 0, 0, 100000, 0, 0, 25812));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(99230, 0, 0, 100000, 0, 0, 25812));

    cur_date = Datetime(199912080000);
    pre_date = Datetime(199912070000);
    next_date = Datetime(199912090000);
    BOOST_CHECK(tm->returnStock(cur_date, stock, 25.50, 200) == true);
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(99170, 0, 0, 100000, 0, 0, 20652));
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(99230, 0, 0, 100000, 0, 0, 25812));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(99170, 0, 0, 100000, 0, 0, 20652));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(99170, 0, 0, 100000, 0, 0, 20652));

    BOOST_CHECK(tm->returnStock(cur_date, stock, 25.50, 400) == true);
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(99110, 0, 0, 100000, 0, 0, 10332));
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(99230, 0, 0, 100000, 0, 0, 25812));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(99110, 0, 0, 100000, 0, 0, 10332));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(99110, 0, 0, 100000, 0, 0, 10332));

    BOOST_CHECK(tm->returnStock(cur_date, stock, 25.50, 400) == true);
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(99050, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(99230, 0, 0, 100000, 0, 0, 25812));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(99050, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(99050, 0, 0, 100000, 0, 0, 0));
}


/** @par 检测点, 日线，忽略权息信息 */
BOOST_AUTO_TEST_CASE( test_TradeManager_trade_no_rights_by_day ) {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    Stock stock2 = sm.getStock("sz000001");

    CostRecord cost;
    TradeRecord trade;
    FundsRecord funds;
    TradeCostPtr tc = TC_TestStub();
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000, tc);
    tm->setParam<bool>("reinvest", false); //忽略权息
    tm->setParam<bool>("support_borrow_cash", false);
    tm->setParam<bool>("support_borrow_stock", false);

    /** @arg 19991117 27.18 stock 买入1000股 */
    Datetime cur_date, pre_date, next_date;
    cur_date = Datetime(199911170000);
    pre_date = Datetime(199911160000);
    next_date = Datetime(199911180000);
    trade = tm->buy(cur_date, stock, 27.18, 1000, 27.0, 27.18, 27.18);
    cost = tc->getBuyCost(cur_date, stock, 27.18, 1000);
    BOOST_CHECK(trade == TradeRecord(stock, cur_date, BUSINESS_BUY,
            27.18, 27.18, 27.18, 1000, cost, 27.0, 72810, PART_INVALID));
    BOOST_CHECK(tm->firstDatetime() == cur_date);
    BOOST_CHECK(tm->lastDatetime() == cur_date);
    BOOST_CHECK(std::fabs(tm->cash(cur_date) - 72810) < 0.001);
    BOOST_CHECK(tm->have(stock) == true);
    BOOST_CHECK(tm->getStockNumber() == 1);
    BOOST_CHECK(tm->getHoldNumber(pre_date, stock) == 0);
    BOOST_CHECK(tm->getHoldNumber(cur_date, stock) == 1000);
    BOOST_CHECK(tm->getHoldNumber(next_date, stock) == 1000);
    BOOST_CHECK(tm->getHoldNumber(next_date, stock2) == 0);
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(72810, 27180, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(100000, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(72810, 27180, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(72810, 27020, 0, 100000, 0, 0, 0));

    /** @arg 20000705 23.24 stock 买入1000股 */
    cur_date = Datetime(200007050000);
    pre_date = Datetime(200007040000);
    next_date = Datetime(200007060000);
    trade = tm->buy(cur_date, stock, 23.24, 1000, 23.11, 23.25, 23.23);
    cost = tc->getBuyCost(cur_date, stock, 23.24, 1000);
    BOOST_CHECK(trade == TradeRecord(stock, cur_date, BUSINESS_BUY,
            23.23, 23.24, 23.25, 1000, cost, 23.11, 49560, PART_INVALID));
    BOOST_CHECK(tm->lastDatetime() == cur_date);
    BOOST_CHECK(std::fabs(tm->cash(cur_date) - 49560) < 0.001);
    BOOST_CHECK(tm->have(stock) == true);
    BOOST_CHECK(tm->getStockNumber() == 1);
    BOOST_CHECK(tm->getHoldNumber(pre_date, stock) == 1000);
    BOOST_CHECK(tm->getHoldNumber(cur_date, stock) == 2000);
    BOOST_CHECK(tm->getHoldNumber(next_date, stock) == 2000);
    BOOST_CHECK(tm->getHoldNumber(next_date, stock2) == 0);
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(49560, 46440, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(72810, 23250, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(49560, 46440, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(49560, 46160, 0, 100000, 0, 0, 0));

    /** @arg 20000705 17.73 stock2 买入500股 */
    cur_date = Datetime(200007050000);
    pre_date = Datetime(200007040000);
    next_date = Datetime(200007060000);
    trade = tm->buy(cur_date, stock2, 17.73, 500, 17.56, 18.0, 17.70);
    cost = tc->getBuyCost(cur_date, stock2, 17.73, 500);
    BOOST_CHECK(trade == TradeRecord(stock2, cur_date, BUSINESS_BUY,
            17.70, 17.73, 18.0, 500, cost, 17.56, 40685, PART_INVALID));
    BOOST_CHECK(tm->lastDatetime() == cur_date);
    BOOST_CHECK(std::fabs(tm->cash(cur_date) - 40685) < 0.001);
    BOOST_CHECK(tm->have(stock2) == true);
    BOOST_CHECK(tm->getStockNumber() == 2);
    BOOST_CHECK(tm->getHoldNumber(pre_date, stock) == 1000);
    BOOST_CHECK(tm->getHoldNumber(cur_date, stock) == 2000);
    BOOST_CHECK(tm->getHoldNumber(next_date, stock) == 2000);
    BOOST_CHECK(tm->getHoldNumber(pre_date, stock2) == 0);
    BOOST_CHECK(tm->getHoldNumber(cur_date, stock2) == 500);
    BOOST_CHECK(tm->getHoldNumber(next_date, stock2) == 500);
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(72810, 23250, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(40685, 55275.0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(40685, 55275.0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(40685, 55020.0, 0, 100000, 0, 0, 0));

    /** @arg 20000705 17.61 stock2 买入300股 */
    cur_date = Datetime(200007050000);
    pre_date = Datetime(200007040000);
    next_date = Datetime(200007060000);
    trade = tm->buy(cur_date, stock2, 17.61, 300, 17.50, 17.62, 17.60);
    cost = tc->getBuyCost(cur_date, stock2, 17.61, 300);
    BOOST_CHECK(trade == TradeRecord(stock2, cur_date, BUSINESS_BUY,
            17.60, 17.61, 17.62, 300, cost, 17.50, 35392, PART_INVALID));
    BOOST_CHECK(tm->lastDatetime() == cur_date);
    BOOST_CHECK(std::fabs(tm->cash(cur_date) -35392) < 0.001);
    BOOST_CHECK(tm->have(stock2) == true);
    BOOST_CHECK(tm->getStockNumber() == 2);
    BOOST_CHECK(tm->getHoldNumber(pre_date, stock) == 1000);
    BOOST_CHECK(tm->getHoldNumber(cur_date, stock) == 2000);
    BOOST_CHECK(tm->getHoldNumber(next_date, stock) == 2000);
    BOOST_CHECK(tm->getHoldNumber(pre_date, stock2) == 0);
    BOOST_CHECK(tm->getHoldNumber(cur_date, stock2) == 800);
    BOOST_CHECK(tm->getHoldNumber(next_date, stock2) == 800);
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(72810, 23250, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(35392, 60576.0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(35392, 60576.0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(35392, 60336.0, 0, 100000, 0, 0, 0));

    /** @arg 20000706 存入50000元现金 */
    cur_date = Datetime(200007060000);
    pre_date = Datetime(200007050000);
    next_date = Datetime(200007070000);
    BOOST_CHECK(tm->checkin(cur_date, 50000) == true);
    BOOST_CHECK(tm->lastDatetime() == cur_date);
    BOOST_CHECK(std::fabs(tm->cash(cur_date) - 85392) < 0.001);
    BOOST_CHECK(tm->have(stock) == true);
    BOOST_CHECK(tm->have(stock2) == true);
    BOOST_CHECK(tm->getStockNumber() == 2);
    BOOST_CHECK(tm->getHoldNumber(pre_date, stock) == 2000);
    BOOST_CHECK(tm->getHoldNumber(cur_date, stock) == 2000);
    BOOST_CHECK(tm->getHoldNumber(next_date, stock) == 2000);
    BOOST_CHECK(tm->getHoldNumber(pre_date, stock2) == 800);
    BOOST_CHECK(tm->getHoldNumber(cur_date, stock2) == 800);
    BOOST_CHECK(tm->getHoldNumber(next_date, stock2) == 800);
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(35392, 60576.0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(85392, 60336.0, 0, 150000, 0, 0, 0));
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(85392, 60336.0, 0, 150000, 0, 0, 0));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(85392, 60852.0, 0, 150000, 0, 0, 0));

    /** @arg 20000707 取出20000元现金 */
    cur_date = Datetime(200007070000);
    pre_date = Datetime(200007060000);
    next_date = Datetime(200007080000);
    BOOST_CHECK(tm->checkout(cur_date, 20000) == true);
    BOOST_CHECK(tm->lastDatetime() == cur_date);
    BOOST_CHECK(std::fabs(tm->cash(cur_date) - 65392) < 0.001);
    BOOST_CHECK(tm->have(stock) == true);
    BOOST_CHECK(tm->have(stock2) == true);
    BOOST_CHECK(tm->getStockNumber() == 2);
    BOOST_CHECK(tm->getHoldNumber(pre_date, stock) == 2000);
    BOOST_CHECK(tm->getHoldNumber(cur_date, stock) == 2000);
    BOOST_CHECK(tm->getHoldNumber(next_date, stock) == 2000);
    BOOST_CHECK(tm->getHoldNumber(pre_date, stock2) == 800);
    BOOST_CHECK(tm->getHoldNumber(cur_date, stock2) == 800);
    BOOST_CHECK(tm->getHoldNumber(next_date, stock2) == 800);
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(85392, 60336.0, 0, 150000, 0, 0, 0));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(65392, 60852.0, 0, 130000, 0, 0, 0));
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(65392, 60852.0, 0, 130000, 0, 0, 0));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(65392, 60852.0, 0, 130000, 0, 0, 0));

    /** @arg 20000710 借入10000元现金 */
    cur_date = Datetime(200007100000);
    pre_date = Datetime(200007090000);
    next_date = Datetime(200007110000);
    BOOST_CHECK(tm->borrowCash(cur_date, 10000) == true);
    BOOST_CHECK(tm->lastDatetime() == cur_date);
    BOOST_CHECK(std::fabs(tm->cash(cur_date) - 75362) < 0.001);
    BOOST_CHECK(tm->have(stock) == true);
    BOOST_CHECK(tm->have(stock2) == true);
    BOOST_CHECK(tm->getStockNumber() == 2);
    BOOST_CHECK(tm->getHoldNumber(pre_date, stock) == 2000);
    BOOST_CHECK(tm->getHoldNumber(cur_date, stock) == 2000);
    BOOST_CHECK(tm->getHoldNumber(next_date, stock) == 2000);
    BOOST_CHECK(tm->getHoldNumber(pre_date, stock2) == 800);
    BOOST_CHECK(tm->getHoldNumber(cur_date, stock2) == 800);
    BOOST_CHECK(tm->getHoldNumber(next_date, stock2) == 800);
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(65392, 60852.0, 0, 130000, 0, 0, 0));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(75362, 60388.0, 0, 130000, 0, 10000, 0));
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(75362, 60388.0, 0, 130000, 0, 10000, 0));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(75362, 61344.0, 0, 130000, 0, 10000, 0));
    BOOST_CHECK(tm->getDebtCash(pre_date) == 0.0);
    BOOST_CHECK(tm->getDebtCash(cur_date) == 10000.0);
    BOOST_CHECK(tm->getDebtCash(next_date) == 10000.0);

    /** @arg 20000711 归还5000元现金 */
    cur_date = Datetime(200007110000);
    pre_date = Datetime(200007100000);
    next_date = Datetime(200007120000);
    BOOST_CHECK(tm->returnCash(cur_date, 5000) == true);
    BOOST_CHECK(tm->lastDatetime() == cur_date);
    BOOST_CHECK(std::fabs(tm->cash(cur_date) - 70322) < 0.001);
    BOOST_CHECK(tm->have(stock) == true);
    BOOST_CHECK(tm->have(stock2) == true);
    BOOST_CHECK(tm->getStockNumber() == 2);
    BOOST_CHECK(tm->getHoldNumber(pre_date, stock) == 2000);
    BOOST_CHECK(tm->getHoldNumber(cur_date, stock) == 2000);
    BOOST_CHECK(tm->getHoldNumber(next_date, stock) == 2000);
    BOOST_CHECK(tm->getHoldNumber(pre_date, stock2) == 800);
    BOOST_CHECK(tm->getHoldNumber(cur_date, stock2) == 800);
    BOOST_CHECK(tm->getHoldNumber(next_date, stock2) == 800);
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(75362, 60388.0, 0, 130000, 0, 10000, 0));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(70322, 61344.0, 0, 130000, 0, 5000, 0));
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(70322, 61344.0, 0, 130000, 0, 5000, 0));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(70322, 61768.0, 0, 130000, 0, 5000, 0));
    BOOST_CHECK(tm->getDebtCash(pre_date) == 10000.0);
    BOOST_CHECK(tm->getDebtCash(cur_date) == 5000.0);
    BOOST_CHECK(tm->getDebtCash(next_date) == 5000.0);

    /** @arg 20000712 存入sz000001,18.0, 200股 */
    cur_date = Datetime(200007120000);
    pre_date = Datetime(200007110000);
    next_date = Datetime(200007130000);
    BOOST_CHECK(tm->checkinStock(cur_date, stock2, 18.0, 200) == true);
    BOOST_CHECK(tm->lastDatetime() == cur_date);
    BOOST_CHECK(std::fabs(tm->cash(cur_date) - 70322) < 0.001);
    BOOST_CHECK(tm->have(stock) == true);
    BOOST_CHECK(tm->have(stock2) == true);
    BOOST_CHECK(tm->getStockNumber() == 2);
    BOOST_CHECK(tm->getHoldNumber(pre_date, stock) == 2000);
    BOOST_CHECK(tm->getHoldNumber(cur_date, stock) == 2000);
    BOOST_CHECK(tm->getHoldNumber(next_date, stock) == 2000);
    BOOST_CHECK(tm->getHoldNumber(pre_date, stock2) == 800);
    BOOST_CHECK(tm->getHoldNumber(cur_date, stock2) == 1000);
    BOOST_CHECK(tm->getHoldNumber(next_date, stock2) == 1000);
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(70322, 61344.0, 0, 130000, 0, 5000, 0));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(70322, 65410.0, 0, 130000, 3600, 5000, 0));
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(70322, 65410.0, 0, 130000, 3600, 5000, 0));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(70322, 64880.0, 0, 130000, 3600, 5000, 0));
    BOOST_CHECK(tm->getDebtCash(pre_date) == 5000.0);
    BOOST_CHECK(tm->getDebtCash(cur_date) == 5000.0);
    BOOST_CHECK(tm->getDebtCash(next_date) == 5000.0);

    /** @arg 20000712 存入sz000005,10.01, 1000股 */
    Stock stock3 = sm.getStock("sz000005");
    cur_date = Datetime(200007120000);
    pre_date = Datetime(200007110000);
    next_date = Datetime(200007130000);
    BOOST_CHECK(tm->checkinStock(cur_date, stock3, 10.01, 1000) == true);
    BOOST_CHECK(tm->lastDatetime() == cur_date);
    BOOST_CHECK(std::fabs(tm->cash(cur_date) - 70322) < 0.001);
    BOOST_CHECK(tm->have(stock) == true);
    BOOST_CHECK(tm->have(stock2) == true);
    BOOST_CHECK(tm->have(stock3) == true);
    BOOST_CHECK(tm->getStockNumber() == 3);
    BOOST_CHECK(tm->getHoldNumber(pre_date, stock) == 2000);
    BOOST_CHECK(tm->getHoldNumber(cur_date, stock) == 2000);
    BOOST_CHECK(tm->getHoldNumber(next_date, stock) == 2000);
    BOOST_CHECK(tm->getHoldNumber(pre_date, stock2) == 800);
    BOOST_CHECK(tm->getHoldNumber(cur_date, stock2) == 1000);
    BOOST_CHECK(tm->getHoldNumber(next_date, stock2) == 1000);
    BOOST_CHECK(tm->getHoldNumber(pre_date, stock3) == 0);
    BOOST_CHECK(tm->getHoldNumber(cur_date, stock3) == 1000);
    BOOST_CHECK(tm->getHoldNumber(next_date, stock3) == 1000);
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(70322, 61344.0, 0, 130000, 0, 5000, 0));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(70322, 75450.0, 0, 130000, 13610, 5000, 0));
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(70322, 75450.0, 0, 130000, 13610, 5000, 0));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(70322, 74830.0, 0, 130000, 13610, 5000, 0));
    BOOST_CHECK(tm->getDebtCash(pre_date) == 5000.0);
    BOOST_CHECK(tm->getDebtCash(cur_date) == 5000.0);
    BOOST_CHECK(tm->getDebtCash(next_date) == 5000.0);


    /** @arg 20000713 卖出全部持仓 */
    cur_date = Datetime(200007130000);
    pre_date = Datetime(200007120000);
    next_date = Datetime(200007140000);
    tm->sell(cur_date, stock, 23.44);
    tm->sell(cur_date, stock2, 18);
    tm->sell(cur_date, stock3, 9.95);
    BOOST_CHECK(tm->lastDatetime() == cur_date);
    BOOST_CHECK(std::fabs(tm->cash(cur_date) - 145092) < 0.001);
    BOOST_CHECK(tm->have(stock) == false);
    BOOST_CHECK(tm->have(stock2) == false);
    BOOST_CHECK(tm->have(stock3) == false);
    BOOST_CHECK(tm->getStockNumber() == 0);
    BOOST_CHECK(tm->getHoldNumber(pre_date, stock) == 2000);
    BOOST_CHECK(tm->getHoldNumber(cur_date, stock) == 0);
    BOOST_CHECK(tm->getHoldNumber(next_date, stock) == 0);
    BOOST_CHECK(tm->getHoldNumber(pre_date, stock2) == 1000);
    BOOST_CHECK(tm->getHoldNumber(cur_date, stock2) == 0);
    BOOST_CHECK(tm->getHoldNumber(next_date, stock2) == 0);
    BOOST_CHECK(tm->getHoldNumber(pre_date, stock3) == 1000);
    BOOST_CHECK(tm->getHoldNumber(cur_date, stock3) == 0);
    BOOST_CHECK(tm->getHoldNumber(next_date, stock3) == 0);
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(70322, 75450.0, 0, 130000, 13610, 5000, 0));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(145092, 0, 0, 130000, 13610, 5000, 0));
    funds = tm->getFunds(Null<Datetime>());
    BOOST_CHECK(funds == FundsRecord(145092, 0, 0, 130000, 13610, 5000, 0));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(145092, 0, 0, 130000, 13610, 5000, 0));
    BOOST_CHECK(tm->getDebtCash(pre_date) == 5000.0);
    BOOST_CHECK(tm->getDebtCash(cur_date) == 5000.0);
    BOOST_CHECK(tm->getDebtCash(next_date) == 5000.0);

    /** @arg 20000713 18.2 stock2 买入300股 */
    cur_date = Datetime(200007130000);
    pre_date = Datetime(200007120000);
    next_date = Datetime(200007140000);
    trade = tm->buy(cur_date, stock2, 18.2, 300, 17.50, 19.62, 18.20);
    cost = tc->getBuyCost(cur_date, stock2, 18.2, 300);
    BOOST_CHECK(trade == TradeRecord(stock2, cur_date, BUSINESS_BUY,
            18.2, 18.2, 19.62, 300, cost, 17.50, 139622, PART_INVALID));
    BOOST_CHECK(tm->lastDatetime() == cur_date);
    BOOST_CHECK(std::fabs(tm->cash(cur_date) - 139622) < 0.001);
    BOOST_CHECK(tm->have(stock) == false);
    BOOST_CHECK(tm->have(stock2) == true);
    BOOST_CHECK(tm->have(stock3) == false);
    BOOST_CHECK(tm->getStockNumber() == 1);
    BOOST_CHECK(tm->getHoldNumber(pre_date, stock) == 2000);
    BOOST_CHECK(tm->getHoldNumber(cur_date, stock) == 0);
    BOOST_CHECK(tm->getHoldNumber(next_date, stock) == 0);
    BOOST_CHECK(tm->getHoldNumber(pre_date, stock2) == 1000);
    BOOST_CHECK(tm->getHoldNumber(cur_date, stock2) == 300);
    BOOST_CHECK(tm->getHoldNumber(next_date, stock2) == 300);
    BOOST_CHECK(tm->getHoldNumber(pre_date, stock3) == 1000);
    BOOST_CHECK(tm->getHoldNumber(cur_date, stock3) == 0);
    BOOST_CHECK(tm->getHoldNumber(next_date, stock3) == 0);
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(70322, 75450, 0, 130000, 13610, 5000, 0));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(139622, 5400, 0, 130000, 13610, 5000, 0));
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(139622, 5400, 0, 130000, 13610, 5000, 0));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(139622, 5388.0, 0, 130000, 13610, 5000, 0));


    /** @arg 20000714 18.08 stock2 借入1000股 */
    cur_date = Datetime(200007140000);
    pre_date = Datetime(200007130000);
    next_date = Datetime(200007150000);
    BOOST_CHECK(tm->borrowStock(cur_date, stock2, 18.08, 1000) == true);
    BOOST_CHECK(tm->lastDatetime() == cur_date);
    BOOST_CHECK(std::fabs(tm->cash(cur_date) - 139572) < 0.001);
    BOOST_CHECK(tm->have(stock2) == true);
    BOOST_CHECK(tm->getStockNumber() == 1);
    BOOST_CHECK(tm->getHoldNumber(pre_date, stock2) == 300);
    BOOST_CHECK(tm->getHoldNumber(cur_date, stock2) == 300);
    BOOST_CHECK(tm->getHoldNumber(next_date, stock2) == 300);
    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(139622, 5400, 0, 130000, 13610, 5000, 0));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(139572, 5388, 0, 130000, 13610, 5000, 18080));
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(139572, 5388, 0, 130000, 13610, 5000, 18080));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(139572, 5388, 0, 130000, 13610, 5000, 18080));
    BOOST_CHECK(tm->getDebtNumber(pre_date, stock2) == 0);
    BOOST_CHECK(tm->getDebtNumber(cur_date, stock2) == 1000);
    BOOST_CHECK(tm->getDebtNumber(next_date, stock2) == 1000);


    /** @arg 20000714 18.06 stock2 卖空1000股 */
    cur_date = Datetime(200007140000);
    pre_date = Datetime(200007130000);
    next_date = Datetime(200007150000);
    trade = tm->sellShort(cur_date, stock2, 18.06, 1000, 18.5, 17.00, 18.08);
    cost = tm->getSellCost(cur_date, stock2, 18.06, 1000);
    BOOST_CHECK(trade == TradeRecord(stock2, cur_date, BUSINESS_SELL_SHORT,
            18.08, 18.06, 17, 1000, cost, 18.5, 157612, PART_INVALID));
    BOOST_CHECK(tm->lastDatetime() == cur_date);
    BOOST_CHECK(std::fabs(tm->cash(cur_date) - 157612) < 0.001);
    BOOST_CHECK(tm->have(stock2) == true);
    BOOST_CHECK(tm->haveShort(stock2) == true);
    BOOST_CHECK(tm->getStockNumber() == 1);
    BOOST_CHECK(tm->getShortStockNumber() == 1);
    BOOST_CHECK(tm->getHoldNumber(pre_date, stock2) == 300);
    BOOST_CHECK(tm->getHoldNumber(cur_date, stock2) == 300);
    BOOST_CHECK(tm->getHoldNumber(next_date, stock2) == 300);
    BOOST_CHECK(tm->getShortHoldNumber(pre_date, stock2) == 0);
    BOOST_CHECK(tm->getShortHoldNumber(cur_date, stock2) == 1000);
    BOOST_CHECK(tm->getShortHoldNumber(next_date, stock2) == 1000);

    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(139622, 5400, 0, 130000, 13610, 5000, 0));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(157612, 5388, 17960, 130000, 13610, 5000, 18080));
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(157612, 5388, 17960, 130000, 13610, 5000, 18080));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(157612, 5388, 17960, 130000, 13610, 5000, 18080));
    BOOST_CHECK(tm->getDebtNumber(pre_date, stock2) == 0);
    BOOST_CHECK(tm->getDebtNumber(cur_date, stock2) == 1000);
    BOOST_CHECK(tm->getDebtNumber(next_date, stock2) == 1000);


    /** @arg 20000717 17.8 stock2 空头买回1000股 */
    cur_date = Datetime(200007170000);
    pre_date = Datetime(200007160000);
    next_date = Datetime(200007180000);
    trade = tm->buyShort(cur_date, stock2, 17.8, 1000);
    cost = tm->getBuyCost(cur_date, stock2, 18.06, 1000);
    BOOST_CHECK(trade == TradeRecord(stock2, cur_date, BUSINESS_BUY_SHORT,
            0, 17.8, 0, 1000, cost, 0, 139802, PART_INVALID));
    BOOST_CHECK(tm->lastDatetime() == cur_date);
    BOOST_CHECK(std::fabs(tm->cash(cur_date) - 139802) < 0.001);
    BOOST_CHECK(tm->have(stock2) == true);
    BOOST_CHECK(tm->haveShort(stock2) == false);
    BOOST_CHECK(tm->getStockNumber() == 1);
    BOOST_CHECK(tm->getShortStockNumber() == 0);
    BOOST_CHECK(tm->getHoldNumber(pre_date, stock2) == 300);
    BOOST_CHECK(tm->getHoldNumber(cur_date, stock2) == 300);
    BOOST_CHECK(tm->getHoldNumber(next_date, stock2) == 300);
    BOOST_CHECK(tm->getShortHoldNumber(pre_date, stock2) == 1000);
    BOOST_CHECK(tm->getShortHoldNumber(cur_date, stock2) == 0);
    BOOST_CHECK(tm->getShortHoldNumber(next_date, stock2) == 0);

    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(157612, 5388, 17960, 130000, 13610, 5000, 18080));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(139802, 5343, 0, 130000, 13610, 5000, 18080));
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(139802, 5343, 0, 130000, 13610, 5000, 18080));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(139802, 5352, 0, 130000, 13610, 5000, 18080));
    BOOST_CHECK(tm->getDebtNumber(pre_date, stock2) == 1000);
    BOOST_CHECK(tm->getDebtNumber(cur_date, stock2) == 1000);
    BOOST_CHECK(tm->getDebtNumber(next_date, stock2) == 1000);

    /** @arg 20000717 18.08 归还stock2 1000股 */
    cur_date = Datetime(200007170000);
    pre_date = Datetime(200007160000);
    next_date = Datetime(200007180000);
    BOOST_CHECK(tm->returnStock(cur_date, stock2, 18.08, 1000) == true);
    BOOST_CHECK(tm->lastDatetime() == cur_date);
    BOOST_CHECK(std::fabs(tm->cash(cur_date) - 139742) < 0.001);
    BOOST_CHECK(tm->have(stock2) == true);
    BOOST_CHECK(tm->haveShort(stock2) == false);
    BOOST_CHECK(tm->getStockNumber() == 1);
    BOOST_CHECK(tm->getShortStockNumber() == 0);
    BOOST_CHECK(tm->getHoldNumber(pre_date, stock2) == 300);
    BOOST_CHECK(tm->getHoldNumber(cur_date, stock2) == 300);
    BOOST_CHECK(tm->getHoldNumber(next_date, stock2) == 300);
    BOOST_CHECK(tm->getShortHoldNumber(pre_date, stock2) == 1000);
    BOOST_CHECK(tm->getShortHoldNumber(cur_date, stock2) == 0);
    BOOST_CHECK(tm->getShortHoldNumber(next_date, stock2) == 0);

    funds = tm->getFunds(pre_date);
    BOOST_CHECK(funds == FundsRecord(157612, 5388, 17960, 130000, 13610, 5000, 18080));
    funds = tm->getFunds(cur_date);
    BOOST_CHECK(funds == FundsRecord(139742, 5343, 0, 130000, 13610, 5000, 0));
    funds = tm->getFunds();
    BOOST_CHECK(funds == FundsRecord(139742, 5343, 0, 130000, 13610, 5000, 0));
    funds = tm->getFunds(next_date);
    BOOST_CHECK(funds == FundsRecord(139742, 5352, 0, 130000, 13610, 5000, 0));
    BOOST_CHECK(tm->getDebtNumber(pre_date, stock2) == 1000);
    BOOST_CHECK(tm->getDebtNumber(cur_date, stock2) == 0);
    BOOST_CHECK(tm->getDebtNumber(next_date, stock2) == 0);
}


/** @par 检测点, 自动融资、融券操作， 日线，忽略权息信息 */
BOOST_AUTO_TEST_CASE( test_TradeManager_trade_financing_securities_lending_no_rights_by_day ) {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    Stock stock2 = sm.getStock("sz000001");

    CostRecord cost;
    TradeRecord trade;
    FundsRecord funds;
    TradeCostPtr tc = TC_TestStub();
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 10000, tc);
    tm->setParam<bool>("reinvest", false); //忽略权息
    tm->setParam<bool>("support_borrow_cash", true);
    tm->setParam<bool>("support_borrow_stock", true);

    Datetime cur_date, pre_date, next_date;

    /** @arg 19991110 27.75 买入1000股 */
    cur_date = Datetime(199911100000);
    pre_date = Datetime(199911090000);
    next_date = Datetime(199911110000);
    trade = tm->buy(cur_date, stock, 27.75, 1000, 27.70, 28.0, 27.75);
    cost = tm->getBuyCost(cur_date, stock, 27.75, 1000);

}


/** @par 检测点，测试 getTradeList */
BOOST_AUTO_TEST_CASE( test_getTradeList ) {
    StockManager& sm = StockManager::instance();
    Stock stk = sm.getStock("sz000001");

    CostRecord cost;

    TradeManagerPtr tm = crtTM(Datetime(199305010000), 100000);
    tm->setParam<bool>("reinvest", true);

    tm->buy(Datetime(199305200000L), stk, 55.7, 100);
    tm->buy(Datetime(199305250000L), stk, 27.5, 100);
    tm->buy(Datetime(199407110000L), stk, 8.55, 200);

    /** @arg 获取全部交易记录 */
    TradeRecordList tr_list = tm->getTradeList();
    BOOST_CHECK(tr_list.size() == 8);
    BOOST_CHECK(tr_list[0] == TradeRecord(Stock(), Datetime(199305010000L),
                                          BUSINESS_INIT, 100000, 100000, 0, 0,
                                          cost, 0, 100000, PART_INVALID));
    BOOST_CHECK(tr_list[7] == TradeRecord(stk, Datetime(199407110000L),
                                          BUSINESS_BUY, 0, 8.55, 0, 200,
                                          cost, 0, 90142.50, PART_INVALID));

    /** @arg 指定日期范围获取交易记录 start 等于账户建立日期，end 为  Null<Datetime>() */
    tr_list = tm->getTradeList(Datetime(199305010000), Null<Datetime>());

    BOOST_CHECK(tr_list.size() == 8);
    BOOST_CHECK(tr_list[0] == TradeRecord(Stock(), Datetime(199305010000L),
                                          BUSINESS_INIT, 100000, 100000, 0, 0,
                                          cost, 0, 100000, PART_INVALID));
    BOOST_CHECK(tr_list[7] == TradeRecord(stk, Datetime(199407110000L),
                                          BUSINESS_BUY, 0, 8.55, 0, 200,
                                          cost, 0, 90142.50, PART_INVALID));

    /** @arg 指定日期范围获取交易记录 start 等于第一条买入记录日期，end为Null */
    tr_list = tm->getTradeList(Datetime(199305200000L), Null<Datetime>());

    BOOST_CHECK(tr_list.size() == 7);
    BOOST_CHECK(tr_list[0] == TradeRecord(stk, Datetime(199305200000L),
                                          BUSINESS_BUY, 0, 55.70, 0, 100,
                                          cost, 0, 94430, PART_INVALID));
    BOOST_CHECK(tr_list[6] == TradeRecord(stk, Datetime(199407110000L),
                                          BUSINESS_BUY, 0, 8.55, 0, 200,
                                          cost, 0, 90142.50, PART_INVALID));

    /** @arg 指定日期范围获取交易记录 start 介于两个交易记录日期之间，end为Null */
    tr_list = tm->getTradeList(Datetime(199305210000L), Null<Datetime>());

    BOOST_CHECK(tr_list.size() == 6);
    BOOST_CHECK(tr_list[0] == TradeRecord(stk, Datetime(199305240000L),
                                          BUSINESS_BONUS, 30, 30, 0, 0,
                                          cost, 0, 94490, PART_INVALID));
    BOOST_CHECK(tr_list[5] == TradeRecord(stk, Datetime(199407110000L),
                                          BUSINESS_BUY, 0, 8.55, 0, 200,
                                          cost, 0, 90142.50, PART_INVALID));

    /** @arg 指定日期范围获取交易记录 start 大于 end */
    tr_list = tm->getTradeList(Null<Datetime>(), Datetime(199305210000L));
    BOOST_CHECK(tr_list.size() == 0);

    /** @arg 指定日期范围获取交易记录 start 等于 end */
    tr_list = tm->getTradeList(Datetime(199305210000L), Datetime(199305210000L));
    BOOST_CHECK(tr_list.size() == 0);

    /** @arg 指定日期范围获取交易记录 start等于某交易记录日期，end大于最后一条交易记录日期 */
    tr_list = tm->getTradeList(Datetime(199305200000L), Datetime(199407120000L));

    BOOST_CHECK(tr_list.size() == 7);
    BOOST_CHECK(tr_list[0] == TradeRecord(stk, Datetime(199305200000L),
                                          BUSINESS_BUY, 0, 55.70, 0, 100,
                                          cost, 0, 94430, PART_INVALID));
    BOOST_CHECK(tr_list[6] == TradeRecord(stk, Datetime(199407110000L),
                                          BUSINESS_BUY, 0, 8.55, 0, 200,
                                          cost, 0, 90142.50, PART_INVALID));

    /** @arg 指定日期范围获取交易记录 start等于某交易记录日期，end等于最后一条交易记录日期 */
    tr_list = tm->getTradeList(Datetime(199305200000L), Datetime(199407110000L));

    BOOST_CHECK(tr_list.size() == 4);
    BOOST_CHECK(tr_list[0] == TradeRecord(stk, Datetime(199305200000L),
                                          BUSINESS_BUY, 0, 55.70, 0, 100,
                                          cost, 0, 94430, PART_INVALID));
    BOOST_CHECK(tr_list[3] == TradeRecord(stk, Datetime(199305250000L),
                                          BUSINESS_BUY, 0, 27.5, 0, 100,
                                          cost, 0, 91710, PART_INVALID));
}

/** @par 检测点, 测试addTradeRecord */
BOOST_AUTO_TEST_CASE( test_TradeManager_addTradeRecord ) {
    StockManager& sm = StockManager::instance();
    Stock stk = sm.getStock("sz000001");

    CostRecord cost;

    TradeManagerPtr tm = crtTM(Datetime(199305010000), 100000);
    tm->setParam<bool>("reinvest", true);

    tm->buy(Datetime(199305200000L), stk, 55.7, 100);
    tm->buy(Datetime(199305250000L), stk, 27.5, 100);
    tm->buy(Datetime(199407110000L), stk, 8.55, 200);

    /** @arg 加入账户初始化交易记录 */
    TradeRecordList tr_list = tm->getTradeList();
    BOOST_CHECK(tr_list.size() == 8);
    BOOST_CHECK(tr_list[0] == TradeRecord(Stock(), Datetime(199305010000L),
                                          BUSINESS_INIT, 100000, 100000, 0, 0,
                                          cost, 0, 100000, PART_INVALID));

    TradeRecord tr(Stock(), Datetime(199201010000L),
                   BUSINESS_INIT, 200000, 200000, 0, 0,
                   cost, 0, 200000, PART_INVALID);
    tm->addTradeRecord(tr);

    tr_list = tm->getTradeList();
    BOOST_CHECK(tr_list.size() == 1);
    BOOST_CHECK(tr_list[0] == tr);

    /** @arg 复制一个tm的交易记录至另一个tm */
    tm = crtTM(Datetime(199305010000), 100000);
    tm->setParam<bool>("reinvest", true);
    tm->buy(Datetime(199305200000L), stk, 55.7, 100);
    tm->buy(Datetime(199305250000L), stk, 27.5, 100);
    tm->buy(Datetime(199407110000L), stk, 8.55, 200);
    tr_list = tm->getTradeList();

    TMPtr tm2 = crtTM(Datetime(199101010000), 100000, TC_Zero(), "TM2");
    tm2->setParam<bool>("reinvest", false);
    BOOST_CHECK(tm->initDatetime() != tm2->initDatetime());
    for (auto iter = tr_list.begin(); iter != tr_list.end(); ++iter) {
        tm2->addTradeRecord(*iter);
    }

    BOOST_CHECK(tm2->name() != tm->name());
    BOOST_CHECK(tm2->getParam<bool>("reinvest") != tm->getParam<bool>("reinvest"));
    BOOST_CHECK(tm2->initDatetime() == tm->initDatetime());
    BOOST_CHECK(tm2->lastDatetime() == tm->lastDatetime());
    BOOST_CHECK(tm2->currentCash() == tm->currentCash());

    tr_list = tm2->getTradeList();
    BOOST_CHECK(tr_list.size() == 8);
    BOOST_CHECK(tr_list[0] == TradeRecord(Stock(), Datetime(199305010000L),
                                          BUSINESS_INIT, 100000, 100000, 0, 0,
                                          cost, 0, 100000, PART_INVALID));
    BOOST_CHECK(tr_list[7] == TradeRecord(stk, Datetime(199407110000L),
                                          BUSINESS_BUY, 0, 8.55, 0, 200,
                                          cost, 0, 90142.50, PART_INVALID));

}

/** @} */
