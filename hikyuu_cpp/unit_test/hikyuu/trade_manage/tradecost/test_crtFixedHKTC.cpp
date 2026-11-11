/*
 * test_crtFixedHKTC.cpp
 *
 *  Created on: 2026-11-11
 *      Author: fasiondog
 */
#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/config.h>
#include <hikyuu/utilities/exception.h>
#include <hikyuu/trade_manage/crtTM.h>
#include <hikyuu/trade_manage/tradecost/crt/TC_HK.h>
#include <hikyuu/trade_manage/margin/crt/MG_Fixed.h>

#if HKU_SUPPORT_SERIALIZATION
#include <fstream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#endif

using namespace hku;

/**
 * @defgroup test_crtFixedHKTC test_crtFixedHKTC
 * @ingroup test_hikyuu_trade_manage_suite
 * @{
 */

/** @par Test points: the Hong Kong model charges every fee on both sides, so the buy and the sell
 * records are identical, and each fee lands in its own bucket */
TEST_CASE("test_TC_HK") {
    StockManager& sm = StockManager::instance();
    Stock sh = sm.getStock("sh600004");
    REQUIRE_UNARY(!sh.isNull());
    const Datetime dt(201001010000);

    /** @arg All parameters zero: a zero cost in both directions */
    TradeCostPtr zero = TC_HK(0.0, 0.0, 0.0, 0.0, 0.0);
    CHECK_EQ(zero->name(), "TC_HK");
    CHECK_EQ(zero->getBuyCost(dt, sh, 10.0, 1000).total, 0.0);
    CHECK_EQ(zero->getSellCost(dt, sh, 10.0, 1000).total, 0.0);

    /** @arg Buy: the commission below the floor is lifted, the stamp duty and the tradefee are
     * charged on the turnover value and the fixed per-trade fee is added */
    TradeCostPtr cost = TC_HK(0.0003, 5.0, 0.001, 0.00005, 2.0);
    CostRecord result = cost->getBuyCost(dt, sh, 10.0, 1000);
    CostRecord expect;
    expect.commission = 5.0;   // 10000 * 0.0003 = 3.0, lifted to the floor
    expect.stamptax = 10.0;    // 10000 * 0.001, double-sided
    expect.transferfee = 0.5;  // 10000 * 0.00005
    expect.others = 2.0;
    expect.total = 17.5;
    CHECK_EQ(result, expect);

    /** @arg Sell: identical to the buy record — every fee of the model is double-sided */
    CHECK_EQ(cost->getSellCost(dt, sh, 10.0, 1000), result);

    /** @arg The commission above the floor is used as computed */
    CHECK_EQ(TC_HK(0.001, 5.0)->getBuyCost(dt, sh, 10.0, 1000).commission, 10.0);

    /** @arg Zero quantity: no turnover value, so no cost */
    CHECK_EQ(cost->getBuyCost(dt, sh, 10.0, 0.0).total, 0.0);
    CHECK_EQ(cost->getSellCost(dt, sh, 10.0, 0.0).total, 0.0);

    /** @arg Clone keeps the parameters and the name */
    TradeCostPtr cloned = cost->clone();
    CHECK_EQ(cloned->name(), cost->name());
    CHECK_EQ(cloned->getParam<price_t>("stamptax"), 0.001);
    CHECK_EQ(cloned->getParam<price_t>("tradefee"), 0.00005);
    CHECK_EQ(cloned->getSellCost(dt, sh, 10.0, 1000), cost->getSellCost(dt, sh, 10.0, 1000));
}

/** @par Test points: the contract multiplier scales the turnover-based fees while the per-trade
 * fee stays flat */
TEST_CASE("test_TC_HK_unit_multiplier") {
    StockManager& sm = StockManager::instance();
    Stock sh = sm.getStock("sh600004");
    REQUIRE_UNARY(!sh.isNull());
    const Datetime dt(201001010000);

    Stock fut("SHF", "TEST01", "test contract", 12, true, Datetime(201001010000),
              Datetime(203001010000), 1.0, 10.0, 2, 1, 10000);
    fut.setKDataDriver(sh.getKDataDirver());  // a driver is part of a non-null stock
    REQUIRE_UNARY(!fut.isNull());
    CHECK_EQ(fut.unit(), 10.0);

    TradeCostPtr cost = TC_HK(0.0001, 0.0, 0.001, 0.00005, 3.0);
    CostRecord result = cost->getBuyCost(dt, fut, 4000.0, 10);  // 4000 * 10 * 10 = 400000
    CostRecord expect;
    expect.commission = 40.0;
    expect.stamptax = 400.0;
    expect.transferfee = 20.0;
    expect.others = 3.0;
    expect.total = 463.0;
    CHECK_EQ(result, expect);
}

/** @par Test points: the commission floor only lifts the value below it, equality is kept as is */
TEST_CASE("test_TC_HK_lowest_commission") {
    StockManager& sm = StockManager::instance();
    Stock sh = sm.getStock("sh600004");
    REQUIRE_UNARY(!sh.isNull());
    const Datetime dt(201001010000);

    /** @arg Below the floor: clamped to lowest_commission */
    CHECK_EQ(TC_HK(0.0004, 5.0)->getBuyCost(dt, sh, 10.0, 1000).commission, 5.0);

    /** @arg Exactly at the floor: kept (the comparison is strict) */
    CHECK_EQ(TC_HK(0.0005, 5.0)->getBuyCost(dt, sh, 10.0, 1000).commission, 5.0);

    /** @arg Above the floor: the computed commission is used */
    CHECK_EQ(TC_HK(0.0006, 5.0)->getBuyCost(dt, sh, 10.0, 1000).commission, 6.0);
}

/** @par Test points: every parameter rejects a negative value */
TEST_CASE("test_TC_HK_param") {
    TradeCostPtr cost = TC_HK();

    /** @arg A negative value of any parameter raises */
    CHECK_THROWS_AS(cost->setParam<price_t>("commission", -0.1), hku::exception);
    CHECK_THROWS_AS(cost->setParam<price_t>("lowest_commission", -1.0), hku::exception);
    CHECK_THROWS_AS(cost->setParam<price_t>("stamptax", -0.1), hku::exception);
    CHECK_THROWS_AS(cost->setParam<price_t>("tradefee", -0.1), hku::exception);
    CHECK_THROWS_AS(cost->setParam<price_t>("per_trade", -1.0), hku::exception);
    CHECK_THROWS_AS(cost->setParam<price_t>("settlefee", -0.1), hku::exception);
    CHECK_THROWS_AS(cost->setParam<price_t>("settlefee_min", -1.0), hku::exception);
    CHECK_THROWS_AS(cost->setParam<price_t>("settlefee_max", -1.0), hku::exception);
}

/** @par Test points: the stamp duty is rounded up to a whole HK dollar, and the floating point
 * noise at an exact dollar amount does not add a dollar */
TEST_CASE("test_TC_HK_stamp_duty_round_up") {
    StockManager& sm = StockManager::instance();
    Stock sh = sm.getStock("sh600004");
    REQUIRE_UNARY(!sh.isNull());
    const Datetime dt(201001010000);
    TradeCostPtr cost = TC_HK(0.0, 0.0, 0.001, 0.0, 0.0);

    /** @arg 1050 turnover: 1.05 -> charged as 2 (rounded up to a whole dollar) */
    CHECK_EQ(cost->getBuyCost(dt, sh, 1.05, 1000).stamptax, 2.0);

    /** @arg An exact dollar amount is kept (the float noise of 10000 * 0.001 must not round up) */
    CHECK_EQ(cost->getBuyCost(dt, sh, 10.0, 1000).stamptax, 10.0);

    /** @arg 10005 turnover: 10.005 -> charged as 11 */
    CHECK_EQ(cost->getBuyCost(dt, sh, 10.005, 1000).stamptax, 11.0);

    /** @arg A zero rate charges nothing */
    CHECK_EQ(TC_HK(0.0, 0.0, 0.0, 0.0, 0.0)->getBuyCost(dt, sh, 10.0, 1000).stamptax, 0.0);
}

/** @par Test points: the settlement fee is charged as a ratio and bounded per order; a bound of
 * 0 means the bound is not applied, and a zero ratio charges nothing */
TEST_CASE("test_TC_HK_settlement_fee") {
    StockManager& sm = StockManager::instance();
    Stock sh = sm.getStock("sh600004");
    REQUIRE_UNARY(!sh.isNull());
    const Datetime dt(201001010000);

    TradeCostPtr cost = TC_HK(0.0, 0.0, 0.0, 0.0, 0.0, 0.00002, 2.0, 100.0);

    /** @arg 1,000,000 turnover x 0.002% = 20, inside the bounds */
    CHECK_EQ(cost->getBuyCost(dt, sh, 1000.0, 1000).transferfee, 20.0);

    /** @arg 10,000 turnover x 0.002% = 0.2, lifted to the minimum 2 */
    CHECK_EQ(cost->getBuyCost(dt, sh, 10.0, 1000).transferfee, 2.0);

    /** @arg 10,000,000 turnover x 0.002% = 200, capped at the maximum 100 */
    CHECK_EQ(cost->getBuyCost(dt, sh, 10000.0, 1000).transferfee, 100.0);

    /** @arg The fee is charged on the sell side as well */
    CHECK_EQ(cost->getSellCost(dt, sh, 10.0, 1000).transferfee, 2.0);

    /** @arg A zero ratio charges nothing even with the bounds set */
    CHECK_EQ(TC_HK(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 2.0, 100.0)
               ->getBuyCost(dt, sh, 10.0, 1000)
               .transferfee,
             0.0);

    /** @arg A bound of 0 is not applied: the raw ratio is kept above and below */
    CHECK_EQ(TC_HK(0.0, 0.0, 0.0, 0.0, 0.0, 0.00002)->getBuyCost(dt, sh, 10.0, 1000).transferfee,
             0.2);
    CHECK_EQ(TC_HK(0.0, 0.0, 0.0, 0.0, 0.0, 0.00002, 0.0, 0.0)
               ->getBuyCost(dt, sh, 10000.0, 1000)
               .transferfee,
             200.0);
}

/** @par Test points: the default parameters follow the current Hong Kong fee schedule */
TEST_CASE("test_TC_HK_default_param") {
    TradeCostPtr cost = TC_HK();

    /** @arg The exchange fee plus the SFC / AFIRD levies add up to 0.0085% per side */
    CHECK_EQ(cost->getParam<price_t>("tradefee"), 0.000085);

    /** @arg The settlement fee and its bounds are not charged by default */
    CHECK_EQ(cost->getParam<price_t>("settlefee"), 0.0);
    CHECK_EQ(cost->getParam<price_t>("settlefee_min"), 0.0);
    CHECK_EQ(cost->getParam<price_t>("settlefee_max"), 0.0);
}

/** @par Test points: the account charges the same record through all four trade entries. The
 * short entries need a future security (the spot stock rejects them), so the case runs on
 * SHFRB0000 and the model treats every security the same way */
TEST_CASE("test_TC_HK_trade_manager") {
    StockManager& sm = StockManager::instance();
    Stock rb = sm.getStock("SHFRB0000");
    REQUIRE_UNARY(!rb.isNull());
    KRecord bar = rb.getKRecord(100, KQuery::DAY);
    REQUIRE_UNARY(bar.isValid());

    TradeCostPtr cost = TC_HK(0.0003, 0.0, 0.001, 0.00005, 2.0);
    auto tm = crtTM(bar.datetime, 1000000.0, cost, "SYS", MG_Fixed(0.5));

    /** @arg buy charges getBuyCost */
    TradeRecord tr = tm->buy(bar.datetime, rb, bar.closePrice, 10);
    CHECK_EQ(tr.business, BUSINESS_BUY);
    CHECK_EQ(tr.cost, cost->getBuyCost(bar.datetime, rb, bar.closePrice, 10));

    /** @arg sell charges getSellCost */
    tr = tm->sell(bar.datetime, rb, bar.closePrice, 10);
    CHECK_EQ(tr.business, BUSINESS_SELL);
    CHECK_EQ(tr.cost, cost->getSellCost(bar.datetime, rb, bar.closePrice, 10));

    /** @arg sellShort charges getSellCost as well */
    tr = tm->sellShort(bar.datetime, rb, bar.closePrice, 10);
    CHECK_EQ(tr.business, BUSINESS_SELL_SHORT);
    CHECK_EQ(tr.cost, cost->getSellCost(bar.datetime, rb, bar.closePrice, 10));

    /** @arg buyShort charges getBuyCost as well */
    tr = tm->buyShort(bar.datetime, rb, bar.closePrice, 10);
    CHECK_EQ(tr.business, BUSINESS_BUY_SHORT);
    CHECK_EQ(tr.cost, cost->getBuyCost(bar.datetime, rb, bar.closePrice, 10));
}

#if HKU_SUPPORT_SERIALIZATION
/** @par Test points: the serialization round trip keeps the parameters */
TEST_CASE("test_FixedHKTradeCost_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/TC_HK.xml";

    TradeCostPtr cost = TC_HK(0.0003, 5.0, 0.001, 0.00005, 2.0);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(cost);
    }

    TradeCostPtr cost2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(cost2);
    }

    Stock sh = sm.getStock("sh600004");
    const Datetime dt(201001010000);
    CHECK_EQ(cost2->name(), "TC_HK");
    CHECK_EQ(cost2->getParam<price_t>("tradefee"), 0.00005);
    CHECK_EQ(cost2->getSellCost(dt, sh, 10.0, 1000), cost->getSellCost(dt, sh, 10.0, 1000));
}
#endif /* HKU_SUPPORT_SERIALIZATION */

/** @} */
