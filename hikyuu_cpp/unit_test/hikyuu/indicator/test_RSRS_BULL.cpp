/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-07-12
 *  Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/RSRS_BULL.h>
#include <hikyuu/indicator/crt/KDATA.h>

using namespace hku;
using value_t = Indicator::value_t;

TEST_CASE("test_RSRS_BULL") {
    // Test the parameter validation
    CHECK_THROWS_AS(RSRS_BULL(1, 750), std::exception);  // n < 2
    CHECK_THROWS_AS(RSRS_BULL(20, 1), std::exception);   // m < 2
}

TEST_CASE("test_RSRS_BULL_kdata") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-1000));

    // The normal case, n = 20 and m = 750 (given explicitly to test a specific window size)
    Indicator result = RSRS_BULL(kdata, 20, 750);
    CHECK_EQ(result.name(), "RSRS_BULL");
    CHECK_EQ(result.size(), kdata.size());
    CHECK_EQ(result.getResultNumber(), 4);

    // discard = n-1 + m-1 = 19 + 749 = 768 (the ADX style, all the result sets share one discard)
    CHECK_EQ(result.discard(), 768);

    // Verify that all the result sets are nan in the first discard positions
    for (size_t i = 0; i < result.discard(); ++i) {
        CHECK_UNARY(std::isnan(result.get(i, 0)));  // The level 4 correction value
        CHECK_UNARY(std::isnan(result.get(i, 1)));  // The beta value
        CHECK_UNARY(std::isnan(result.get(i, 2)));  // The R2 value
        CHECK_UNARY(std::isnan(result.get(i, 3)));  // The Z value
    }

    // Verify that the following values are not nan
    for (size_t i = result.discard(); i < result.size(); ++i) {
        CHECK_UNARY(!std::isnan(result.get(i, 0)));  // The level 4 correction value
        CHECK_UNARY(!std::isnan(result.get(i, 1)));  // The beta value
        CHECK_UNARY(!std::isnan(result.get(i, 2)));  // The R2 value
        CHECK_UNARY(!std::isnan(result.get(i, 3)));  // The Z value
    }

    // Verify that the R2 value is within 0~1
    for (size_t i = result.discard(); i < result.size(); ++i) {
        value_t r2 = result.get(i, 2);
        CHECK_UNARY(r2 >= 0.0 && r2 <= 1.0);
    }
}

TEST_CASE("test_RSRS_BULL_consistency") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-800));

    // Calculating the same input several times should give the same result
    Indicator result1 = RSRS_BULL(kdata, 20, 750);
    Indicator result2 = RSRS_BULL(kdata, 20, 750);

    CHECK_EQ(result1.size(), result2.size());
    CHECK_EQ(result1.discard(), result2.discard());

    for (size_t i = result1.discard(); i < result1.size(); ++i) {
        for (int j = 0; j < 4; ++j) {
            value_t v1 = result1.get(i, j);
            value_t v2 = result2.get(i, j);
            if (std::isnan(v1)) {
                CHECK_UNARY(std::isnan(v2));
            } else {
                CHECK_EQ(v1, doctest::Approx(v2));
            }
        }
    }
}

TEST_CASE("test_RSRS_BULL_different_params") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-500));

    // Test the different window sizes
    Indicator result1 = RSRS_BULL(kdata, 10, 100);
    Indicator result2 = RSRS_BULL(kdata, 30, 200);

    CHECK_EQ(result1.name(), "RSRS_BULL");
    CHECK_EQ(result2.name(), "RSRS_BULL");
    CHECK_EQ(result1.getResultNumber(), 4);
    CHECK_EQ(result2.getResultNumber(), 4);

    // Different parameters should have different discard values
    CHECK_EQ(result1.discard(), 9 + 99);    // n-1 + m-1 = 9 + 99 = 108
    CHECK_EQ(result2.discard(), 29 + 199);  // n-1 + m-1 = 29 + 199 = 228
}

#if HKU_SUPPORT_SERIALIZATION
TEST_CASE("test_RSRS_BULL_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/RSRS_BULL.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-300));
    Indicator x1 = RSRS_BULL(kdata, 10, 50);
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

    CHECK_EQ(x2.name(), "RSRS_BULL");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = x1.discard(); i < x1.size(); ++i) {
        for (int j = 0; j < 4; ++j) {
            value_t v1 = x1.get(i, j);
            value_t v2 = x2.get(i, j);
            if (std::isnan(v1)) {
                CHECK_UNARY(std::isnan(v2));
            } else {
                CHECK_EQ(v1, doctest::Approx(v2));
            }
        }
    }
}
#endif