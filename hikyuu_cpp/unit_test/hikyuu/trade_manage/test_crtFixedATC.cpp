/*
 * test_TC_FixedA.cpp
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */
#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/StockTypeInfo.h>
#include <hikyuu/trade_manage/crt/TC_FixedA.h>

#include <hikyuu/config.h>
#if HKU_SUPPORT_SERIALIZATION
#include <fstream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#endif

using namespace hku;

/**
 * @defgroup test_TC_FixedA test_TC_FixedA
 * @ingroup test_hikyuu_trade_manage_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_TC_FixedA") {
    StockManager& sm = StockManager::instance();
    Stock stock;
    CostRecord result, expect;
    TradeCostPtr cost_func = TC_FixedA(0.0018, 5, 0.001, 0.001, 1.0);

    /** @arg Stock is Null */
    result = cost_func->getBuyCost(Datetime(200101010000), stock, 10.0, 100);
    CHECK_EQ(result, expect);
    result = cost_func->getSellCost(Datetime(200101010000), stock, 10.0, 1000);
    CHECK_EQ(result, expect);

    /** @arg Buy a Shanghai stock, fewer than 1000 shares, a commission below 5 */
    stock = sm.getStock("sh600004");
    result = cost_func->getBuyCost(Datetime(200101010000), stock, 10.0, 100);
    expect.commission = 5.0;
    expect.stamptax = 0.0;
    expect.transferfee = 1.0;
    expect.total = 6.0;
    CHECK_EQ(result, expect);

    /** @arg Buy a Shanghai stock, exactly 1000 shares, a commission above 5 */
    result = cost_func->getBuyCost(Datetime(200101010000), stock, 10.0, 1000);
    expect.commission = 18.0;
    expect.stamptax = 0.0;
    expect.transferfee = 1.0;
    expect.total = 19.0;
    CHECK_EQ(result, expect);

    /** @arg Buy a Shanghai stock, more than 1000 shares, a commission above 5 */
    result = cost_func->getBuyCost(Datetime(200101010000), stock, 10.0, 2100);
    expect.commission = 37.80;
    expect.stamptax = 0.0;
    expect.transferfee = 2.1;
    expect.total = 39.9;
    CHECK_EQ(result, expect);

    /** @arg Sell a Shanghai stock, fewer than 1000 shares, a commission below 5 */
    stock = sm.getStock("sh600004");
    result = cost_func->getSellCost(Datetime(200101010000), stock, 10.0, 100);
    expect.commission = 5.0;
    expect.stamptax = 1.0;
    expect.transferfee = 1.0;
    expect.total = 7.0;
    CHECK_EQ(result, expect);

    /** @arg Sell a Shanghai stock, exactly 1000 shares, a commission above 5 */
    result = cost_func->getSellCost(Datetime(200101010000), stock, 10.0, 1000);
    expect.commission = 18.0;
    expect.stamptax = 10.0;
    expect.transferfee = 1.0;
    expect.total = 29.0;
    CHECK_EQ(result, expect);

    /** @arg Sell a Shanghai stock, more than 1000 shares, a commission above 5 */
    result = cost_func->getSellCost(Datetime(200101010000), stock, 10.0, 2100);
    expect.commission = 37.80;
    expect.stamptax = 21;
    expect.transferfee = 2.1;
    expect.total = 60.9;
    CHECK_EQ(result, expect);

    /** @arg Buy a Shenzhen stock, fewer than 1000 shares, a commission below 5 */
    stock = sm.getStock("sz000001");
    result = cost_func->getBuyCost(Datetime(200101010000), stock, 10.0, 100);
    expect.commission = 5.0;
    expect.stamptax = 0.0;
    expect.transferfee = 0.0;
    expect.total = 5.0;
    CHECK_EQ(result, expect);

    /** @arg Buy a Shenzhen stock, exactly 1000 shares, a commission above 5 */
    result = cost_func->getBuyCost(Datetime(200101010000), stock, 10.0, 1000);
    expect.commission = 18.0;
    expect.stamptax = 0.0;
    expect.transferfee = 0.0;
    expect.total = 18.0;
    CHECK_EQ(result, expect);

    /** @arg Buy a Shenzhen stock, more than 1000 shares, a commission above 5 */
    result = cost_func->getBuyCost(Datetime(200101010000), stock, 10.0, 2100);
    expect.commission = 37.80;
    expect.stamptax = 0.0;
    expect.transferfee = 0.0;
    expect.total = 37.8;
    CHECK_EQ(result, expect);

    /** @arg Sell a Shenzhen stock, fewer than 1000 shares, a commission below 5 */
    result = cost_func->getSellCost(Datetime(200101010000), stock, 10.0, 100);
    expect.commission = 5.0;
    expect.stamptax = 1.0;
    expect.transferfee = 0.0;
    expect.total = 6.0;
    CHECK_EQ(result, expect);

    /** @arg Sell a Shenzhen stock, exactly 1000 shares, a commission above 5 */
    result = cost_func->getSellCost(Datetime(200101010000), stock, 10.0, 1000);
    expect.commission = 18.0;
    expect.stamptax = 10.0;
    expect.transferfee = 0.0;
    expect.total = 28.0;
    CHECK_EQ(result, expect);

    /** @arg Sell a Shenzhen stock, more than 1000 shares, a commission above 5 */
    result = cost_func->getSellCost(Datetime(200101010000), stock, 10.0, 2100);
    expect.commission = 37.80;
    expect.stamptax = 21;
    expect.transferfee = 0.0;
    expect.total = 58.8;
    CHECK_EQ(result, expect);
}

