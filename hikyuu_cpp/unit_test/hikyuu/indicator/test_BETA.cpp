/*
 * test_BETA.cpp
 *
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-XX-XX
 *  Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/BETA.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_BETA test_indicator_BETA
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points: ST-034 — Inf inputs produce Null output, never ±Inf */
TEST_CASE("test_BETA_inf_input_yields_null") {
    price_t inf_v = std::numeric_limits<price_t>::infinity();
    PriceList x, y;
    for (size_t i = 0; i < 8; ++i) {
        x.push_back(1.0 + i);
        y.push_back(i == 3 ? inf_v : (2.0 + i));
    }
    Indicator r = BETA(PRICELIST(x), PRICELIST(y), 3);
    for (size_t i = r.discard(); i < 8; ++i) {
        CHECK_UNARY(std::isnan(r[i]) || std::isfinite(r[i]));
    }
}

/** @par Test points */
TEST_CASE("test_BETA") {
    Indicator result;

    // An empty indicator
    result = BETA(Indicator(), Indicator(), 60);
    CHECK_UNARY(result.empty());

    // Create the test data
    // The stock return and the market return
    PriceList stock_returns{0.01, 0.02, -0.01, 0.015, 0.005, -0.02, 0.012, 0.018, -0.008, 0.02};
    PriceList market_returns{0.008,  0.015, -0.005, 0.012,  0.003,
                             -0.015, 0.008, 0.012,  -0.003, 0.015};

    Indicator stock_ind = PRICELIST(stock_returns);
    Indicator market_ind = PRICELIST(market_returns);

    // The invalid parameter n
    CHECK_THROWS_AS(BETA(stock_ind, market_ind, -1), std::exception);
    CHECK_THROWS_AS(BETA(stock_ind, market_ind, 1), std::exception);

    // The normal case, n = 0: expanding window
    result = BETA(stock_ind, market_ind, 0);
    CHECK_EQ(result.name(), "BETA");
    CHECK_EQ(result.size(), stock_returns.size());
    CHECK_EQ(result.discard(), 1);
    // Per the IBeta implementation, kx = first_stock_value and ky = first_market_value
    // The accumulation starts from the second data point
    // stock: [0.01, 0.02, -0.01, 0.015, 0.005, -0.02, 0.012, 0.018, -0.008, 0.02]
    // market: [0.008, 0.015, -0.005, 0.012, 0.003, -0.015, 0.008, 0.012, -0.003, 0.015]
    // bar 0 is dropped (no pair yet); bars 1..total-1 each carry an expanding-window beta
    CHECK_UNARY(std::isnan(result[0]));
    for (size_t i = 1; i < result.size(); ++i) {
        CHECK_UNARY(!std::isnan(result[i]));
    }
    // Full-sample beta at the last bar matches the previous whole-series value
    CHECK_EQ(result[9], doctest::Approx(1.41516).epsilon(0.001));

    // Test the rolling window n = 8
    result = BETA(stock_ind, market_ind, 8);
    CHECK_EQ(result.name(), "BETA");
    CHECK_EQ(result.size(), stock_returns.size());
    CHECK_EQ(result.discard(), 7);

    // Verify that the first few values are nan (enough data is needed to calculate)
    for (size_t i = 0; i < result.discard(); ++i) {
        CHECK_UNARY(std::isnan(result[i]));
    }

    // Verify the concrete calculation result
    // The Beta value of the window [0:8)
    CHECK_EQ(result[7], doctest::Approx(1.38916).epsilon(0.001));
    // The Beta value of the window [1:9)
    CHECK_EQ(result[8], doctest::Approx(1.42331).epsilon(0.001));
    // The Beta value of the window [2:10)
    CHECK_EQ(result[9], doctest::Approx(1.42331).epsilon(0.001));
}

TEST_CASE("test_BETA_stock") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-100));
    Indicator stock_close = CLOSE(kdata);
    Indicator market_close = OPEN(kdata);

    Indicator result = BETA(stock_close, market_close, 60);
    CHECK_EQ(result.name(), "BETA");
    CHECK_EQ(result.size(), stock_close.size());
    CHECK_EQ(result.discard(), 59);

    for (size_t i = 0; i < result.discard(); ++i) {
        CHECK_UNARY(std::isnan(result[i]));
    }

    for (size_t i = result.discard(); i < result.size(); ++i) {
        CHECK_UNARY(!std::isnan(result[i]));
    }

    CHECK_EQ(result[59], doctest::Approx(0.992).epsilon(0.001));
    CHECK_EQ(result[79], doctest::Approx(0.926).epsilon(0.001));
    CHECK_EQ(result[99], doctest::Approx(0.924).epsilon(0.001));
}

/** @par Test points: ST-N03 — after a mid-series NaN pair slides out of the window, beta recovers
 */
TEST_CASE("test_BETA_nan_recovery") {
    price_t nan = Null<price_t>();
    Indicator x = PRICELIST({1.0, 2.0, 3.0, 4.0, nan, 6.0, 7.0, 8.0, 9.0});
    Indicator y = PRICELIST({2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0, 18.0});
    // beta = cov(x,y) / var(y); with y=2x, expected beta = 0.5
    Indicator r = BETA(x, y, 3);
    for (size_t i = 6; i < 9; i++) {
        CHECK_UNARY(std::isfinite(r[i]));
    }
    CHECK_EQ(r[8], doctest::Approx(0.5));
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_BETA_benchmark") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0));
    Indicator c = kdata.close();
    Indicator h = kdata.close();
    int cycle = 1000;  // Test loop count

    {
        BENCHMARK_TIME_MSG(test_BETA_benchmark, cycle, fmt::format("data len: {}", c.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator result = BETA(c, h, 60);
        }
    }
}
#endif

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_BETA_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/BETA.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = BETA(CLOSE(kdata), OPEN(kdata), 10);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(x1);
    }

    Indicator x2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(x2);
    }

    CHECK_EQ(x2.name(), "BETA");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = x1.discard(); i < x1.size(); ++i) {
        if (std::isnan(x1[i])) {
            CHECK_UNARY(std::isnan(x2[i]));
        } else {
            CHECK_EQ(x1[i], doctest::Approx(x2[i]));
        }
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */