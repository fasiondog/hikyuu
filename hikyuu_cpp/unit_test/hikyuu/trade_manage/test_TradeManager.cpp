/*
 * test_TradeManager.cpp
 *
 *  Created on: 2013-2-21
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/OrderBrokerBase.h>
#include <hikyuu/trade_manage/crt/TC_TestStub.h>
#include <hikyuu/trade_manage/crt/TC_FixedA.h>
#include <hikyuu/trade_manage/crt/crtTM.h>

#include <fstream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

using namespace hku;

namespace {

class CaptureOrderBroker final : public OrderBrokerBase {
public:
    void _buy(Datetime, const string&, const string&, price_t, double num, price_t, price_t,
              SystemPart, const string&) override {
        last_buy_num = num;
        buy_count++;
    }

    void _sell(Datetime, const string&, const string&, price_t, double num, price_t, price_t,
               SystemPart, const string&) override {
        last_sell_num = num;
        sell_count++;
    }

    double last_buy_num{0.0};
    double last_sell_num{0.0};
    size_t buy_count{0};
    size_t sell_count{0};
};

}  // namespace

/**
 * @defgroup test_TradeManager test_TradeManager
 * @ingroup test_hikyuu_trade_manage_suite
 * @{
 */

TEST_CASE("test_MAX_DOUBLE") {
    // Test whether the absolute value of a negative MAX_DOUBLE equals MAX_DOUBLE
    double x = -MAX_DOUBLE;
    CHECK_EQ(std::abs(x), MAX_DOUBLE);
}

/** @par Test points */
TEST_CASE("test_TradeManager_init") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    CostRecord result, expect;
    TradeManagerPtr tm =
      crtTM(Datetime(199901010000), 100000, TC_FixedA(0.0018, 5, 0.001, 0.001, 1.0), "TEST");

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
                         0.0, 0, Null<CostRecord>(), 0.0, 100000.0, PART_INVALID));
    CHECK_EQ(tm->getPositionList().empty(), true);
    CHECK_EQ(tm->getPosition(Datetime(199901010000), stock), Null<PositionRecord>());

    CHECK_EQ(tm->getShortPositionList().empty(), true);
    CHECK_EQ(tm->getShortPosition(stock), Null<PositionRecord>());
}

/** @par Test points */
TEST_CASE("test_TradeManager_getBuyCost") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    CostRecord result, expect;
    TradeManagerPtr tm =
      crtTM(Datetime(199901010000), 100000, TC_FixedA(0.0018, 5, 0.001, 0.001, 1.0), "TEST");

    /** @arg Whether calling CostFunc works */
    result = tm->getBuyCost(Datetime(200101010000), stock, 10.0, 1000);
    expect.commission = 18.0;
    expect.stamptax = 0.0;
    expect.transferfee = 1.0;
    expect.total = 19.0;
    CHECK_EQ(result, expect);
}

/** @par Test points */
TEST_CASE("test_TradeManager_getSellCost") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600004");
    CostRecord result, expect;
    TradeManagerPtr tm =
      crtTM(Datetime(199901010000), 100000, TC_FixedA(0.0018, 5, 0.001, 0.001, 1.0));

    /** @arg Whether calling CostFunc works */
    result = tm->getSellCost(Datetime(200101010000), stock, 10.0, 100);
    expect.commission = 5.0;
    expect.stamptax = 1.0;
    expect.transferfee = 1.0;
    expect.total = 7.0;
    CHECK_EQ(result, expect);
}

/** @par Test points */
TEST_CASE("test_TradeManager_can_not_buy") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeCostPtr costfunc = TC_FixedA(0.0018, 5, 0.001, 0.001, 1.0);
    TradeManagerPtr tm;
    TradeRecord result;
    CostRecord cost;
    TradeRecord trade;
    TradeRecordList trade_list;

    /** @arg The initial balance is 0, no trade yet and the ex-rights/ex-dividend data is ignored */
    tm = crtTM(Datetime(199901010000), 0, costfunc, "SYS");
    result = tm->buy(Datetime(199911180000), stock, 27.2, 100, 0, 27.2, 27.2);
    CHECK_EQ(result, Null<TradeRecord>());

    /** @arg The initial balance is 100000 and a Null<Stock> is operated on */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->buy(Datetime(199911180000), Null<Stock>(), 27.2, 100, 0, 27.2, 27.2);
    CHECK_EQ(result, Null<TradeRecord>());
    CHECK_EQ(tm->cash(Datetime(199911180000)), 100000);

    /** @arg Try to buy before the initial position building day */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->buy(Datetime(199001010000), stock, 26.36, 100, 0, 26.36, 26.36);
    CHECK_EQ(result, Null<TradeRecord>());

#if 0  // This restriction was removed
    /** @arg The initial balance is 100000 with no trade and the data ignored, but the security cannot be traded on that date, e.g. a non-trading day */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->buy(Datetime(199911130000), stock, 27.2, 100, 0, 27.2, 27.2);
    CHECK_EQ(result, Null<TradeRecord>());

    /** @arg The initial balance is 100000 with no trade and the data ignored, but the buy price exceeds the high price of the day */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->buy(Datetime(199911170000), stock, 27.2, 100, 0, 27.2, 27.2);
    CHECK_EQ(result, Null<TradeRecord>());
