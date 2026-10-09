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
#include <hikyuu/trade_manage/Performance.h>

#include <fstream>
#include <cmath>
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

// A test cost function with a controllable margin financing cost, used to exercise the
// auto-financing caliber of buy
class TestBorrowCost final : public TradeCostBase {
public:
    TestBorrowCost(price_t rate, price_t fixed)
    : TradeCostBase("TestBorrowCost"), m_rate(rate), m_fixed(fixed) {}

    CostRecord getBuyCost(const Datetime&, const Stock&, price_t, double) const override {
        return CostRecord();
    }

    CostRecord getSellCost(const Datetime&, const Stock&, price_t, double) const override {
        return CostRecord();
    }

    CostRecord getBorrowCashCost(const Datetime&, price_t cash) const override {
        CostRecord ret;
        price_t fee = roundEx(m_rate * cash + m_fixed, 2);
        ret.commission = fee;
        ret.total = fee;
        return ret;
    }

protected:
    TradeCostPtr _clone() override {
        return std::make_shared<TestBorrowCost>(m_rate, m_fixed);
    }

private:
    price_t m_rate{0.0};
    price_t m_fixed{0.0};
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

/** @par Test points */
TEST_CASE("test_TradeManager_checkoutStock_base_asset") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000);

    /** @arg After depositing the stock the base asset is the deposited value */
    CHECK_EQ(tm->checkinStock(Datetime(199901020000), stock, 10.0, 100), true);
    FundsRecord funds = tm->getFunds(Datetime(199901020000));
    CHECK_EQ(funds.base_asset, 1000.0);

    /** @arg After withdrawing the stock the base asset must be zero */
    CHECK_EQ(tm->checkoutStock(Datetime(199901030000), stock, 10.0, 100), true);
    funds = tm->getFunds(Datetime(199901030000));
    CHECK_EQ(funds.base_asset, 0.0);
}

/** @par Test points */
TEST_CASE("test_TradeManager_getFunds_fractional_stock") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000);

    /** @arg The fractional shares produced by the dividend replay must not be truncated in
     * getFunds */
    CHECK_EQ(tm->checkinStock(Datetime(200001040000), stock, 10.0, 1000), true);
    CHECK_EQ(tm->checkinStock(Datetime(200001050000), stock, 10.0, 151.5), true);

    price_t price = stock.getMarketValue(Datetime(200001060000), KQuery::DAY);
    REQUIRE_UNARY(price > 0.0);
    FundsRecord funds = tm->getFunds(Datetime(200001060000));
    CHECK_EQ(funds.market_value, doctest::Approx(price * 1151.5));
}

namespace {

class ReturnStockCostFunc : public TradeCostBase {
public:
    ReturnStockCostFunc() : TradeCostBase("ReturnStockCostFunc") {}

    CostRecord getBuyCost(const Datetime& datetime, const Stock& stock, price_t price,
                          double num) const override {
        return CostRecord();
    }

    CostRecord getSellCost(const Datetime& datetime, const Stock& stock, price_t price,
                           double num) const override {
        return CostRecord();
    }

    CostRecord getReturnStockCost(const Datetime& borrow_datetime, const Datetime& return_datetime,
                                  const Stock& stock, price_t price, double num) const override {
        return CostRecord(num * 0.01, 0.0, 0.0, 0.0, num * 0.01);
    }

    TradeCostPtr _clone() override {
        return TradeCostPtr(new ReturnStockCostFunc);
    }
};

}  // namespace

/** @par Test points */
TEST_CASE("test_TradeManager_returnStock_cost") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeManagerPtr tm =
      crtTM(Datetime(199901010000), 100000, TradeCostPtr(new ReturnStockCostFunc));

    /** @arg Borrow in two lots and return in one, the cost is charged by the actual number of
     * each segment (previously each segment was charged repeatedly by the full number) */
    CHECK_EQ(tm->borrowStock(Datetime(199901020000), stock, 10.0, 50), true);
    CHECK_EQ(tm->borrowStock(Datetime(199901030000), stock, 10.0, 50), true);
    CHECK_EQ(tm->returnStock(Datetime(199901040000), stock, 10.0, 100), true);

    const TradeRecord tr = tm->getTradeList().back();
    CHECK_EQ(tr.business, BUSINESS_RETURN_STOCK);
    CHECK_EQ(tr.cost.total, doctest::Approx(1.0));
}

/** @par Test points */
TEST_CASE("test_TradeManager_buy_margin") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 6000, TC_Zero());
    tm->setParam<bool>("support_borrow_cash", true);

    /** @arg Enough own cash: no borrowing at all */
    TradeRecord tr = tm->buy(Datetime(199901020000), stock, 10.0, 100);
    CHECK_EQ(tr.business, BUSINESS_BUY);
    CHECK_EQ(tm->getDebtCash(Datetime(199901020000)), 0.0);

    /** @arg Insufficient cash: borrow only the shortfall */
    tr = tm->buy(Datetime(199901030000), stock, 10.0, 600);
    CHECK_EQ(tr.business, BUSINESS_BUY);
    CHECK_EQ(tr.number, 600);
    CHECK_EQ(tm->getDebtCash(Datetime(199901030000)), 1000);

    /** @arg Insufficient buying power: fail cleanly without any residue */
    tr = tm->buy(Datetime(199901040000), stock, 10.0, 5000);
    CHECK_EQ(tr.business, BUSINESS_INVALID);
    CHECK_EQ(tm->getDebtCash(Datetime(199901040000)), 1000);
    CHECK_EQ(tm->currentCash(), 0.0);
}

/** @par Test points */
TEST_CASE("test_TradeManager_buyShort_cash_check") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000, TC_Zero());

    CHECK_EQ(tm->borrowStock(Datetime(199901020000), stock, 10.0, 100), true);
    CHECK_EQ(tm->sellShort(Datetime(199901030000), stock, 10.0, 100).business, BUSINESS_SELL_SHORT);

    /** @arg The cash is insufficient to buy back: reject without overdrawing */
    CHECK_EQ(tm->checkout(Datetime(199901040000), 101000), true);
    CHECK_EQ(tm->currentCash(), 0.0);
    TradeRecord tr = tm->buyShort(Datetime(199901050000), stock, 10.0, 100);
    CHECK_EQ(tr.business, BUSINESS_INVALID);
    CHECK_EQ(tm->currentCash(), 0.0);

    /** @arg Enough cash after checkin: buy back normally */
    tm->checkin(Datetime(199901060000), 1000);
    tr = tm->buyShort(Datetime(199901070000), stock, 10.0, 100);
    CHECK_EQ(tr.business, BUSINESS_BUY_SHORT);
    CHECK_EQ(tm->getShortPosition(stock).number, 0.0);
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

