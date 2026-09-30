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

    /** @arg empty asset info resets cash and clears all positions */
    OrderBrokerPtr empty_broker = make_shared<AssetOrderBroker>("");
    BrokerTradeManager tm2(empty_broker);
    tm2.fetchAssetInfoFromBroker(empty_broker);
    CHECK_EQ(tm2.getStockNumber(), 0);
    CHECK_EQ(tm2.currentCash(), 0.0);
}

/** @} */  // end of test_BrokerTradeManager