#endif

    /** @arg The initial balance is 100000 with no trade and the data ignored, but the buy price
     * equals the high price of the day */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->buy(Datetime(199911170000), stock, 27.18, 100, 0, 27.18, 27.18);
    cost = tm->getBuyCost(Datetime(199911170000), stock, 27.18, 100);
    trade = TradeRecord(stock, Datetime(199911170000), BUSINESS_BUY, 27.18, 27.18, 27.18, 100, cost,
                        0.0, 100000 - cost.total - 27.18 * 100, PART_INVALID);
    CHECK_EQ(result, trade);
    CHECK_EQ(tm->cash(Datetime(199911170000)), trade.cash);
    trade_list = tm->getTradeList();
    CHECK_EQ(trade_list.size(), 2);
    CHECK_EQ(trade_list[1], trade);

#if 0
    /** @arg The initial balance is 100000 with no trade and the data ignored, but the buy price is lower than the low price of the day */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->buy(Datetime(199911170000), stock, 26.36, 100, 0, 26.36, 26.36);
    CHECK_EQ(result, Null<TradeRecord>());
#endif

    /** @arg The initial balance is 100000 with no trade and the data ignored, but the buy price
     * equals the low price of the day */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->buy(Datetime(199911160000), stock, 26.48, 100, 0, 26.48, 26.48);
    cost = tm->getBuyCost(Datetime(199911160000), stock, 26.48, 100);
    CHECK_EQ(result, TradeRecord(stock, Datetime(199911160000), BUSINESS_BUY, 26.48, 26.48, 26.48,
                                 100, cost, 0.0, 100000 - cost.total - 26.48 * 100, PART_INVALID));

    /** @arg Try to trade before the last trade time */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->buy(Datetime(199911170000), stock, 26.48, 100, 0, 26.48, 26.48);
    CHECK_UNARY(!(result == Null<TradeRecord>()));
    result = tm->buy(Datetime(199911160000), stock, 26.48, 100, 0, 26.48, 26.48);
    CHECK_EQ(result, Null<TradeRecord>());

    /** @arg Try to buy a quantity of 0 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->buy(Datetime(199911170000), stock, 26.48, 0, 0, 26.48, 26.48);
    CHECK_EQ(result, Null<TradeRecord>());

    /** @arg The buy quantity is less than the minimum trade quantity */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result =
      tm->buy(Datetime(199911170000), stock, 26.48, stock.minTradeNumber() - 1, 0, 26.48, 26.48);
    CHECK_EQ(result, Null<TradeRecord>());

    /** @arg The buy quantity is greater than the maximum trade quantity */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result =
      tm->buy(Datetime(199911170000), stock, 26.48, stock.maxTradeNumber() - 1, 0, 26.48, 26.48);
    CHECK_EQ(result, Null<TradeRecord>());
}

/** @par Test points */
TEST_CASE("test_TradeManager_can_not_sell") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeCostPtr costfunc = TC_FixedA(0.0018, 5, 0.001, 0.001, 1.0);
    TradeManagerPtr tm;
    TradeRecord result;
    CostRecord cost;
    TradeRecord trade;
    TradeRecordList trade_list;

    /** @arg The initial balance is 0, no trade yet and the ex-rights/ex-dividend data is ignored */
    tm = crtTM(Datetime(199901010000), 0, costfunc, "SYS");
    result = tm->sell(Datetime(199911180000), stock, 27.2, 100);
    CHECK_EQ(result, Null<TradeRecord>());

    /** @arg The initial balance is 100000 and a Null<Stock> is operated on */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->sell(Datetime(199911180000), Null<Stock>(), 27.2, 100);
    CHECK_EQ(result, Null<TradeRecord>());
    CHECK_EQ(tm->cash(Datetime(199911180000)), 100000);

    /** @arg Try to sell before the last trading day */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->buy(Datetime(199911170000), stock, 27.18, 100, 0);
    CHECK_EQ(tm->getHoldNumber(Datetime(199911170000), stock), 100);
    result = tm->sell(Datetime(199801010000), stock, 26.36, 100);
    CHECK_EQ(result, Null<TradeRecord>());

    /** @arg The sell quantity is 0 */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->buy(Datetime(199911170000), stock, 27.18, 100, 0);
    CHECK_EQ(tm->getHoldNumber(Datetime(199911170000), stock), 100);
    result = tm->sell(Datetime(199911180000), stock, 26.36, 0);
    CHECK_EQ(result, Null<TradeRecord>());

    /** @arg The sell quantity is less than the minimum trade quantity */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->buy(Datetime(199911170000), stock, 27.18, 100, 0);
    CHECK_EQ(tm->getHoldNumber(Datetime(199911170000), stock), 100);
    result = tm->sell(Datetime(199911180000), stock, 26.36, stock.minTradeNumber() - 1);
    CHECK_EQ(result, Null<TradeRecord>());

    /** @arg The sell quantity is greater than the maximum trade quantity */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->buy(Datetime(199911170000), stock, 27.18, 100, 0);
    CHECK_EQ(tm->getHoldNumber(Datetime(199911170000), stock), 100);
    result = tm->sell(Datetime(199911180000), stock, 26.36, stock.maxTradeNumber() + 1);
    CHECK_EQ(result, Null<TradeRecord>());

    /** @arg Sell a stock that is not held */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->sell(Datetime(199901020000), stock, 26.36, 100);
    CHECK_EQ(result, Null<TradeRecord>());

    /** @arg The sell quantity is greater than the current position quantity */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->buy(Datetime(199911170000), stock, 27.18, 100, 0);
    CHECK_EQ(tm->getHoldNumber(Datetime(199911170000), stock), 100);
    result = tm->sell(Datetime(199911180000), stock, 26.36, 101);
    CHECK_EQ(result, Null<TradeRecord>());

    /** @arg Ignoring the ex-rights/ex-dividend data, sell all the bought stocks */
    tm = crtTM(Datetime(199901010000), 100000, costfunc, "SYS");
    result = tm->buy(Datetime(199911170000), stock, 27.18, 100, 0, 27.18, 27.18);
    cost = tm->getBuyCost(Datetime(199911170000), stock, 27.18, 100);
    CHECK_EQ(result, TradeRecord(stock, Datetime(199911170000), BUSINESS_BUY, 27.18, 27.18, 27.18,
                                 100, cost, 0.0, 100000 - cost.total - 27.18 * 100, PART_INVALID));
    CHECK_EQ(tm->getHoldNumber(Datetime(199911170000), stock), 100);
    CHECK_EQ(tm->getStockNumber(), 1);
    CHECK_EQ(tm->cash(Datetime(199911170000)), 97276.0);
    result = tm->sell(Datetime(199911180000), stock, 26.36, std::numeric_limits<double>::max(), 0,
                      0, 26.36);
    cost = tm->getSellCost(Datetime(199911180000), stock, 26.36, 100);
    CHECK_EQ(result, TradeRecord(stock, Datetime(199911180000), BUSINESS_SELL, 26.36, 26.36, 0.0,
                                 100, cost, 0.0, 99903.36, PART_INVALID));
    CHECK_EQ(tm->getHoldNumber(Datetime(199911180000), stock), 0);
    CHECK_EQ(tm->getStockNumber(), 0);
    CHECK_EQ(tm->cash(Datetime(199911180000)), 99903.36);

    /** @arg With the ex-rights/ex-dividend data, buy and sell the stock and ignore the trade cost
     */
    tm = crtTM(Datetime(199901010000), 1000000, TC_Zero(), "SYS");
    tm->buy(Datetime(199911170000), stock, 27.18, 1000, 0);
    CHECK_EQ(tm->cash(Datetime(199911170000)), 972820);
    tm->sell(Datetime(200605150000), stock, 10.2, 100);
    CHECK_EQ(tm->cash(Datetime(200605150000)), 974685);  // 973840);
    CHECK_EQ(tm->getHoldNumber(Datetime(200605160000), stock), 1850);
    tm->sell(Datetime(200612010000), stock, 16.87, std::numeric_limits<double>::max());
    CHECK_EQ(tm->cash(Datetime(200612010000)), 1006135.0);  // 1005049.5);
}