/** @par Test point: getRefTradeList returns the same content as getTradeList without copying */
TEST_CASE("test_getRefTradeList") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm.getStock("sz000001");

    TradeManagerPtr tm = crtTM(Datetime(199305010000), 100000);

    /** @arg The reference content equals the copied list */
    tm->buy(Datetime(199305200000L), stk, 55.7, 100);
    tm->buy(Datetime(199407110000L), stk, 8.55, 200);

    const TradeRecordList& ref_list = tm->getRefTradeList();
    TradeRecordList copy_list = tm->getTradeList();
    CHECK_EQ(ref_list.size(), copy_list.size());
    for (size_t i = 0; i < copy_list.size(); ++i) {
        CHECK_EQ(ref_list[i], copy_list[i]);
    }

    /** @arg The reference stays valid and observes later trades (no stale snapshot) */
    size_t prev_size = ref_list.size();
    tm->buy(Datetime(199408010000L), stk, 9.0, 100);
    CHECK_GT(ref_list.size(), prev_size);
    CHECK_EQ(ref_list.back(), tm->getTradeList().back());
}

/** @par Test point: getFundsList must match the per-date getFunds exactly (the merge replay uses
 *  the same accumulators and the same order), including unsorted input, duplicates, the date
 *  equal to the last trade date, and dates after the last trade date */
TEST_CASE("test_getFundsList_equivalence") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm.getStock("sz000001");

    TradeManagerPtr tm = crtTM(Datetime(199012010000), 100000);
    tm->buy(Datetime(199101020000L), stk, 60.0, 100);
    tm->sell(Datetime(199206150000L), stk, 75.0, 50);
    tm->checkin(Datetime(199301050000), 20000);
    tm->buy(Datetime(199407110000L), stk, 8.55, 200);

    /** @arg Unsorted input mixing every branch: before the first trade, between trades, equal to
     *  the last trade date, after the last trade date, and a duplicate */
    DatetimeList dates = {Datetime(199301050000), Datetime(199012150000), Datetime(199407110000),
                          Datetime(199101050000), Datetime(199501010000), Datetime(199206200000),
                          Datetime(199101050000)};

    FundsList funds_list = tm->getFundsList(dates);
    CHECK_EQ(funds_list.size(), dates.size());
    for (size_t i = 0; i < dates.size(); ++i) {
        FundsRecord expect = tm->getFunds(dates[i]);
        const FundsRecord& got = funds_list[i];
        CHECK_EQ(got.cash, expect.cash);
        CHECK_EQ(got.market_value, expect.market_value);
        CHECK_EQ(got.short_market_value, expect.short_market_value);
        CHECK_EQ(got.base_cash, expect.base_cash);
        CHECK_EQ(got.base_asset, expect.base_asset);
        CHECK_EQ(got.borrow_cash, expect.borrow_cash);
        CHECK_EQ(got.borrow_asset, expect.borrow_asset);
    }

    /** @arg The input order is preserved */
    CHECK_EQ(funds_list[0], tm->getFunds(Datetime(199301050000)));
    CHECK_EQ(funds_list[1], tm->getFunds(Datetime(199012150000)));
    CHECK_EQ(funds_list[2], tm->getFunds(Datetime(199407110000)));
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

/** @par Test point: rebuild an account from a full trade list covering every reconstructable
 * business type */
TEST_CASE("test_TradeManager_addTradeRecord_roundtrip") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");

    // Use a window without any ex-rights event so no derived record is generated and the source
    // and the rebuilt lists match one-to-one
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 1000000, TC_Zero(), "SRC");
    tm->setParam<bool>("support_borrow_cash", true);
    tm->setParam<bool>("support_borrow_stock", true);

    CHECK_UNARY(tm->checkinStock(Datetime(199902010000), stock, 10.0, 500));
    CHECK_UNARY(tm->borrowCash(Datetime(199902020000), 5000));
    CHECK_EQ(tm->buy(Datetime(199902030000), stock, 10.0, 300).business, BUSINESS_BUY);
    CHECK_UNARY(tm->borrowStock(Datetime(199902040000), stock, 10.0, 400));
    CHECK_EQ(tm->sellShort(Datetime(199902040000), stock, 10.0, 300).business, BUSINESS_SELL_SHORT);
    CHECK_UNARY(tm->returnStock(Datetime(199902050000), stock, 10.0, 100));
    CHECK_EQ(tm->sell(Datetime(199902060000), stock, 12.0, 200).business, BUSINESS_SELL);
    CHECK_EQ(tm->buyShort(Datetime(199902070000), stock, 11.0, 200).business, BUSINESS_BUY_SHORT);
    CHECK_UNARY(tm->checkoutStock(Datetime(199902090000), stock, 11.0, 100));
    CHECK_UNARY(tm->checkout(Datetime(199902100000), 1000));

    TradeRecordList src = tm->getTradeList();
    TradeManagerPtr tm2 = crtTM(Datetime(199001010000), 0, TC_Zero(), "DST");
    for (const auto& tr : src) {
        /** @arg every reconstructable record is accepted */
        CHECK_UNARY(tm2->addTradeRecord(tr));
    }

    /** @arg the rebuilt account matches the source on every queried dimension */
    CHECK_EQ(tm2->getTradeList().size(), src.size());
    CHECK_EQ(tm2->currentCash(), tm->currentCash());
    CHECK_EQ(tm2->lastDatetime(), tm->lastDatetime());
    CHECK_EQ(tm2->getDebtCash(Datetime(199902100000)), tm->getDebtCash(Datetime(199902100000)));
    CHECK_EQ(tm2->getHoldNumber(Datetime(199902100000), stock),
             tm->getHoldNumber(Datetime(199902100000), stock));
    CHECK_EQ(tm2->getShortHoldNumber(Datetime(199902100000), stock),
             tm->getShortHoldNumber(Datetime(199902100000), stock));
    CHECK_EQ(tm2->getDebtNumber(Datetime(199902100000), stock),
             tm->getDebtNumber(Datetime(199902100000), stock));
}

