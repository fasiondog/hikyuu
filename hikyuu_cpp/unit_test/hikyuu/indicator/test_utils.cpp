/*
 * test_utils.cpp
 *
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-05-28
 *  Author: fasiondog
 */

#include "../test_config.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/utils.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/MACD.h>
#include <hikyuu/indicator/crt/RESULT.h>

using namespace hku;

/**
 * @defgroup test_indicator_utils test_indicator_utils
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test point: test the empty input */
TEST_CASE("test_multi_regression_empty") {
    Stock stock = getStock("sh000001");

    // An empty indicator list
    auto result = multi_regression(stock, KQuery(-20), IndicatorList{});
    CHECK(result.empty());

    // An invalid query
    auto result2 =
      multi_regression(stock, KQuery(Datetime(2099, 1, 1), Datetime(2099, 1, 2)), MA(CLOSE(), 5));
    CHECK(result2.empty());
}

/** @par Test point: test the unary linear regression with fixed data */
TEST_CASE("test_multi_regression_single_factor_fixed") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-100));

    if (!kdata.empty()) {
        Indicator ind = MA(CLOSE(), 5);
        auto result = multi_regression(stock, KQuery(-100), ind);

        CHECK_EQ(result.size(), 2);

        // The fixed value verification - set according to the actual run result
        CHECK(result[0] == doctest::Approx(0.035479989606263).epsilon(1e-6));
        CHECK(result[1] == doctest::Approx(-0.000014864499944).epsilon(1e-6));
    }
}

/** @par Test point: test the multiple factor regression */
TEST_CASE("test_multi_regression_multi_factor") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-200));

    if (!kdata.empty()) {
        Indicator ind1 = MA(CLOSE(), 5);
        Indicator macd = MACD(CLOSE());
        Indicator ind2 = RESULT(macd, 0);
        Indicator ind3 = RESULT(macd, 1);

        // Use the template version
        auto result = multi_regression(stock, KQuery(-200), ind1, ind2, ind3);

        // It should return alpha and three betas
        CHECK_EQ(result.size(), 4);

        // The fixed value verification - set according to the actual run result
        CHECK(result[0] == doctest::Approx(-0.007766162437654).epsilon(1e-6));
        CHECK(result[1] == doctest::Approx(0.000002228680454).epsilon(1e-6));
        CHECK(result[2] == doctest::Approx(0.000262073918492).epsilon(1e-6));
        CHECK(result[3] == doctest::Approx(-0.000043186383439).epsilon(1e-6));
    }
}

/** @par Test point: test the IndicatorList version */
TEST_CASE("test_multi_regression_indicator_list") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-150));

    if (!kdata.empty()) {
        IndicatorList inds = {MA(CLOSE(), 5), MA(CLOSE(), 10), MA(CLOSE(), 20)};
        auto result = multi_regression(stock, KQuery(-150), inds);

        // It should return alpha and three betas
        CHECK_EQ(result.size(), 4);

        // The fixed value verification - set according to the actual run result
        CHECK(result[0] == doctest::Approx(0.006943948947997).epsilon(1e-6));
        CHECK(result[1] == doctest::Approx(-0.000033354626070).epsilon(1e-6));
        CHECK(result[2] == doctest::Approx(0.000061308868555).epsilon(1e-6));
        CHECK(result[3] == doctest::Approx(-0.000031001376132).epsilon(1e-6));
    }
}

/** @par Test point: test the case with insufficient data */
TEST_CASE("test_multi_regression_insufficient_data") {
    Stock stock = getStock("sh000001");

    // Use a very small window, the data points are insufficient
    auto result =
      multi_regression(stock, KQuery(-5), MA(CLOSE(), 5), MA(CLOSE(), 10), MA(CLOSE(), 20));
    CHECK(result.empty());
}

/** @par Test point: test the variadic parameters of the template version */
TEST_CASE("test_multi_regression_variadic_template") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-252));

    if (!kdata.empty()) {
        // Test with 1 factor
        auto result1 = multi_regression(stock, KQuery(-252), MA(CLOSE(), 5));
        CHECK_EQ(result1.size(), 2);
        CHECK(result1[0] == doctest::Approx(-0.001537620312836).epsilon(1e-6));
        CHECK(result1[1] == doctest::Approx(0.000000303218095).epsilon(1e-6));

        // Test with 2 factors
        auto result2 = multi_regression(stock, KQuery(-252), MA(CLOSE(), 5), MA(CLOSE(), 10));
        CHECK_EQ(result2.size(), 3);
        CHECK(result2[0] == doctest::Approx(0.000457517335535).epsilon(1e-6));
        CHECK(result2[1] == doctest::Approx(0.000016452132076).epsilon(1e-6));
        CHECK(result2[2] == doctest::Approx(-0.000016871762754).epsilon(1e-6));

        // Test with 3 factors
        auto result3 =
          multi_regression(stock, KQuery(-252), MA(CLOSE(), 5), MA(CLOSE(), 10), MA(CLOSE(), 20));
        CHECK_EQ(result3.size(), 4);
        CHECK(result3[0] == doctest::Approx(0.001552110753821).epsilon(1e-6));
        CHECK(result3[1] == doctest::Approx(0.000012868928538).epsilon(1e-6));
        CHECK(result3[2] == doctest::Approx(-0.000009341380416).epsilon(1e-6));
        CHECK(result3[3] == doctest::Approx(-0.000004368132896).epsilon(1e-6));
    }
}

/** @par Test point: test the complete version multi_regression_full */
TEST_CASE("test_multi_regression_full") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-100));

    if (!kdata.empty()) {
        Indicator ind = MA(CLOSE(), 5);
        auto result = multi_regression_full(stock, KQuery(-100), ind);

        // The result should contain: alpha, beta, residuals, RSS, R2
        // Assuming n valid data points, the result size is 2 + n + 2 = n + 4
        CHECK_GT(result.size(), 4);

        // The fixed value verification - set according to the actual run result
        CHECK(result[0] == doctest::Approx(0.035479989606263).epsilon(1e-6));   // alpha
        CHECK(result[1] == doctest::Approx(-0.000014864499944).epsilon(1e-6));  // beta

        size_t rss_pos = result.size() - 2;
        size_t r2_pos = result.size() - 1;
        CHECK(result[rss_pos] == doctest::Approx(0.015468753348483).epsilon(1e-6));  // RSS
        CHECK(result[r2_pos] == doctest::Approx(0.019762192080754).epsilon(1e-6));   // R²

        // Verify that R2 is within a reasonable range [0, 1]
        CHECK(result[r2_pos] >= 0.0);
        CHECK(result[r2_pos] <= 1.0);

        // Verify that RSS is non-negative
        CHECK(result[rss_pos] >= 0.0);
    }
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_multi_regression_benchmark") {
    Stock stock = getStock("sh000001");
    int cycle = 100;

    {
        BENCHMARK_TIME_MSG(test_multi_regression_benchmark, cycle, "multi_regression benchmark");
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            auto result = multi_regression(stock, KQuery(-252), MA(CLOSE(), 5), MA(CLOSE(), 10));
        }
    }
}
#endif

/** @} */