/** @par Test points */
TEST_CASE("test_TradeManager_can_not_checkin") {
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000);

    /** @arg Try to deposit an amount <= 0 */
    CHECK_EQ(tm->checkin(Datetime(199901020000), 0), false);
    CHECK_EQ(tm->checkin(Datetime(199901020000), -0.01), false);
    CHECK_EQ(tm->checkin(Datetime(199901020000), 0.01), true);

    /** @arg Try to withdraw before the last trading date */
    tm->checkin(Datetime(200001020000), 10000);
    CHECK_EQ(tm->checkin(Datetime(200001010000), 200), false);
}

/** @par Test points */
TEST_CASE("test_TradeManager_can_not_checkout") {
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000);

    /** @arg Try to withdraw an amount <= 0 */
    CHECK_EQ(tm->checkout(Datetime(199901020000), 0), false);
    CHECK_EQ(tm->checkout(Datetime(199901020000), -0.01), false);
    CHECK_EQ(tm->checkout(Datetime(199901020000), 0.01), true);

    /** @arg Try to withdraw before the last trading date */
    tm->checkin(Datetime(200001020000), 0.01);
    CHECK_EQ(tm->checkout(Datetime(200001010000), 200), false);

    /** @arg The amount to withdraw is greater than the current balance */
    CHECK_EQ(tm->currentCash(), 100000);
    CHECK_EQ(tm->checkout(Datetime(200001030000), 100000.01), false);
    CHECK_EQ(tm->checkout(Datetime(200001030000), 100000), true);
}

/** @par Test points */
TEST_CASE("test_TradeManager_can_not_borrowCash") {
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000);

    /** @arg Try to deposit an amount <= 0 */
    CHECK_EQ(tm->borrowCash(Datetime(199901020000), 0), false);
    CHECK_EQ(tm->borrowCash(Datetime(199901020000), -0.01), false);
    CHECK_EQ(tm->borrowCash(Datetime(199901020000), 0.01), true);

    /** @arg Try to withdraw before the last trading date */
    tm->checkin(Datetime(200001020000), 10000);
    CHECK_EQ(tm->borrowCash(Datetime(200001010000), 200), false);
}

/** @par Test points */
TEST_CASE("test_TradeManager_can_not_returnCash") {
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000);

    /** @arg Try to operate before the last trading date */
    tm->borrowCash(Datetime(200001020000), 50000);
    CHECK_EQ(tm->returnCash(Datetime(200001010000), 200), false);

    /** @arg Try to return an amount <= 0 */
    CHECK_EQ(tm->returnCash(Datetime(200001020000), 0), false);
    CHECK_EQ(tm->returnCash(Datetime(200001020000), -0.01), false);
    CHECK_EQ(tm->returnCash(Datetime(200001020000), 0.01), true);
    CHECK_EQ(tm->borrowCash(Datetime(200001020000), 0.01), true);

    /** @arg The amount to return is greater than the current debt */
    CHECK_EQ(tm->getDebtCash(Datetime(200001030000)), 50000);
    CHECK_EQ(tm->returnCash(Datetime(200001030000), 50000.01), false);
    CHECK_EQ(tm->returnCash(Datetime(200001030000), 50000), true);

    /** @arg The amount to return is greater than the current balance */
    tm->borrowCash(Datetime(200001040000), 50000);
    tm->checkout(Datetime(200001040000), 120000);
    CHECK_EQ(tm->currentCash(), 30000);
    CHECK_EQ(tm->getDebtCash(Datetime(200001040000)), 50000);
    CHECK_EQ(tm->returnCash(Datetime(200001040000), 50000), false);
}