/** @par Test point: an over-repayment RETURN_CASH record is rejected without draining the loan list
 */
TEST_CASE("test_TradeManager_addTradeRecord_return_cash_beyond_debt") {
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000, TC_Zero(), "SRC");
    CHECK_UNARY(tm->borrowCash(Datetime(199902010000), 3000));

    TradeManagerPtr tm2 = crtTM(Datetime(199901010000), 100000, TC_Zero(), "DST");
    for (const auto& tr : tm->getTradeList()) {
        CHECK_UNARY(tm2->addTradeRecord(tr));
    }
    CHECK_EQ(tm2->getDebtCash(Datetime(199902010000)), 3000.0);

    CostRecord cost;
    TradeRecord over(Null<Stock>(), Datetime(199902020000), BUSINESS_RETURN_CASH, 5000, 5000, 0.0,
                     0, cost, 0.0, 0, PART_INVALID);
    /** @arg rejected, and the loan list / debt stay untouched (atomic) */
    CHECK_UNARY(!tm2->addTradeRecord(over));
    CHECK_EQ(tm2->getDebtCash(Datetime(199902020000)), 3000.0);
}

/** @par Test points */
TEST_CASE("test_TradeManager_returnCash_multi_loan") {
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000);

    /** @arg Repay multiple loans sequentially with an exact amount */
    tm->borrowCash(Datetime(199901020000), 100.10);
    tm->borrowCash(Datetime(199901030000), 0.20);
    CHECK_EQ(tm->returnCash(Datetime(199901040000), 100.30), true);
    CHECK_EQ(tm->getDebtCash(Datetime(199901050000)), 0.0);

    /** @arg Returning more than the debt but within the grid unit is treated as fully repaid */
    tm->borrowCash(Datetime(199901060000), 100.00);
    CHECK_EQ(tm->returnCash(Datetime(199901070000), 100.004), true);
    CHECK_EQ(tm->getDebtCash(Datetime(199901080000)), 0.0);

    /** @arg Returning more than the debt beyond the grid unit is rejected */
    tm->borrowCash(Datetime(199901090000), 100.00);
    CHECK_EQ(tm->returnCash(Datetime(199901100000), 100.01), false);
    CHECK_EQ(tm->getDebtCash(Datetime(199901110000)), 100.0);
}

/** @par Test points */
TEST_CASE("test_TradeManager_addPosition") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    Stock stock2 = sm.getStock("sh600004");
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000);

    /** @arg A null stock is rejected */
    PositionRecord pr(Null<Stock>(), Datetime(199911170000), Null<Datetime>(), 100, 0, 0, 100, 1000,
                      0, 0, 0);
    CHECK_EQ(tm->addPosition(pr), false);

    /** @arg A closed position record (cleanDatetime not null) is rejected */
    pr.stock = stock;
    pr.cleanDatetime = Datetime(199911180000);
    CHECK_EQ(tm->addPosition(pr), false);

    /** @arg A take datetime earlier than the account creation date is rejected */
    pr = PositionRecord(stock, Datetime(199801010000), Null<Datetime>(), 100, 0, 0, 100, 1000, 0, 0,
                        0);
    CHECK_EQ(tm->addPosition(pr), false);

    /** @arg A position can be added while the trade list holds only the INIT record */
    pr = PositionRecord(stock, Datetime(199911170000), Null<Datetime>(), 100, 0, 0, 100, 1000, 0, 0,
                        0);
    CHECK_EQ(tm->addPosition(pr), true);
    CHECK_EQ(tm->have(stock), true);
    CHECK_EQ(tm->getHoldNumber(Datetime(199911170000), stock), 100);

    /** @arg The same stock cannot be added twice */
    CHECK_EQ(tm->addPosition(PositionRecord(stock, Datetime(199911180000), Null<Datetime>(), 100, 0,
                                            0, 100, 1000, 0, 0, 0)),
             false);

    /** @arg A later take datetime moves the init datetime and syncs the INIT record datetime */
    tm = crtTM(Datetime(199901010000), 100000);
    CHECK_EQ(tm->addPosition(PositionRecord(stock, Datetime(199911170000), Null<Datetime>(), 100, 0,
                                            0, 100, 1000, 0, 0, 0)),
             true);
    CHECK_EQ(tm->initDatetime(), Datetime(199911170000));
    CHECK_EQ(tm->getTradeList()[0].datetime, Datetime(199911170000));

    /** @arg Several stocks can be added before any trade record is made */
    tm = crtTM(Datetime(199901010000), 100000);
    CHECK_EQ(tm->addPosition(PositionRecord(stock, Datetime(199911170000), Null<Datetime>(), 100, 0,
                                            0, 100, 1000, 0, 0, 0)),
             true);
    CHECK_EQ(tm->addPosition(PositionRecord(stock2, Datetime(199911170000), Null<Datetime>(), 200,
                                            0, 0, 200, 2000, 0, 0, 0)),
             true);
    CHECK_EQ(tm->getStockNumber(), 2);

    /** @arg Once the trade list holds more than the INIT record, adding a position is rejected */
    tm = crtTM(Datetime(199901010000), 100000);
    tm->buy(Datetime(199911170000), stock2, 10.0, 100);
    CHECK_EQ(tm->addPosition(PositionRecord(stock, Datetime(199911180000), Null<Datetime>(), 100, 0,
                                            0, 100, 1000, 0, 0, 0)),
             false);
}