/** @par Test points */
TEST_CASE("test_TC_FixedA_stamptax_type") {
    // sell-side stamp duty applies to A/GEM/STAR/BSE; B-shares are not taxed
    StockManager& sm = StockManager::instance();
    CostRecord result, expect;
    TradeCostPtr cost_func = TC_FixedA(0.0018, 5, 0.001, 0.001, 1.0);
    const Datetime dt(200101010000);
    // borrow a real stock's data driver so the synthetic typed stocks are not null
    auto driver = sm.getStock("sh600004").getKDataDirver();
    auto makeStock = [&](const string& market, const string& code, uint32_t type) {
        Stock s(market, code, code, type, true, dt, dt);
        s.setKDataDriver(driver);
        return s;
    };

    /** @arg GEM (Shenzhen) sell: stamp duty charged, no transfer fee */
    Stock gem = makeStock("SZ", "300001", STOCKTYPE_GEM);
    result = cost_func->getSellCost(dt, gem, 10.0, 1000);
    expect = CostRecord();
    expect.commission = 18.0;
    expect.stamptax = 10.0;
    expect.transferfee = 0.0;
    expect.total = 28.0;
    CHECK_EQ(result, expect);

    /** @arg STAR (Shanghai) sell: stamp duty charged plus Shanghai transfer fee */
    Stock star = makeStock("SH", "688001", STOCKTYPE_START);
    result = cost_func->getSellCost(dt, star, 10.0, 1000);
    expect = CostRecord();
    expect.commission = 18.0;
    expect.stamptax = 10.0;
    expect.transferfee = 1.0;
    expect.total = 29.0;
    CHECK_EQ(result, expect);

    /** @arg Beijing Stock Exchange (BJ) sell: stamp duty charged, no transfer fee */
    Stock bj = makeStock("BJ", "830001", STOCKTYPE_A_BJ);
    result = cost_func->getSellCost(dt, bj, 10.0, 1000);
    expect = CostRecord();
    expect.commission = 18.0;
    expect.stamptax = 10.0;
    expect.transferfee = 0.0;
    expect.total = 28.0;
    CHECK_EQ(result, expect);

    /** @arg B-share (Shanghai) sell: no stamp duty, only Shanghai transfer fee */
    Stock b = makeStock("SH", "900001", STOCKTYPE_B);
    result = cost_func->getSellCost(dt, b, 10.0, 1000);
    expect = CostRecord();
    expect.commission = 18.0;
    expect.stamptax = 0.0;
    expect.transferfee = 1.0;
    expect.total = 19.0;
    CHECK_EQ(result, expect);
}

#if HKU_SUPPORT_SERIALIZATION
/** @par Test points */
TEST_CASE("test_FixedATC_export") {
    StockManager& sm = StockManager::instance();

    string filename(sm.tmpdir());
    filename += "/TC_FixedA.xml";

    TradeCostPtr func1 = TC_FixedA();
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(func1);
    }

    TradeCostPtr func2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(func2);
    }

    CHECK_EQ(func2->name(), "FixedATradeCost");

    CostRecord result, expect;
    Stock stock = sm.getStock("sh600004");
    result = func2->getBuyCost(Datetime(200101010000), stock, 10.0, 100);
    expect.commission = 5.0;
    expect.stamptax = 0.0;
    expect.transferfee = 1.0;
    expect.total = 6.0;
    CHECK_EQ(result, expect);

    result = func2->getBuyCost(Datetime(200101010000), stock, 10.0, 1000);
    expect.commission = 18.0;
    expect.stamptax = 0.0;
    expect.transferfee = 1.0;
    expect.total = 19.0;
    CHECK_EQ(result, expect);
}
#endif /* HKU_SUPPORT_SERIALIZATION */

/** @} */