/** @par Test points */
TEST_CASE("test_TradeManager_can_not_checkinStock") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000);

    /** @arg The stock to deposit is null */
    CHECK_EQ(tm->checkinStock(Datetime(199901020000), Stock(), 10, 100), false);

    /** @arg The quantity to deposit is 0 */
    CHECK_EQ(tm->checkinStock(Datetime(199901020000), stock, 10, 0), false);

    /** @arg The amount to deposit is <= 0 */
    CHECK_EQ(tm->checkinStock(Datetime(199901020000), stock, 0, 100), false);
    CHECK_EQ(tm->checkinStock(Datetime(199901020000), stock, -0.01, 100), false);

    /** @arg Try to withdraw before the last trading date */
    tm->checkin(Datetime(200001020000), 10000);
    CHECK_EQ(tm->checkinStock(Datetime(200001010000), stock, 10.0, 200), false);
}

/** @par Test points */
TEST_CASE("test_TradeManager_can_not_checkoutStock") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000);

    /** @arg The stock to withdraw is null */
    CHECK_EQ(tm->checkoutStock(Datetime(199901020000), Stock(), 10, 100), false);

    /** @arg The quantity to withdraw is 0 */
    CHECK_EQ(tm->checkinStock(Datetime(199901020000), stock, 10, 100), true);
    CHECK_EQ(tm->checkoutStock(Datetime(199901020000), stock, 10, 0), false);

    /** @arg The amount to withdraw is <= 0 */
    CHECK_EQ(tm->checkoutStock(Datetime(199901020000), stock, 0, 100), false);
    CHECK_EQ(tm->checkoutStock(Datetime(199901020000), stock, -0.01, 100), false);

    /** @arg Try to withdraw before the last trading date */
    CHECK_EQ(tm->checkinStock(Datetime(199901010000), stock, 10.0, 200), false);
}

TEST_CASE("test_TradeManager_short_orders_use_executed_number") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000, TC_Zero());
    auto broker = std::make_shared<CaptureOrderBroker>();
    tm->setBrokerLastDatetime(Datetime(199901010000));
    tm->regBroker(broker);

    Datetime sell_datetime(199911170000);
    CHECK_EQ(tm->borrowStock(sell_datetime, stock, 10.0, 100), true);
    TradeRecord sell_record = tm->sellShort(sell_datetime, stock, 10.0, 200);
    CHECK_EQ(sell_record.number, 100);
    CHECK_EQ(broker->sell_count, 1);
    CHECK_EQ(broker->last_sell_num, sell_record.number);

    Datetime buy_datetime(199911180000);
    TradeRecord buy_record = tm->buyShort(buy_datetime, stock, 10.0, MAX_DOUBLE);
    CHECK_EQ(buy_record.number, 100);
    CHECK_EQ(broker->buy_count, 1);
    CHECK_EQ(broker->last_buy_num, buy_record.number);
}

/** @par Test points */
TEST_CASE("test_TradeManager_can_not_borrowStock") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000);

    /** @arg The stock to borrow is null */
    CHECK_EQ(tm->borrowStock(Datetime(199901020000), Stock(), 10, 100), false);

    /** @arg The quantity to borrow is 0 */
    CHECK_EQ(tm->borrowStock(Datetime(199901020000), stock, 10, 0), false);

    /** @arg The amount to borrow is <= 0 */
    CHECK_EQ(tm->borrowStock(Datetime(199901020000), stock, 0, 100), false);
    CHECK_EQ(tm->borrowStock(Datetime(199901020000), stock, -0.01, 100), false);

    /** @arg Try to borrow before the last trading date */
    tm->checkin(Datetime(200001020000), 10000);
    CHECK_EQ(tm->borrowStock(Datetime(200001010000), stock, 10.0, 200), false);
}

/** @par Test points */
TEST_CASE("test_TradeManager_can_not_returnStock") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000);

    CHECK_EQ(tm->borrowStock(Datetime(199901020000), stock, 10, 100), true);

    /** @arg The stock to return is null */
    CHECK_EQ(tm->returnStock(Datetime(199901020000), Stock(), 10, 100), false);

    /** @arg The quantity to return is 0 */
    CHECK_EQ(tm->returnStock(Datetime(199901020000), stock, 10, 0), false);

    /** @arg The amount to return is <= 0 */
    CHECK_EQ(tm->returnStock(Datetime(199901020000), stock, 0, 100), false);
    CHECK_EQ(tm->returnStock(Datetime(199901020000), stock, -0.01, 100), false);

    /** @arg Try to return before the last trading date */
    CHECK_EQ(tm->returnStock(Datetime(199901010000), stock, 10.0, 100), false);

    /** @arg The quantity to return is greater than the borrowed one */
    CHECK_EQ(tm->returnStock(Datetime(199901030000), stock, 10.0, 101), false);
    CHECK_EQ(tm->returnStock(Datetime(199901030000), stock, 10.0, 100), true);
}