/** @par Test points */
TEST_CASE("test_TradeManager_param_guards") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    double nan = std::numeric_limits<double>::quiet_NaN();
    double inf = std::numeric_limits<double>::infinity();
    TradeManagerPtr tm;
    TradeRecord result;
    auto invalid = [](const TradeRecord& r) { return r.business == BUSINESS_INVALID; };

    /** @arg buy rejects negative, NaN and infinite real price */
    tm = crtTM(Datetime(199901010000), 1000000, TC_FixedA(0.0018, 5, 0.001, 0.001, 1.0), "TEST");
    CHECK_UNARY(invalid(tm->buy(Datetime(199911170000), stock, -10.0, 100)));
    CHECK_UNARY(invalid(tm->buy(Datetime(199911170000), stock, nan, 100)));
    CHECK_UNARY(invalid(tm->buy(Datetime(199911170000), stock, inf, 100)));

    /** @arg buy rejects a NaN number, which used to pass all the number comparisons */
    CHECK_UNARY(invalid(tm->buy(Datetime(199911170000), stock, 10.0, nan)));
    CHECK_UNARY(invalid(tm->buy(Datetime(199911170000), stock, 10.0, inf)));

    /** @arg the cash stays untouched by the rejected orders and a normal buy still succeeds */
    CHECK_EQ(tm->cash(Datetime(199911170000)), 1000000.0);
    result = tm->buy(Datetime(199911170000), stock, 10.0, 100);
    CHECK_EQ(result.business, BUSINESS_BUY);

    /** @arg a zero real price is the legal market-order placeholder of the Strategy path */
    result = tm->buy(Datetime(199911170000), stock, 0.0, 100);
    CHECK_EQ(result.business, BUSINESS_BUY);

    /** @arg sell rejects negative, NaN and infinite real price */
    CHECK_UNARY(invalid(tm->sell(Datetime(199911180000), stock, -10.0, 100)));
    CHECK_UNARY(invalid(tm->sell(Datetime(199911180000), stock, nan, 100)));
    CHECK_UNARY(invalid(tm->sell(Datetime(199911180000), stock, inf, 100)));

    /** @arg a normal sell still succeeds */
    result = tm->sell(Datetime(199911180000), stock, 10.0, MAX_DOUBLE);
    CHECK_EQ(result.business, BUSINESS_SELL);

    /** @arg sellShort rejects negative, NaN and infinite real price and a NaN number */
    tm = crtTM(Datetime(199901010000), 1000000, TC_FixedA(0.0018, 5, 0.001, 0.001, 1.0), "TEST");
    CHECK_UNARY(invalid(tm->sellShort(Datetime(199911170000), stock, -10.0, 100)));
    CHECK_UNARY(invalid(tm->sellShort(Datetime(199911170000), stock, nan, 100)));
    CHECK_UNARY(invalid(tm->sellShort(Datetime(199911170000), stock, inf, 100)));
    CHECK_UNARY(invalid(tm->sellShort(Datetime(199911170000), stock, 10.0, nan)));

    /** @arg a normal sellShort still succeeds */
    CHECK_EQ(tm->borrowStock(Datetime(199911170000), stock, 10.0, 100), true);
    result = tm->sellShort(Datetime(199911170000), stock, 10.0, 100);
    CHECK_EQ(result.business, BUSINESS_SELL_SHORT);

    /** @arg buyShort rejects negative, NaN and infinite real price and a NaN number */
    CHECK_UNARY(invalid(tm->buyShort(Datetime(199911180000), stock, -10.0, 100)));
    CHECK_UNARY(invalid(tm->buyShort(Datetime(199911180000), stock, nan, 100)));
    CHECK_UNARY(invalid(tm->buyShort(Datetime(199911180000), stock, inf, 100)));
    CHECK_UNARY(invalid(tm->buyShort(Datetime(199911180000), stock, 10.0, nan)));

    /** @arg buyShort with MAX_DOUBLE number to close the short position still succeeds */
    result = tm->buyShort(Datetime(199911180000), stock, 10.0, MAX_DOUBLE);
    CHECK_EQ(result.business, BUSINESS_BUY_SHORT);
}

/** @par Test points */
TEST_CASE("test_TradeManager_stock_ops_guards") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    double nan = std::numeric_limits<double>::quiet_NaN();
    double inf = std::numeric_limits<double>::infinity();
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 1000000, TC_Zero(), "TEST");

    /** @arg checkinStock rejects zero, negative, NaN and infinite numbers */
    CHECK_UNARY(!tm->checkinStock(Datetime(199911170000), stock, 10.0, 0));
    CHECK_UNARY(!tm->checkinStock(Datetime(199911170000), stock, 10.0, -100));
    CHECK_UNARY(!tm->checkinStock(Datetime(199911170000), stock, 10.0, nan));
    CHECK_UNARY(!tm->checkinStock(Datetime(199911170000), stock, 10.0, inf));

    /** @arg the rejected checkins leave no position behind */
    CHECK_EQ(tm->getHoldNumber(Datetime(199911170000), stock), 0);

    /** @arg a normal checkin succeeds */
    CHECK_UNARY(tm->checkinStock(Datetime(199911170000), stock, 10.0, 100));
    CHECK_EQ(tm->getHoldNumber(Datetime(199911170000), stock), 100);

    /** @arg checkoutStock rejects negative, NaN and infinite numbers that used to inflate the
     * position through "number > pos.number" being false */
    CHECK_UNARY(!tm->checkoutStock(Datetime(199911180000), stock, 10.0, 0));
    CHECK_UNARY(!tm->checkoutStock(Datetime(199911180000), stock, 10.0, -50));
    CHECK_UNARY(!tm->checkoutStock(Datetime(199911180000), stock, 10.0, nan));
    CHECK_UNARY(!tm->checkoutStock(Datetime(199911180000), stock, 10.0, inf));
    CHECK_EQ(tm->getHoldNumber(Datetime(199911180000), stock), 100);

    /** @arg borrowStock rejects zero, negative, NaN and infinite numbers */
    CHECK_UNARY(!tm->borrowStock(Datetime(199911180000), stock, 10.0, 0));
    CHECK_UNARY(!tm->borrowStock(Datetime(199911180000), stock, 10.0, -100));
    CHECK_UNARY(!tm->borrowStock(Datetime(199911180000), stock, 10.0, nan));
    CHECK_UNARY(!tm->borrowStock(Datetime(199911180000), stock, 10.0, inf));
    CHECK_UNARY(tm->getBorrowStockList().empty());

    /** @arg a normal borrow succeeds as the baseline for the return guards */
    CHECK_UNARY(tm->borrowStock(Datetime(199911180000), stock, 10.0, 100));

    /** @arg returnStock rejects negative, NaN and infinite numbers that used to bypass the
     * "number > bor.number" guard and increase the debt instead of repaying */
    CHECK_UNARY(!tm->returnStock(Datetime(199911180000), stock, 10.0, 0));
    CHECK_UNARY(!tm->returnStock(Datetime(199911180000), stock, 10.0, -100));
    CHECK_UNARY(!tm->returnStock(Datetime(199911180000), stock, 10.0, nan));
    CHECK_UNARY(!tm->returnStock(Datetime(199911180000), stock, 10.0, inf));
    CHECK_EQ(tm->getBorrowStockList()[0].number, 100);

    /** @arg a normal return succeeds and clears the debt */
    CHECK_UNARY(tm->returnStock(Datetime(199911180000), stock, 10.0, 100));
    CHECK_UNARY(tm->getBorrowStockList().empty());
}

