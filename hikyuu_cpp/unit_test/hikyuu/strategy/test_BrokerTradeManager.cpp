/*
 * test_BrokerTradeManager.cpp
 *
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2026-09-26
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/OrderBrokerBase.h>
#include <hikyuu/strategy/BrokerTradeManager.h>

using namespace hku;

namespace {

class AssetOrderBroker final : public OrderBrokerBase {
public:
    explicit AssetOrderBroker(const string& asset) : m_asset(asset) {}

    void _buy(Datetime, const string&, const string&, price_t, double, price_t, price_t, SystemPart,
              const string&) override {}

    void _sell(Datetime, const string&, const string&, price_t, double, price_t, price_t,
               SystemPart, const string&) override {}

    string _getAssetInfo() override {
        return m_asset;
    }

private:
    string m_asset;
};

}  // namespace

/**
 * @defgroup test_BrokerTradeManager test_BrokerTradeManager
 * @ingroup test_hikyuu_trade_manage_suite
 * @{
 */

/** @par Test points: fetchAssetInfoFromBroker computes totalRisk with the correct sign and
 * stock.unit() scale */
TEST_CASE("test_BrokerTradeManager_fetchAssetInfoFromBroker_risk") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    REQUIRE_UNARY(!stock.isNull());

    // number is in lots, cost_price and stoploss are per share
    const double number = 10.0;
    const price_t cost_price = 10.0;
    const price_t stoploss = 5.0;
    const price_t unit = stock.unit();
    const int precision = 2;

    string asset = R"({
        "cash": 100000.0,
        "positions": [
            {"market": "SH", "code": "600000", "number": 10.0,
             "stoploss": 5.0, "goal_price": 0.0, "cost_price": 10.0}
        ]
    })";

    OrderBrokerPtr broker = make_shared<AssetOrderBroker>(asset);
    BrokerTradeManager tm(broker);
    tm.fetchAssetInfoFromBroker(broker);

    /** @arg the position is imported from the broker snapshot */
    CHECK_UNARY(tm.have(stock));
    PositionRecord pos = tm.getPosition(Null<Datetime>(), stock);

    /** @arg totalRisk is positive when cost_price exceeds stoploss (regression for the flipped
     * sign) */
    CHECK_UNARY(pos.totalRisk > 0.0);

    /** @arg totalRisk = (cost_price - stoploss) * number * unit */
    CHECK_EQ(pos.totalRisk, roundEx((cost_price - stoploss) * number * unit, precision));

    /** @arg buyMoney scales by stock.unit() (regression for the missing unit) */
    CHECK_EQ(pos.buyMoney, roundEx(number * cost_price * unit, precision));

    /** @arg a failed (empty) fetch on a fresh instance keeps the initial (empty) state */
    OrderBrokerPtr empty_broker = make_shared<AssetOrderBroker>("");
    BrokerTradeManager tm2(empty_broker);
    tm2.fetchAssetInfoFromBroker(empty_broker);
    CHECK_EQ(tm2.getStockNumber(), 0);
    CHECK_EQ(tm2.currentCash(), 0.0);
}

/** @par Test points: a failed fetch must NOT wipe the local account (TM-004 regression: the old
 * code zeroed the cash and dropped all positions on any empty/failed broker response) */
TEST_CASE("test_BrokerTradeManager_fetchAssetInfoFromBroker_keep_state_on_failure") {
    Stock stock = StockManager::instance().getStock("sh600000");
    REQUIRE_UNARY(!stock.isNull());

    string asset = R"({
        "cash": 100000.0,
        "positions": [
            {"market": "SH", "code": "600000", "number": 10.0,
             "stoploss": 5.0, "goal_price": 0.0, "cost_price": 10.0}
        ]
    })";

    class MockBroker final : public OrderBrokerBase {
    public:
        string _getAssetInfo() override {
            if (m_throw) {
                throw std::runtime_error("network timeout");
            }
            return m_asset;
        }

        void _buy(Datetime, const string&, const string&, price_t, double, price_t, price_t,
                  SystemPart, const string&) override {}
        void _sell(Datetime, const string&, const string&, price_t, double, price_t, price_t,
                   SystemPart, const string&) override {}

        string m_asset;
        bool m_throw = false;
    };

    auto broker = make_shared<MockBroker>();
    broker->m_asset = asset;
    BrokerTradeManager tm(broker);

    /** @arg a successful sync imports the account state */
    tm.fetchAssetInfoFromBroker(broker);
    CHECK_EQ(tm.currentCash(), 100000.0);
    CHECK_EQ(tm.getStockNumber(), 1);
    CHECK_UNARY(broker->getLastError().empty());

    /** @arg an exception from the broker keeps the last known state; getAssetInfo no longer
     * swallows it, so the caller sees the real error */
    broker->m_throw = true;
    CHECK_THROWS_AS(broker->getAssetInfo(), std::runtime_error);
    tm.fetchAssetInfoFromBroker(broker);
    CHECK_EQ(tm.currentCash(), 100000.0);
    CHECK_EQ(tm.getStockNumber(), 1);

    /** @arg an empty (failed) response keeps the last known state as well */
    broker->m_throw = false;
    broker->m_asset.clear();
    tm.fetchAssetInfoFromBroker(broker);
    CHECK_EQ(tm.currentCash(), 100000.0);
    CHECK_EQ(tm.getStockNumber(), 1);

    /** @arg a malformed json response keeps the last known state */
    broker->m_asset = "{not a json";
    tm.fetchAssetInfoFromBroker(broker);
    CHECK_EQ(tm.currentCash(), 100000.0);
    CHECK_EQ(tm.getStockNumber(), 1);

    /** @arg the state is refreshable again once the broker recovers */
    broker->m_asset = asset;
    tm.fetchAssetInfoFromBroker(broker);
    CHECK_EQ(tm.currentCash(), 100000.0);
    CHECK_EQ(tm.getStockNumber(), 1);
    CHECK_UNARY(broker->getLastError().empty());
}

/** @par Test points: getFunds(datetime) revalues the snapshot holdings at the queried moment (the
 * cash comes from the snapshot; the market value follows the price of `datetime`), instead of
 * returning zeros when the query time is earlier than the snapshot time or the snapshot-time value.
 */
TEST_CASE("test_BrokerTradeManager_getFunds_by_datetime") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    REQUIRE_UNARY(!stock.isNull());

    const double number = 10.0;
    const price_t unit = stock.unit();
    const int precision = 2;

    string asset = R"({
        "cash": 100000.0,
        "positions": [
            {"market": "SH", "code": "600000", "number": 10.0,
             "stoploss": 5.0, "goal_price": 0.0, "cost_price": 10.0}
        ]
    })";

    OrderBrokerPtr broker = make_shared<AssetOrderBroker>(asset);
    BrokerTradeManager tm(broker);
    tm.fetchAssetInfoFromBroker(broker);

    /** @arg regression: a query earlier than the snapshot time is no longer an all-zero record
     * (the old (datetime >= m_datetime) guard wrongly returned zeros for any earlier query, e.g.
     * the daily bar time 00:00 vs the snapshot time with intraday seconds) */
    Datetime day_bar_dt = tm.lastDatetime() - TimeDelta(1);
    REQUIRE_UNARY(day_bar_dt < tm.lastDatetime());
    FundsRecord funds = tm.getFunds(day_bar_dt, KQuery::DAY);
    CHECK_EQ(funds.cash, 100000.0);
    /** @arg the dated query yields a positive close-based market value (the old code returned 0) */
    CHECK_UNARY(funds.market_value > 0.0);
    CHECK_UNARY(funds.total_assets() > 0.0);

    /** @arg a Null datetime falls back to the current snapshot value (cash and market value) */
    FundsRecord null_dt = tm.getFunds(Null<Datetime>(), KQuery::DAY);
    CHECK_EQ(null_dt.cash, tm.getFunds().cash);
    CHECK_EQ(null_dt.market_value, tm.getFunds().market_value);

    // Pick two real trading days with different closes from the stock history (both before the
    // snapshot time) to verify the dated query values the holdings at each date's own price.
    KData k = stock.getKData(KQuery(-30, -1, KQuery::DAY));
    REQUIRE(k.size() > 1);
    size_t ib = 1;
    while (ib < k.size() && k[ib].closePrice == k[0].closePrice) {
        ++ib;
    }
    /** @arg pin the sensitivity: two different closes exist in the window */
    REQUIRE_UNARY(ib < k.size());
    Datetime da = k[0].datetime;
    Datetime db = k[ib].datetime;
    price_t pa = stock.getMarketValue(da, "DAY");
    price_t pb = stock.getMarketValue(db, "DAY");
    REQUIRE_UNARY(pa > 0.0 && pb > 0.0);
    REQUIRE_UNARY(pa != pb);

    /** @arg market_value == number * price(datetime) * unit (valued at the queried moment) */
    CHECK_EQ(tm.getFunds(da, KQuery::DAY).market_value, roundEx(number * pa * unit, precision));
    CHECK_EQ(tm.getFunds(db, KQuery::DAY).market_value, roundEx(number * pb * unit, precision));

    /** @arg the assets drift with time: two dates with different prices give different market value
     */
    CHECK_UNARY(tm.getFunds(da, KQuery::DAY).market_value !=
                tm.getFunds(db, KQuery::DAY).market_value);
}

/** @par Test points: market orders (realPrice == 0) are accounted at the latest close as the
 * estimated fill price; the cash flow is only applied when the fill price is known */
TEST_CASE("test_BrokerTradeManager_market_order_accounting") {
    Stock stock = StockManager::instance().getStock("sh600000");
    REQUIRE_UNARY(!stock.isNull());

    const price_t unit = stock.unit();
    const int precision = 2;

    KData last_k = stock.getKData(KQuery(-1));
    REQUIRE_UNARY(!last_k.empty());
    const price_t ref_price = last_k.back().closePrice;
    REQUIRE_UNARY(ref_price > 0.0);

    OrderBrokerPtr broker = make_shared<AssetOrderBroker>("");
    BrokerTradeManager tm(broker);
    tm.checkin(Datetime::now(), 1000000.0);

    /** @arg a market-order buy accounts the position cost at the latest close and leaves the cash
     * untouched */
    TradeRecord tr = tm.buy(Datetime::now(), stock, 0.0, 100.0);
    CHECK_UNARY(tr.business == BUSINESS_BUY);
    PositionRecord pos = tm.getPosition(Null<Datetime>(), stock);
    CHECK_EQ(pos.buyMoney, roundEx(ref_price * 100.0 * unit, precision));
    CHECK_EQ(tm.currentCash(), 1000000.0);

    /** @arg a market-order sell accounts sellMoney at the latest close and leaves the cash
     * untouched */
    TradeRecord tr_sell = tm.sell(Datetime::now(), stock, 0.0, 100.0);
    CHECK_UNARY(tr_sell.business == BUSINESS_SELL);
    CHECK_EQ(tm.currentCash(), 1000000.0);
    CHECK_UNARY(!tm.have(stock));

    /** @arg a known-price buy still deducts the cash */
    TradeRecord tr_limit = tm.buy(Datetime::now(), stock, ref_price, 100.0);
    CHECK_UNARY(tr_limit.business == BUSINESS_BUY);
    CHECK_UNARY(tm.currentCash() < 1000000.0);
    pos = tm.getPosition(Null<Datetime>(), stock);
    CHECK_EQ(pos.buyMoney, roundEx(ref_price * 100.0 * unit, precision));

    /** @arg an invalid (negative) price is rejected */
    TradeRecord tr_neg = tm.buy(Datetime::now(), stock, -1.0, 100.0);
    CHECK_UNARY(tr_neg.business == BUSINESS_INVALID);

    /** @arg str() prints positions without a valuation error after market-order accounting */
    CHECK_UNARY(!tm.str().empty());
}

/** @par Test points: checkin/checkout cash validation (a negative checkin must not bypass the
 * checkout clamp, and an over-amount checkout is clamped with a warning) */
TEST_CASE("test_BrokerTradeManager_checkin_checkout") {
    OrderBrokerPtr broker = make_shared<AssetOrderBroker>("");
    BrokerTradeManager tm(broker);

    /** @arg checkin rejects a non-positive or non-finite cash */
    CHECK_UNARY(!tm.checkin(Datetime::now(), -1000.0));
    CHECK_UNARY(!tm.checkin(Datetime::now(), 0.0));
    CHECK_UNARY(!tm.checkin(Datetime::now(), std::numeric_limits<double>::quiet_NaN()));
    CHECK_EQ(tm.currentCash(), 0.0);

    /** @arg a valid checkin adds the cash */
    CHECK_UNARY(tm.checkin(Datetime::now(), 100000.0));
    CHECK_EQ(tm.currentCash(), 100000.0);

    /** @arg checkout rejects a non-positive or non-finite cash */
    CHECK_UNARY(!tm.checkout(Datetime::now(), -1.0));
    CHECK_UNARY(!tm.checkout(Datetime::now(), 0.0));
    CHECK_UNARY(!tm.checkout(Datetime::now(), std::numeric_limits<double>::infinity()));
    CHECK_EQ(tm.currentCash(), 100000.0);

    /** @arg a checkout exceeding the current cash is clamped to 0 */
    CHECK_UNARY(tm.checkout(Datetime::now(), 200000.0));
    CHECK_EQ(tm.currentCash(), 0.0);

    /** @arg a normal checkout deducts the cash */
    CHECK_UNARY(tm.checkin(Datetime::now(), 50000.0));
    CHECK_UNARY(tm.checkout(Datetime::now(), 20000.0));
    CHECK_EQ(tm.currentCash(), 30000.0);
}

/** @} */  // end of test_BrokerTradeManager