/** @par Test point: multiple borrows and returns of cash */
TEST_CASE("test_TradeManager_trade_multi_borrow_cash_by_day") {
    FundsRecord funds;
    TradeCostPtr tc = TC_TestStub();
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000, tc);

    Datetime cur_date, pre_date, next_date;

    /** @arg 19991117: borrow 5000 and return it in 2 times */
    cur_date = Datetime(199911170000);
    pre_date = Datetime(199911160000);
    next_date = Datetime(199911180000);
    CHECK_EQ(tm->borrowCash(cur_date, 5000), true);
    funds = tm->getFunds(pre_date);
    CHECK_EQ(funds, FundsRecord(100000, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(cur_date);
    CHECK_EQ(funds, FundsRecord(104970, 0, 0, 100000, 0, 5000, 0));
    funds = tm->getFunds(next_date);
    CHECK_EQ(funds, FundsRecord(104970, 0, 0, 100000, 0, 5000, 0));

    cur_date = Datetime(199911180000);
    pre_date = Datetime(199911170000);
    next_date = Datetime(199911190000);
    CHECK_EQ(tm->returnCash(cur_date, 3000), true);
    funds = tm->getFunds(pre_date);
    CHECK_EQ(funds, FundsRecord(104970, 0, 0, 100000, 0, 5000, 0));
    funds = tm->getFunds(cur_date);
    CHECK_EQ(funds, FundsRecord(101930, 0, 0, 100000, 0, 2000, 0));
    funds = tm->getFunds(next_date);
    CHECK_EQ(funds, FundsRecord(101930, 0, 0, 100000, 0, 2000, 0));

    CHECK_EQ(tm->returnCash(cur_date, 2000.01), false);
    CHECK_EQ(tm->returnCash(cur_date, 2000), true);
    funds = tm->getFunds(pre_date);
    CHECK_EQ(funds, FundsRecord(104970, 0, 0, 100000, 0, 5000, 0));
    funds = tm->getFunds(cur_date);
    CHECK_EQ(funds, FundsRecord(99890, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(next_date);
    CHECK_EQ(funds, FundsRecord(99890, 0, 0, 100000, 0, 0, 0));

    /** @arg Borrow 5000 in two times and return it once */
    cur_date = Datetime(199911190000);
    pre_date = Datetime(199911180000);
    next_date = Datetime(199911200000);
    CHECK_EQ(tm->borrowCash(cur_date, 3000), true);
    CHECK_EQ(tm->borrowCash(cur_date, 2000), true);
    funds = tm->getFunds(pre_date);
    CHECK_EQ(funds, FundsRecord(99890, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(cur_date);
    CHECK_EQ(funds, FundsRecord(104830, 0, 0, 100000, 0, 5000, 0));
    funds = tm->getFunds(next_date);
    CHECK_EQ(funds, FundsRecord(104830, 0, 0, 100000, 0, 5000, 0));

    CHECK_EQ(tm->returnCash(cur_date, 5000.01), false);
    CHECK_EQ(tm->returnCash(cur_date, 5000), true);
    funds = tm->getFunds(pre_date);
    CHECK_EQ(funds, FundsRecord(99890, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(cur_date);
    CHECK_EQ(funds, FundsRecord(99750, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(next_date);
    CHECK_EQ(funds, FundsRecord(99750, 0, 0, 100000, 0, 0, 0));

    /** @arg Borrow 5000 in two times and return it in two times across records */
    tm->reset();
    cur_date = Datetime(199911200000);
    pre_date = Datetime(199911190000);
    next_date = Datetime(199911210000);
    CHECK_EQ(tm->borrowCash(cur_date, 3000), true);
    CHECK_EQ(tm->borrowCash(cur_date, 2000), true);
    funds = tm->getFunds(pre_date);
    CHECK_EQ(funds, FundsRecord(100000, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(cur_date);
    CHECK_EQ(funds, FundsRecord(104940, 0, 0, 100000, 0, 5000, 0));
    funds = tm->getFunds(next_date);
    CHECK_EQ(funds, FundsRecord(104940, 0, 0, 100000, 0, 5000, 0));

    CHECK_EQ(tm->returnCash(cur_date, 4000), true);
    funds = tm->getFunds(pre_date);
    CHECK_EQ(funds, FundsRecord(100000, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(cur_date);
    CHECK_EQ(funds, FundsRecord(100860, 0, 0, 100000, 0, 1000, 0));
    funds = tm->getFunds(next_date);
    CHECK_EQ(funds, FundsRecord(100860, 0, 0, 100000, 0, 1000, 0));

    CHECK_EQ(tm->returnCash(cur_date, 1000.01), false);
    CHECK_EQ(tm->returnCash(cur_date, 1000), true);
    funds = tm->getFunds(pre_date);
    CHECK_EQ(funds, FundsRecord(100000, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(cur_date);
    CHECK_EQ(funds, FundsRecord(99820, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(next_date);
    CHECK_EQ(funds, FundsRecord(99820, 0, 0, 100000, 0, 0, 0));
}

/** @par Test point: multiple borrows and returns of stocks */
TEST_CASE("test_TradeManager_trade_multi_borrow_stock_by_day") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    Stock stock2 = sm.getStock("sz000001");

    CostRecord cost;
    TradeRecord trade;
    FundsRecord funds;
    TradeCostPtr tc = TC_TestStub();
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000, tc);
    tm->setParam<bool>("support_borrow_cash", false);
    tm->setParam<bool>("support_borrow_stock", false);

    Datetime cur_date, pre_date, next_date;

    /** @arg 19991117: buy 1000 shares once and return them in two records */
    cur_date = Datetime(199911170000);
    pre_date = Datetime(199911160000);
    next_date = Datetime(199911180000);
    CHECK_EQ(tm->borrowStock(cur_date, stock, 27.18, 1000), true);
    funds = tm->getFunds(pre_date);
    CHECK_EQ(funds, FundsRecord(100000, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(cur_date);
    CHECK_EQ(funds, FundsRecord(99950, 0, 0, 100000, 0, 0, 27180));
    funds = tm->getFunds(next_date);
    CHECK_EQ(funds, FundsRecord(99950, 0, 0, 100000, 0, 0, 27180));

    cur_date = Datetime(199911180000);
    pre_date = Datetime(199911170000);
    next_date = Datetime(199911190000);
    CHECK_EQ(tm->returnStock(cur_date, stock, 27.1, 800), true);
    funds = tm->getFunds(pre_date);
    CHECK_EQ(funds, FundsRecord(99950, 0, 0, 100000, 0, 0, 27180));
    funds = tm->getFunds(cur_date);
    CHECK_EQ(funds, FundsRecord(99890, 0, 0, 100000, 0, 0, 5436));
    funds = tm->getFunds(next_date);
    CHECK_EQ(funds, FundsRecord(99890, 0, 0, 100000, 0, 0, 5436));

    CHECK_EQ(tm->returnStock(cur_date, stock, 26.8, 200), true);
    funds = tm->getFunds(pre_date);
    CHECK_EQ(funds, FundsRecord(99950, 0, 0, 100000, 0, 0, 27180));
    funds = tm->getFunds(cur_date);
    CHECK_EQ(funds, FundsRecord(99830, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(next_date);
    CHECK_EQ(funds, FundsRecord(99830, 0, 0, 100000, 0, 0, 0));

    /** @arg 19991123: buy 1000 shares in two times and return them once */
    cur_date = Datetime(199911230000);
    pre_date = Datetime(199911220000);
    next_date = Datetime(199911240000);
    CHECK_EQ(tm->borrowStock(cur_date, stock, 26.21, 200), true);
    CHECK_EQ(tm->borrowStock(cur_date, stock, 26.43, 800), true);
    funds = tm->getFunds(pre_date);
    CHECK_EQ(funds, FundsRecord(99830, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(cur_date);
    CHECK_EQ(funds, FundsRecord(99730, 0, 0, 100000, 0, 0, 26386));
    funds = tm->getFunds(next_date);
    CHECK_EQ(funds, FundsRecord(99730, 0, 0, 100000, 0, 0, 26386));

    cur_date = Datetime(199911240000);
    pre_date = Datetime(199911230000);
    next_date = Datetime(199911250000);
    CHECK_EQ(tm->returnStock(cur_date, stock, 26.20, 1000), true);
    funds = tm->getFunds(pre_date);
    CHECK_EQ(funds, FundsRecord(99730, 0, 0, 100000, 0, 0, 26386));
    funds = tm->getFunds(cur_date);
    CHECK_EQ(funds, FundsRecord(99610, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(next_date);
    CHECK_EQ(funds, FundsRecord(99610, 0, 0, 100000, 0, 0, 0));

    /** @arg 19991123: buy 1000 shares in two times, return them in two times across records */
    cur_date = Datetime(199911300000);
    pre_date = Datetime(199911290000);
    next_date = Datetime(199912010000);
    CHECK_EQ(tm->borrowStock(cur_date, stock, 26.28, 200), true);
    CHECK_EQ(tm->borrowStock(cur_date, stock, 26.42, 800), true);
    funds = tm->getFunds(pre_date);
    CHECK_EQ(funds, FundsRecord(99610, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(cur_date);
    CHECK_EQ(funds, FundsRecord(99510, 0, 0, 100000, 0, 0, 26392));
    funds = tm->getFunds(next_date);
    CHECK_EQ(funds, FundsRecord(99510, 0, 0, 100000, 0, 0, 26392));

    cur_date = Datetime(199912010000);
    pre_date = Datetime(199911300000);
    next_date = Datetime(199912020000);
    CHECK_EQ(tm->returnStock(cur_date, stock, 26.30, 500), true);
    funds = tm->getFunds(pre_date);
    CHECK_EQ(funds, FundsRecord(99510, 0, 0, 100000, 0, 0, 26392));
    funds = tm->getFunds(cur_date);
    CHECK_EQ(funds, FundsRecord(99390, 0, 0, 100000, 0, 0, 13210));
    funds = tm->getFunds(next_date);
    CHECK_EQ(funds, FundsRecord(99390, 0, 0, 100000, 0, 0, 13210));

    CHECK_EQ(tm->returnStock(cur_date, stock, 26.30, 500), true);
    funds = tm->getFunds(pre_date);
    CHECK_EQ(funds, FundsRecord(99510, 0, 0, 100000, 0, 0, 26392));
    funds = tm->getFunds(cur_date);
    CHECK_EQ(funds, FundsRecord(99330, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(next_date);
    CHECK_EQ(funds, FundsRecord(99330, 0, 0, 100000, 0, 0, 0));

    /** @arg 19991207: buy 1000 shares in two times and return them in three times in one record */
    cur_date = Datetime(199912070000);
    pre_date = Datetime(199912060000);
    next_date = Datetime(199912080000);
    CHECK_EQ(tm->borrowStock(cur_date, stock, 25.8, 600), true);
    CHECK_EQ(tm->borrowStock(cur_date, stock, 25.83, 400), true);
    funds = tm->getFunds(pre_date);
    CHECK_EQ(funds, FundsRecord(99330, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(cur_date);
    CHECK_EQ(funds, FundsRecord(99230, 0, 0, 100000, 0, 0, 25812));
    funds = tm->getFunds(next_date);
    CHECK_EQ(funds, FundsRecord(99230, 0, 0, 100000, 0, 0, 25812));

    cur_date = Datetime(199912080000);
    pre_date = Datetime(199912070000);
    next_date = Datetime(199912090000);
    CHECK_EQ(tm->returnStock(cur_date, stock, 25.50, 200), true);
    funds = tm->getFunds(pre_date);
    CHECK_EQ(funds, FundsRecord(99230, 0, 0, 100000, 0, 0, 25812));
    funds = tm->getFunds(cur_date);
    CHECK_EQ(funds, FundsRecord(99170, 0, 0, 100000, 0, 0, 20652));
    funds = tm->getFunds(next_date);
    CHECK_EQ(funds, FundsRecord(99170, 0, 0, 100000, 0, 0, 20652));

    CHECK_EQ(tm->returnStock(cur_date, stock, 25.50, 400), true);
    funds = tm->getFunds(pre_date);
    CHECK_EQ(funds, FundsRecord(99230, 0, 0, 100000, 0, 0, 25812));
    funds = tm->getFunds(cur_date);
    CHECK_EQ(funds, FundsRecord(99110, 0, 0, 100000, 0, 0, 10332));
    funds = tm->getFunds(next_date);
    CHECK_EQ(funds, FundsRecord(99110, 0, 0, 100000, 0, 0, 10332));

    CHECK_EQ(tm->returnStock(cur_date, stock, 25.50, 400), true);
    funds = tm->getFunds(pre_date);
    CHECK_EQ(funds, FundsRecord(99230, 0, 0, 100000, 0, 0, 25812));
    funds = tm->getFunds(cur_date);
    CHECK_EQ(funds, FundsRecord(99050, 0, 0, 100000, 0, 0, 0));
    funds = tm->getFunds(next_date);
    CHECK_EQ(funds, FundsRecord(99050, 0, 0, 100000, 0, 0, 0));
}

/** @par Test point: test getTradeList */
TEST_CASE("test_getTradeList") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm.getStock("sz000001");

    CostRecord cost;

    TradeManagerPtr tm = crtTM(Datetime(199305010000), 100000);

    tm->buy(Datetime(199305200000L), stk, 55.7, 100);
    tm->buy(Datetime(199305250000L), stk, 27.5, 100);
    tm->buy(Datetime(199407110000L), stk, 8.55, 200);

    /** @arg Get all the trade records */
    TradeRecordList tr_list = tm->getTradeList();
    CHECK_EQ(tr_list.size(), 8);
    CHECK_EQ(tr_list[0], TradeRecord(Stock(), Datetime(199305010000L), BUSINESS_INIT, 100000,
                                     100000, 0, 0, cost, 0, 100000, PART_INVALID));
    CHECK_EQ(tr_list[7], TradeRecord(stk, Datetime(199407110000L), BUSINESS_BUY, 0, 8.55, 0, 200,
                                     cost, 0, 90142.50, PART_INVALID));

    /** @arg Get the records in a range where start is the account creation date and end is
     * Null<Datetime>() */
    tr_list = tm->getTradeList(Datetime(199305010000), Null<Datetime>());

    CHECK_EQ(tr_list.size(), 8);
    CHECK_EQ(tr_list[0], TradeRecord(Stock(), Datetime(199305010000L), BUSINESS_INIT, 100000,
                                     100000, 0, 0, cost, 0, 100000, PART_INVALID));
    CHECK_EQ(tr_list[7], TradeRecord(stk, Datetime(199407110000L), BUSINESS_BUY, 0, 8.55, 0, 200,
                                     cost, 0, 90142.50, PART_INVALID));

    /** @arg Get the records in a range where start is the first buy record date and end is Null */
    tr_list = tm->getTradeList(Datetime(199305200000L), Null<Datetime>());

    CHECK_EQ(tr_list.size(), 7);
    CHECK_EQ(tr_list[0], TradeRecord(stk, Datetime(199305200000L), BUSINESS_BUY, 0, 55.70, 0, 100,
                                     cost, 0, 94430, PART_INVALID));
    CHECK_EQ(tr_list[6], TradeRecord(stk, Datetime(199407110000L), BUSINESS_BUY, 0, 8.55, 0, 200,
                                     cost, 0, 90142.50, PART_INVALID));

    /** @arg Get the records in a range where start lies between two record dates, end is Null */
    tr_list = tm->getTradeList(Datetime(199305210000L), Null<Datetime>());

    CHECK_EQ(tr_list.size(), 6);
    // cash_base 94430 + a single dividend 30 = 94460 (the old code double counted it as 94490)
    CHECK_EQ(tr_list[0], TradeRecord(stk, Datetime(199305240000L), BUSINESS_BONUS, 30, 30, 0, 0,
                                     cost, 0, 94460, PART_INVALID));
    CHECK_EQ(tr_list[5], TradeRecord(stk, Datetime(199407110000L), BUSINESS_BUY, 0, 8.55, 0, 200,
                                     cost, 0, 90142.50, PART_INVALID));

    /** @arg Get the records in a range where start is greater than end */
    tr_list = tm->getTradeList(Null<Datetime>(), Datetime(199305210000L));
    CHECK_EQ(tr_list.size(), 0);

    /** @arg Get the records in a range where start equals end */
    tr_list = tm->getTradeList(Datetime(199305210000L), Datetime(199305210000L));
    CHECK_EQ(tr_list.size(), 0);

    /** @arg Get the records where start is a record date and end is later than the last record */
    tr_list = tm->getTradeList(Datetime(199305200000L), Datetime(199407120000L));

    CHECK_EQ(tr_list.size(), 7);
    CHECK_EQ(tr_list[0], TradeRecord(stk, Datetime(199305200000L), BUSINESS_BUY, 0, 55.70, 0, 100,
                                     cost, 0, 94430, PART_INVALID));
    CHECK_EQ(tr_list[6], TradeRecord(stk, Datetime(199407110000L), BUSINESS_BUY, 0, 8.55, 0, 200,
                                     cost, 0, 90142.50, PART_INVALID));

    /** @arg Get the records where start is a record date and end is the last record date */
    tr_list = tm->getTradeList(Datetime(199305200000L), Datetime(199407110000L));

    CHECK_EQ(tr_list.size(), 4);
    CHECK_EQ(tr_list[0], TradeRecord(stk, Datetime(199305200000L), BUSINESS_BUY, 0, 55.70, 0, 100,
                                     cost, 0, 94430, PART_INVALID));
    CHECK_EQ(tr_list[3], TradeRecord(stk, Datetime(199305250000L), BUSINESS_BUY, 0, 27.5, 0, 100,
                                     cost, 0, 91710, PART_INVALID));
}

/** @par Test point: updateWithWeight must not double count a dividend in a record's cash snapshot
 *  (issue #511). sz000001 on 1993-05-24 pays a 3.0 dividend plus gift/increasement shares. After
 *  buying 100 shares on 05-20 the cash is 94430; the 05-25 buy triggers the update (bonus = 30), so
 *  the BONUS and the same-date GIFT records must both carry 94460, not the inflated 94490.*/
TEST_CASE("test_TradeManager_updateWithWeight_bonus_cash_no_double_count") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm.getStock("sz000001");

    TradeManagerPtr tm = crtTM(Datetime(199305010000), 100000);
    tm->buy(Datetime(199305200000L), stk, 55.7, 100);  // cash -> 94430
    tm->buy(Datetime(199305250000L), stk, 27.5, 100);  // trigger the 1993-05-24 weight update

    TradeRecordList tr_list = tm->getTradeList();
    const TradeRecord* bonus = nullptr;
    const TradeRecord* gift = nullptr;
    for (const auto& t : tr_list) {
        if (t.datetime == Datetime(199305240000L)) {
            if (t.business == BUSINESS_BONUS)
                bonus = &t;
            if (t.business == BUSINESS_GIFT)
                gift = &t;
        }
    }

    /** @arg BONUS cash = batch-start cash + a single dividend, not double counted */
    CHECK(bonus != nullptr);
    if (bonus) {
        CHECK_EQ(bonus->realPrice, 30);
        CHECK_EQ(bonus->cash, 94460);
    }

    /** @arg Same-date GIFT shares the correct post-dividend cash snapshot */
    CHECK(gift != nullptr);
    if (gift) {
        CHECK_EQ(gift->cash, 94460);
    }

    /** @arg Historical cash replay agrees with the record snapshot */
    CHECK_EQ(tm->cash(Datetime(199305240000L)), 94460);
}

/** @par Test point: test addTradeRecord */
TEST_CASE("test_TradeManager_addTradeRecord") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm.getStock("sz000001");

    CostRecord cost;

    TradeManagerPtr tm = crtTM(Datetime(199305010000), 100000);

    tm->buy(Datetime(199305200000L), stk, 55.7, 100);
    tm->buy(Datetime(199305250000L), stk, 27.5, 100);
    tm->buy(Datetime(199407110000L), stk, 8.55, 200);

    /** @arg Add the account initialization trade record */
    TradeRecordList tr_list = tm->getTradeList();
    CHECK_EQ(tr_list.size(), 8);
    CHECK_EQ(tr_list[0], TradeRecord(Stock(), Datetime(199305010000L), BUSINESS_INIT, 100000,
                                     100000, 0, 0, cost, 0, 100000, PART_INVALID));

    TradeRecord tr(Stock(), Datetime(199201010000L), BUSINESS_INIT, 200000, 200000, 0, 0, cost, 0,
                   200000, PART_INVALID);
    tm->addTradeRecord(tr);

    tr_list = tm->getTradeList();
    CHECK_EQ(tr_list.size(), 1);
    CHECK_EQ(tr_list[0], tr);

    /** @arg Copy the trade records of one tm into another */
    tm = crtTM(Datetime(199305010000), 100000);
    tm->buy(Datetime(199305200000L), stk, 55.7, 100);
    tm->buy(Datetime(199305250000L), stk, 27.5, 100);
    tm->buy(Datetime(199407110000L), stk, 8.55, 200);
    tr_list = tm->getTradeList();

    TMPtr tm2 = crtTM(Datetime(199101010000), 100000, TC_Zero(), "TM2");
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
                                     100000, 0, 0, cost, 0, 100000, PART_INVALID));
    CHECK_EQ(tr_list[7], TradeRecord(stk, Datetime(199407110000L), BUSINESS_BUY, 0, 8.55, 0, 200,
                                     cost, 0, 90142.50, PART_INVALID));
}

/** @} */