/** @par Test points */
TEST_CASE("test_TradeManager_short_borrow_exrights") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");

    // sh600000 does 10 turn 3 (bonus shares) on 2006-05-12: the owed quantity must follow
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 1000000, TC_Zero());
    CHECK_UNARY(tm->borrowStock(Datetime(200605100000), stock, 10.0, 200));
    CHECK_EQ(tm->sellShort(Datetime(200605100000), stock, 10.0, 200).business, BUSINESS_SELL_SHORT);

    /** @arg the borrowed (owed) quantity grows by 30% after the 10 turn 3 ex-rights */
    CHECK_EQ(tm->getDebtNumber(Datetime(200605120000), stock), 260.0);

    /** @arg the short position grows by the same ratio so that buyShort can cover it */
    CHECK_EQ(tm->getShortHoldNumber(Datetime(200605120000), stock), 260.0);

    // sh600000 pays a cash dividend of 1.5 per 10 shares on 2000-07-06: compensated to the lender
    tm = crtTM(Datetime(199901010000), 1000000, TC_Zero());
    CHECK_UNARY(tm->borrowStock(Datetime(200007050000), stock, 10.0, 200));
    CHECK_EQ(tm->sellShort(Datetime(200007050000), stock, 10.0, 200).business, BUSINESS_SELL_SHORT);
    price_t cash_before = tm->currentCash();

    /** @arg a cash dividend does not change the owed quantity */
    CHECK_EQ(tm->getDebtNumber(Datetime(200007060000), stock), 200.0);

    /** @arg the cash dividend is compensated to the lender as an account cash outflow */
    CHECK_EQ(tm->currentCash(), roundEx(cash_before - 200.0 * 1.5 * 0.1, 2));

    /** @arg the compensation is recorded as a dedicated DIVIDEND_COMPENSATION record (a
     * financing cost) so that the dividend income metric is not polluted by a negative amount */
    TradeRecordList tr_list = tm->getTradeList(Datetime(200007060000), Datetime(200007070000));
    bool found_comp = false;
    for (const auto& record : tr_list) {
        CHECK_UNARY(record.business != BUSINESS_BONUS);
        if (record.business == BUSINESS_DIVIDEND_COMPENSATION) {
            found_comp = true;
            CHECK_EQ(record.realPrice, 30.0);
        }
    }
    CHECK_UNARY(found_comp);

    /** @arg Total Dividends stays gross: the compensation is a cost, not a negative dividend */
    Performance perf;
    perf.statistics(tm, Datetime(200007060000));
    CHECK_EQ(perf.get("Total Dividends"), 0.0);
}

/** @par Test points */
TEST_CASE("test_TradeManager_margin_and_borrow_atomicity") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");

    // insufficient margin must reject instead of injecting cash via checkin
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100, TC_Zero());
    tm->setParam<bool>("support_borrow_stock", true);
    TradeRecord result = tm->sellShort(Datetime(199911170000), stock, 10.0, 1000);
    /** @arg the order is rejected (100 / 0.6 < 10000) */
    CHECK_EQ(result.business, BUSINESS_INVALID);

    /** @arg no CHECKIN injection and no borrow debt residue */
    for (const auto& tr : tm->getRefTradeList()) {
        CHECK_UNARY(tr.business != BUSINESS_CHECKIN);
        CHECK_UNARY(tr.business != BUSINESS_BORROW_STOCK);
    }
    CHECK_EQ(tm->getDebtNumber(Datetime(199911170000), stock), 0.0);
    CHECK_EQ(tm->currentCash(), 100.0);

    /** @arg borrow on demand: the unsold pool is used first, only the shortfall creates debt */
    tm = crtTM(Datetime(199901010000), 1000000, TC_Zero());
    tm->setParam<bool>("support_borrow_stock", true);
    CHECK_UNARY(tm->borrowStock(Datetime(199911100000), stock, 10.0, 300));
    result = tm->sellShort(Datetime(199911170000), stock, 10.0, 100);
    CHECK_EQ(result.business, BUSINESS_SELL_SHORT);
    CHECK_EQ(result.number, 100.0);
    CHECK_EQ(tm->getDebtNumber(Datetime(199911170000), stock), 300.0);

    /** @arg the second sell borrows only the remaining shortfall (300 - 200 unsold) */
    result = tm->sellShort(Datetime(199911180000), stock, 10.0, 300);
    CHECK_EQ(result.business, BUSINESS_SELL_SHORT);
    CHECK_EQ(tm->getDebtNumber(Datetime(199911180000), stock), 400.0);
    CHECK_EQ(tm->getShortHoldNumber(Datetime(199911180000), stock), 400.0);

    // the auto financing borrow covers exactly the shortfall (zero-cost case)
    tm = crtTM(Datetime(199901010000), 5000, TC_Zero());
    tm->setParam<bool>("support_borrow_cash", true);
    result = tm->buy(Datetime(199911170000), stock, 10.0, 600);
    /** @arg debt equals the cash shortfall and the buy succeeds */
    CHECK_EQ(result.business, BUSINESS_BUY);
    CHECK_EQ(tm->getDebtCash(Datetime(199911170000)), 1000.0);
    CHECK_EQ(tm->currentCash(), 0.0);

    /** @arg the financing gap includes the cost of the borrow itself (fixed fee: 1000 + 30) */
    tm = crtTM(Datetime(199901010000), 5000, std::make_shared<TestBorrowCost>(0.0, 30.0));
    tm->setParam<bool>("support_borrow_cash", true);
    result = tm->buy(Datetime(199911170000), stock, 10.0, 600);
    CHECK_EQ(result.business, BUSINESS_BUY);
    CHECK_EQ(tm->getDebtCash(Datetime(199911170000)), 1030.0);
    CHECK_EQ(tm->currentCash(), 0.0);

    /** @arg a proportional borrow fee converges to the self-consistent gap (0.9*gap = 1000),
     * i.e. the buy is not wrongly rejected (the old fixed 4-step solver would give up) */
    tm = crtTM(Datetime(199901010000), 5000, std::make_shared<TestBorrowCost>(0.1, 0.0));
    tm->setParam<bool>("support_borrow_cash", true);
    result = tm->buy(Datetime(199911170000), stock, 10.0, 600);
    CHECK_EQ(result.business, BUSINESS_BUY);
    CHECK_EQ(tm->getDebtCash(Datetime(199911170000)), 1111.11);
    CHECK_EQ(tm->currentCash(), 0.0);

    /** @arg a rejected buy leaves no loan record behind (atomic) */
    tm = crtTM(Datetime(199901010000), 100, TC_Zero());
    tm->setParam<bool>("support_borrow_cash", true);
    result = tm->buy(Datetime(199911170000), stock, 10.0, 1000);
    CHECK_EQ(result.business, BUSINESS_INVALID);
    CHECK_EQ(tm->getDebtCash(Datetime(199911170000)), 0.0);
    for (const auto& tr : tm->getRefTradeList()) {
        CHECK_UNARY(tr.business != BUSINESS_BORROW_CASH);
    }
}

