/*
 * test_SKEW.cpp
 *
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/SKEW.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_SKEW test_indicator_SKEW
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_SKEW") {
    Indicator result;

    // An empty indicator
    result = SKEW(Indicator(), 10);
    CHECK_UNARY(result.empty());

    // The test data: a perfectly symmetric distribution (an even number of points), the skewness
    // should be 0
    PriceList symmetric{1.0, 2.0, 3.0, 4.0, 4.0, 3.0, 2.0, 1.0};
    Indicator x = PRICELIST(symmetric);

    // The invalid parameter n
    CHECK_THROWS_AS(SKEW(x, -1), std::exception);
    CHECK_THROWS_AS(SKEW(x, 1), std::exception);
    CHECK_THROWS_AS(SKEW(x, 2), std::exception);

    // The normal case, n = 0 (using all the data)
    result = SKEW(x, 0);
    CHECK_EQ(result.name(), "SKEW");
    CHECK_EQ(result.discard(), symmetric.size() - 1);
    CHECK_EQ(result.size(), symmetric.size());

    CHECK_UNARY(std::isnan(result[0]));
    CHECK_UNARY(std::isnan(result[1]));

    // The skewness of a symmetric distribution should be close to 0
    CHECK_EQ(result[result.discard()], doctest::Approx(0.0).epsilon(0.001));

    // The test data: a right skewed distribution (positive skew) - a long tail on the right
    PriceList right_skewed{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 100.0};
    Indicator x_right = PRICELIST(right_skewed);

    result = SKEW(x_right, 0);
    CHECK_EQ(result.name(), "SKEW");
    // The skewness of a right skewed distribution should be positive
    CHECK_GT(result[result.discard()], 0.0);

    // The test data: a left skewed distribution (negative skew) - a short tail on the left and the
    // large values on the right
    PriceList left_skewed{1.0, 92.0, 93.0, 94.0, 95.0, 96.0, 97.0, 98.0, 99.0, 100.0};
    Indicator x_left = PRICELIST(left_skewed);

    result = SKEW(x_left, 0);
    CHECK_EQ(result.name(), "SKEW");
    // The skewness of a left skewed distribution should be negative
    CHECK_LT(result[result.discard()], 0.0);

    // Test the fixed window n = 5
    PriceList data{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    Indicator x_data = PRICELIST(data);

    result = SKEW(x_data, 5);
    CHECK_EQ(result.name(), "SKEW");
    CHECK_EQ(result.discard(), 4);
    CHECK_EQ(result.size(), data.size());

    // An arithmetic sequence is symmetric, so the skewness should be close to 0
    for (size_t i = result.discard(); i < result.size(); ++i) {
        CHECK_EQ(result[i], doctest::Approx(0.0).epsilon(0.001));
    }

    // Test the functional call style SKEW(n)(ind)
    result = SKEW(5)(x_data);
    CHECK_EQ(result.name(), "SKEW");
    CHECK_EQ(result.discard(), 4);
    CHECK_EQ(result.size(), data.size());

    // Verify that the functional call matches the direct call
    for (size_t i = result.discard(); i < result.size(); ++i) {
        CHECK_EQ(result[i], doctest::Approx(0.0).epsilon(0.001));
    }
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_SKEW_benchmark") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0));
    Indicator c = kdata.close();
    int cycle = 1000;  // Test loop count

    {
        BENCHMARK_TIME_MSG(test_SKEW_benchmark, cycle, fmt::format("data len: {}", c.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator result = SKEW(c, 200);
        }
    }
}
#endif

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_SKEW_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/SKEW.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = SKEW(CLOSE(kdata), 10);
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

    CHECK_EQ(x2.name(), "SKEW");
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