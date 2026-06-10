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

/** @par 检测点：测试空输入 */
TEST_CASE("test_multi_regression_empty") {
    Stock stock = getStock("sh000001");

    // 空指标列表
    auto result = multi_regression(stock, KQuery(-20), IndicatorList{});
    CHECK(result.empty());

    // 无效查询
    auto result2 =
      multi_regression(stock, KQuery(Datetime(2099, 1, 1), Datetime(2099, 1, 2)), MA(CLOSE(), 5));
    CHECK(result2.empty());
}

/** @par 检测点：测试使用固定数据的一元线性回归 */
TEST_CASE("test_multi_regression_single_factor_fixed") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-100));

    if (!kdata.empty()) {
        Indicator ind = MA(CLOSE(), 5);
        auto result = multi_regression(stock, KQuery(-100), ind);

        CHECK_EQ(result.size(), 2);

        // 固定值验证 - 根据实际运行结果设置
        CHECK(result[0] == doctest::Approx(0.035479989606263).epsilon(1e-6));
        CHECK(result[1] == doctest::Approx(-0.000014864499944).epsilon(1e-6));
    }
}

/** @par 检测点：测试多因子回归 */
TEST_CASE("test_multi_regression_multi_factor") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-200));

    if (!kdata.empty()) {
        Indicator ind1 = MA(CLOSE(), 5);
        Indicator macd = MACD(CLOSE());
        Indicator ind2 = RESULT(macd, 0);
        Indicator ind3 = RESULT(macd, 1);

        // 使用模板版本
        auto result = multi_regression(stock, KQuery(-200), ind1, ind2, ind3);

        // 应该返回 alpha 和三个 beta
        CHECK_EQ(result.size(), 4);

        // 固定值验证 - 根据实际运行结果设置
        CHECK(result[0] == doctest::Approx(-0.007766162437654).epsilon(1e-6));
        CHECK(result[1] == doctest::Approx(0.000002228680454).epsilon(1e-6));
        CHECK(result[2] == doctest::Approx(0.000262073918492).epsilon(1e-6));
        CHECK(result[3] == doctest::Approx(-0.000043186383439).epsilon(1e-6));
    }
}

/** @par 检测点：测试使用 IndicatorList 版本 */
TEST_CASE("test_multi_regression_indicator_list") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-150));

    if (!kdata.empty()) {
        IndicatorList inds = {MA(CLOSE(), 5), MA(CLOSE(), 10), MA(CLOSE(), 20)};
        auto result = multi_regression(stock, KQuery(-150), inds);

        // 应该返回 alpha 和三个 beta
        CHECK_EQ(result.size(), 4);

        // 固定值验证 - 根据实际运行结果设置
        CHECK(result[0] == doctest::Approx(0.006943948947997).epsilon(1e-6));
        CHECK(result[1] == doctest::Approx(-0.000033354626070).epsilon(1e-6));
        CHECK(result[2] == doctest::Approx(0.000061308868555).epsilon(1e-6));
        CHECK(result[3] == doctest::Approx(-0.000031001376132).epsilon(1e-6));
    }
}

/** @par 检测点：测试数据不足的情况 */
TEST_CASE("test_multi_regression_insufficient_data") {
    Stock stock = getStock("sh000001");

    // 使用非常小的窗口，数据点不足
    auto result =
      multi_regression(stock, KQuery(-5), MA(CLOSE(), 5), MA(CLOSE(), 10), MA(CLOSE(), 20));
    CHECK(result.empty());
}

/** @par 检测点：测试模板版本的可变参数 */
TEST_CASE("test_multi_regression_variadic_template") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-252));

    if (!kdata.empty()) {
        // 测试1个因子
        auto result1 = multi_regression(stock, KQuery(-252), MA(CLOSE(), 5));
        CHECK_EQ(result1.size(), 2);
        CHECK(result1[0] == doctest::Approx(-0.001537620312836).epsilon(1e-6));
        CHECK(result1[1] == doctest::Approx(0.000000303218095).epsilon(1e-6));

        // 测试2个因子
        auto result2 = multi_regression(stock, KQuery(-252), MA(CLOSE(), 5), MA(CLOSE(), 10));
        CHECK_EQ(result2.size(), 3);
        CHECK(result2[0] == doctest::Approx(0.000457517335535).epsilon(1e-6));
        CHECK(result2[1] == doctest::Approx(0.000016452132076).epsilon(1e-6));
        CHECK(result2[2] == doctest::Approx(-0.000016871762754).epsilon(1e-6));

        // 测试3个因子
        auto result3 =
          multi_regression(stock, KQuery(-252), MA(CLOSE(), 5), MA(CLOSE(), 10), MA(CLOSE(), 20));
        CHECK_EQ(result3.size(), 4);
        CHECK(result3[0] == doctest::Approx(0.001552110753821).epsilon(1e-6));
        CHECK(result3[1] == doctest::Approx(0.000012868928538).epsilon(1e-6));
        CHECK(result3[2] == doctest::Approx(-0.000009341380416).epsilon(1e-6));
        CHECK(result3[3] == doctest::Approx(-0.000004368132896).epsilon(1e-6));
    }
}

/** @par 检测点：测试完整版本 multi_regression_full */
TEST_CASE("test_multi_regression_full") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-100));

    if (!kdata.empty()) {
        Indicator ind = MA(CLOSE(), 5);
        auto result = multi_regression_full(stock, KQuery(-100), ind);

        // 结果应该包含：alpha, beta, residuals, RSS, R²
        // 假设有 n 个有效数据点，则结果大小为 2 + n + 2 = n + 4
        CHECK_GT(result.size(), 4);

        // 固定值验证 - 根据实际运行结果设置
        CHECK(result[0] == doctest::Approx(0.035479989606263).epsilon(1e-6));   // alpha
        CHECK(result[1] == doctest::Approx(-0.000014864499944).epsilon(1e-6));  // beta

        size_t rss_pos = result.size() - 2;
        size_t r2_pos = result.size() - 1;
        CHECK(result[rss_pos] == doctest::Approx(0.015468753348483).epsilon(1e-6));  // RSS
        CHECK(result[r2_pos] == doctest::Approx(0.019762192080754).epsilon(1e-6));   // R²

        // 验证 R² 在合理范围内 [0, 1]
        CHECK(result[r2_pos] >= 0.0);
        CHECK(result[r2_pos] <= 1.0);

        // 验证 RSS 非负
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