/** @par Test points: getFunds-before-init guard, sell NaN-number contract, getPosition
 * open-holding fallback, and short totalRisk on stoploss=0 / partial cover */
TEST_CASE("test_TradeManager_m3_guards") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    double nan = std::numeric_limits<double>::quiet_NaN();

    /** @arg getFunds before the account creation date returns an empty record */
    TradeManagerPtr tm = crtTM(Datetime(199901100000), 100000, TC_Zero());
    FundsRecord before = tm->getFunds(Datetime(199901010000));
    CHECK_EQ(before.cash, 0.0);
    CHECK_EQ(before.base_cash, 0.0);

    /** @arg a NaN number returns INVALID instead of throwing */
    TradeRecord r;
    CHECK_NOTHROW(r = tm->sell(Datetime(199911170000), stock, 10.0, nan));
    CHECK_EQ(r.business, BUSINESS_INVALID);

    /** @arg getPosition on a date strictly before lastDatetime, for a holding still open
     * then, falls back to the live record (not a null-stock default with a positive number) */
    tm->buy(Datetime(199911150000), stock, 10.0, 100);
    tm->buy(Datetime(199911180000), stock, 10.0, 100);  // pushes lastDatetime past the query
    PositionRecord pr = tm->getPosition(Datetime(199911160000), stock);
    CHECK_UNARY(pr.stock == stock);
    CHECK_EQ(pr.number, 100.0);  // replayed quantity at the query date
    CHECK_UNARY(pr.takeDatetime <= Datetime(199911160000));

    /** @arg sellShort with stoploss==0 accumulates no (negative) risk */
    TradeManagerPtr tm0 = crtTM(Datetime(199901010000), 1000000, TC_Zero());
    tm0->setParam<bool>("support_borrow_stock", true);
    tm0->borrowStock(Datetime(199911100000), stock, 10.0, 500);
    tm0->sellShort(Datetime(199911100000), stock, 10.0, 400, 0.0);
    CHECK_EQ(tm0->getShortPosition(stock).totalRisk, 0.0);

    /** @arg buyShort partial cover scales totalRisk by the remaining fraction */
    TradeManagerPtr tm1 = crtTM(Datetime(199901010000), 1000000, TC_Zero());
    tm1->setParam<bool>("support_borrow_stock", true);
    tm1->borrowStock(Datetime(199911100000), stock, 10.0, 400);
    double full_risk = roundEx((12.0 - 10.0) * 400 * stock.unit(), 2);
    tm1->sellShort(Datetime(199911100000), stock, 10.0, 400, 12.0);
    CHECK_EQ(tm1->getShortPosition(stock).totalRisk, full_risk);
    /** @arg the live path books gross proceeds too, with no open fee folded into buyMoney */
    CHECK_EQ(tm1->getShortPosition(stock).sellMoney, roundEx(10.0 * 400 * stock.unit(), 2));
    CHECK_EQ(tm1->getShortPosition(stock).buyMoney, 0.0);
    tm1->buyShort(Datetime(199911110000), stock, 11.0, 100);
    CHECK_EQ(tm1->getShortPosition(stock).totalRisk, roundEx(full_risk * 300.0 / 400.0, 2));
    /** @arg the live buyShort records the gross buy-back amount on buyMoney */
    CHECK_EQ(tm1->getShortPosition(stock).buyMoney, roundEx(11.0 * 100 * stock.unit(), 2));
}

#if HKU_SUPPORT_SERIALIZATION
/** @par Test point: right after load, a historical cash query must not return the current cash */
TEST_CASE("test_TradeManager_cash_after_load") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000, TC_Zero(), "TM109");
    tm->buy(Datetime(199911170000), stock, 10.0, 100);  // spend 1000 -> current cash 99000
    tm->cash(Datetime(199912010000));                   // advance the (unstrored) update cursor

    string file = sm.tmpdir() + "/tm109_cash.xml";
    {
        std::ofstream ofs(file);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(tm);
    }
    TradeManagerPtr tm2;
    {
        std::ifstream ifs(file);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(tm2);
    }

    /** @arg a pre-buy historical query returns the historical cash, not the current 99000 */
    CHECK_EQ(tm2->cash(Datetime(199901150000)), 100000.0);
    CHECK_EQ(tm2->currentCash(), 99000.0);
}
#endif /* HKU_SUPPORT_SERIALIZATION */

/** @par Test point: tocsv runs without throwing even when the account name contains
 * path separators (the name is sanitized, so the export cannot escape the target directory) */
TEST_CASE("test_TradeManager_tocsv") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000, TC_Zero(), "../evil/name");
    tm->buy(Datetime(199911170000), stock, 10.0, 100);
    tm->setParam<bool>("support_borrow_stock", true);
    tm->borrowStock(Datetime(199911170000), stock, 10.0, 200);
    tm->sellShort(Datetime(199911170000), stock, 10.0, 100);

    // tocsv never throws; the existence check is what pins the sanitization
    CHECK_NOTHROW(tm->tocsv(sm.tmpdir()));
    std::ifstream f(sm.tmpdir() + "/___evil_name_actions.txt");
    CHECK_UNARY(f.good());
}

/** @par Test points: short cost caliber (gross proceeds, fees only in totalCost), rebuild/live
 * parity, and totalRisk rules */
TEST_CASE("test_TradeManager_short_cost_caliber") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    double u = stock.unit();
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 1000000, TC_Zero());

    CostRecord c1;
    c1.commission = 10.0;
    c1.total = 10.0;
    TradeRecord o(stock, Datetime(199911100000), BUSINESS_SELL_SHORT, 10.0, 10.0, 0.0, 100, c1,
                  12.0, tm->currentCash(), PART_INVALID);
    CHECK_UNARY(tm->addTradeRecord(o));

    /** @arg gross sellMoney, fees only in totalCost, no open fee folded into buyMoney */
    PositionRecord p = tm->getShortPosition(stock);
    CHECK_EQ(p.sellMoney, roundEx(10.0 * 100 * u, 2));
    CHECK_EQ(p.buyMoney, 0.0);
    CHECK_EQ(p.totalCost, 10.0);
    /** @arg stoploss=12>0 -> totalRisk from the rebuild path too */
    CHECK_EQ(p.totalRisk, roundEx((12.0 - 10.0) * 100 * u, 2));

    /** @arg a partial cover records gross buyMoney and scales totalRisk by the remaining fraction
     */
    CostRecord c2;
    c2.commission = 8.0;
    c2.total = 8.0;
    TradeRecord cl(stock, Datetime(199911110000), BUSINESS_BUY_SHORT, 12.0, 12.0, 0.0, 40, c2, 12.0,
                   tm->currentCash(), PART_INVALID);
    CHECK_UNARY(tm->addTradeRecord(cl));
    p = tm->getShortPosition(stock);
    CHECK_EQ(p.number, 60.0);
    CHECK_EQ(p.buyMoney, roundEx(12.0 * 40 * u, 2));
    CHECK_EQ(p.totalCost, 18.0);
    CHECK_EQ(p.totalRisk, roundEx((12.0 - 10.0) * 100 * u * 60.0 / 100.0, 2));

    /** @arg after a full close totalProfit() = P1*N - P2*N - all fees is exact (no double count) */
    CostRecord c3;
    c3.commission = 6.0;
    c3.total = 6.0;
    TradeRecord cl2(stock, Datetime(199911120000), BUSINESS_BUY_SHORT, 11.0, 11.0, 0.0, 60, c3,
                    12.0, tm->currentCash(), PART_INVALID);
    CHECK_UNARY(tm->addTradeRecord(cl2));
    CHECK_EQ(tm->getShortPositionList().empty(), true);
    PositionRecordList hist = tm->getShortHistoryPositionList();
    REQUIRE_EQ(hist.size(), 1);
    price_t exp_profit = roundEx(10.0 * 100 * u, 2) -
                         roundEx(roundEx(12.0 * 40 * u, 2) + roundEx(11.0 * 60 * u, 2), 2) - 24.0;
    CHECK_EQ(hist[0].totalProfit(), exp_profit);
}

/** @par Test point: a full checkoutStock marks the closed position with a cleanDatetime and stores
 * it into the history (previously left Null, so totalProfit() errored and returned 0) */
TEST_CASE("test_TradeManager_checkout_clean_datetime") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 100000, TC_Zero());
    tm->buy(Datetime(199911150000), stock, 10.0, 100);
    CHECK_UNARY(tm->checkoutStock(Datetime(199911170000), stock, 11.0, 100));

    CHECK_EQ(tm->getPositionList().empty(), true);
    PositionRecordList hist = tm->getHistoryPositionList();
    REQUIRE_EQ(hist.size(), 1);
    /** @arg the closed record carries the checkout date, so totalProfit() computes normally */
    CHECK_EQ(hist[0].cleanDatetime, Datetime(199911170000));
    CHECK_EQ(hist[0].totalProfit(),
             roundEx(11.0 * 100 * stock.unit() - 10.0 * 100 * stock.unit(), 2));
}

/** @par Test point: getFundsList for a date before the account creation date yields an empty
 * record, consistent with getFunds */
TEST_CASE("test_TradeManager_getFundsList_before_init") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    TradeManagerPtr tm = crtTM(Datetime(199901100000), 100000, TC_Zero());
    tm->buy(Datetime(199911170000), stock, 10.0, 100);  // ensures the batch replay path is taken

    DatetimeList dates;
    dates.push_back(Datetime(199901010000));  // before init
    dates.push_back(Datetime(199911180000));  // after the buy
    FundsList fl = tm->getFundsList(dates);
    CHECK_EQ(fl[0].cash, 0.0);
    CHECK_EQ(fl[0].base_cash, 0.0);
    /** @arg matches getFunds on the same historical date */
    CHECK_EQ(fl[0].cash, tm->getFunds(Datetime(199901010000)).cash);
    CHECK_EQ(fl[1].cash, tm->getFunds(Datetime(199911180000)).cash);
}

/** @par Test point: incremental short-circuit must be idempotent — querying many times yields the
 * same state as querying once at the end (each weight event applied exactly once) */
TEST_CASE("test_TradeManager_updateWithWeight_incremental_idempotent") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    Datetime buy_dt(199911100000L);
    Datetime end_dt(200801010000L);

    // Reference: buy once, query cash only at end_dt (single scan covers every weight event)
    TradeManagerPtr one = crtTM(Datetime(199901010000), 100000, TC_Zero(), "ONE");
    one->buy(buy_dt, stock, 10.0, 100);
    one->cash(end_dt);

    // Incremental path: buy the same, then advance the account by many successive monthly queries
    TradeManagerPtr many = crtTM(Datetime(199901010000), 100000, TC_Zero(), "MANY");
    many->buy(buy_dt, stock, 10.0, 100);
    for (int y = 2000; y <= 2007; ++y) {
        for (int m = 1; m <= 12; ++m) {
            many->cash(Datetime(y, m, 1));
        }
    }
    many->cash(end_dt);

    /** @arg non-empty guard: the window must actually carry weight events, else this is vacuous */
    size_t n_weight = 0;
    for (const auto& t : one->getTradeList()) {
        if (t.business == BUSINESS_BONUS || t.business == BUSINESS_GIFT)
            ++n_weight;
    }
    CHECK_UNARY(n_weight > 0);

    /** @arg identical record-by-record: no duplicated, skipped or reordered weight events */
    const TradeRecordList& la = one->getTradeList();
    const TradeRecordList& lb = many->getTradeList();
    REQUIRE_EQ(la.size(), lb.size());
    for (size_t i = 0; i < la.size(); ++i) {
        CHECK_EQ(lb[i].datetime, la[i].datetime);
        CHECK_EQ(lb[i].business, la[i].business);
        CHECK_EQ(lb[i].realPrice, la[i].realPrice);
        CHECK_EQ(lb[i].number, la[i].number);
    }
    /** @arg same cash / holding / net assets after all events */
    CHECK_EQ(many->currentCash(), one->currentCash());
    CHECK_EQ(many->getHoldNumber(end_dt, stock), one->getHoldNumber(end_dt, stock));
    CHECK_EQ(many->getFunds(end_dt).net_assets(), one->getFunds(end_dt).net_assets());
    /** @arg the last BONUS record matches (same date and amount, not double-counted) */
    auto last_bonus = [](const TradeManagerPtr& tm) {
        TradeRecord last;
        for (const auto& tr : tm->getTradeList()) {
            if (tr.business == BUSINESS_BONUS)
                last = tr;
        }
        return last;
    };
    CHECK_EQ(last_bonus(many).datetime, last_bonus(one).datetime);
    CHECK_EQ(last_bonus(many).realPrice, last_bonus(one).realPrice);
}

/** @par Test point: the incremental cache must give long / borrow / short maps of the SAME stock
 * one shared window per update (pins the delayed write-back after all three loops) */
TEST_CASE("test_TradeManager_updateWithWeight_incremental_cross_map") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    Datetime buy_dt(199911100000L);
    Datetime end_dt(200801010000L);

    auto build = [&](TradeManagerPtr& tm) {
        tm = crtTM(Datetime(199901010000), 1000000, TC_Zero(), "X");
        tm->setParam<bool>("support_borrow_stock", true);
        tm->buy(buy_dt, stock, 10.0, 300);           // long holding
        tm->borrowStock(buy_dt, stock, 10.0, 1000);  // borrow pool
        // sells 600 out of the 1000 pool, no extra borrow; short holding = 600
        tm->sellShort(buy_dt, stock, 10.0, 600);
    };

    TradeManagerPtr one;
    build(one);
    one->cash(end_dt);

    TradeManagerPtr many;
    build(many);
    for (int y = 2000; y <= 2007; ++y) {
        for (int m = 1; m <= 12; ++m) {
            many->getDebtNumber(Datetime(y, m, 1), stock);
        }
    }
    many->cash(end_dt);

    /** @arg non-empty guard: window carries bonus / gift / borrow / short adjustments */
    size_t n = 0;
    for (const auto& t : one->getTradeList()) {
        switch (t.business) {
            case BUSINESS_BONUS:
            case BUSINESS_GIFT:
            case BUSINESS_BORROW_ADJUST:
            case BUSINESS_SHORT_ADJUST:
                ++n;
                break;
            default:
                break;
        }
    }
    CHECK_UNARY(n > 0);

    /** @arg long / borrow / short of the same stock all advance consistently */
    CHECK_EQ(many->getTradeList().size(), one->getTradeList().size());
    CHECK_EQ(many->getHoldNumber(end_dt, stock), one->getHoldNumber(end_dt, stock));
    CHECK_EQ(many->getDebtNumber(end_dt, stock), one->getDebtNumber(end_dt, stock));
    CHECK_EQ(many->getShortHoldNumber(end_dt, stock), one->getShortHoldNumber(end_dt, stock));
    /** @arg the whole-account net asset value matches too (TWR/CAGR/XIRR input) */
    CHECK_EQ(many->getFunds(end_dt).net_assets(), one->getFunds(end_dt).net_assets());
}

/** @par Test point: _reset clears the short/borrow state and _clone preserves it (regression:
 * reset left the short tables behind and clone dropped them) */
TEST_CASE("test_TradeManager_reset_clone_short_state") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");

    TradeManagerPtr tm = crtTM(Datetime(199901010000), 1000000, TC_Zero(), "S");
    tm->borrowStock(Datetime(199911100000L), stock, 10.0, 500);
    tm->sellShort(Datetime(199911100000L), stock, 10.0, 300);

    /** @arg clone preserves the short position and the borrow debt */
    TradeManagerPtr cloned = tm->clone();
    CHECK_EQ(cloned->getShortHoldNumber(Datetime(199911100000L), stock), 300.0);
    CHECK_EQ(cloned->getDebtNumber(Datetime(199911100000L), stock), 500.0);

    /** @arg reset (via an INIT record) clears the short position and the borrow debt */
    TradeRecord init(Null<Stock>(), Datetime(199801010000L), BUSINESS_INIT, 777777, 777777, 0, 0,
                     CostRecord(), 0, 777777, PART_INVALID);
    CHECK_UNARY(tm->addTradeRecord(init));
    CHECK_EQ(tm->getShortHoldNumber(Datetime(199911100000L), stock), 0.0);
    CHECK_EQ(tm->getDebtNumber(Datetime(199911100000L), stock), 0.0);
}

/** @par Test points */
TEST_CASE("test_TradeManager_profit_cum_change_curve") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    DatetimeList dates = {Datetime(199911170000), Datetime(199911180000)};

    /** @arg An account that was never invested returns an undefined (NaN) curve, not a fake 0 */
    TradeManagerPtr tm = crtTM(Datetime(199901010000), 0, TC_Zero(), "TEST");
    PriceList curve = tm->getProfitCumChangeCurve(dates);
    REQUIRE_EQ(curve.size(), 2);
    CHECK_UNARY(std::isnan(curve[0]));
    CHECK_UNARY(std::isnan(curve[1]));

    /** @arg The curve is total assets over the invested base, rounded by the account precision */
    tm = crtTM(Datetime(199901010000), 100000, TC_Zero(), "TEST");
    tm->buy(Datetime(199911170000), stock, 10.0, 100);
    curve = tm->getProfitCumChangeCurve(dates);
    FundsList funds = tm->getFundsList(dates);
    REQUIRE_EQ(curve.size(), funds.size());
    for (size_t i = 0, total = funds.size(); i < total; ++i) {
        CHECK_EQ(curve[i], roundEx(funds[i].total_assets() / funds[i].total_base(), 2));
    }
}

/** @} */
