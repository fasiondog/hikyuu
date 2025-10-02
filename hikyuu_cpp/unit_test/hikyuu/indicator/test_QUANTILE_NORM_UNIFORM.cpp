/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-03
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/QUANTILE_NORM_UNIFORM.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_QUANTILE_NORM_UNIFORM test_indicator_QUANTILE_NORM_UNIFORM
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_QUANTILE_NORM_UNIFORM") {
    /** @arg 只有一条有效数据 */
    PriceList a{0.3};
    Indicator data = PRICELIST(a);
    REQUIRE(data.size() == 1);
    Indicator result = QUANTILE_NORM_UNIFORM(data);
    CHECK_EQ(result.name(), "QUANTILE_NORM_UNIFORM");
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.discard(), 1);
    CHECK_UNARY(std::isnan(result[0]));

    // /** @arg 输入的 quantile 非法 */
    KData k = getKData("SH000001", KQuery(-100));
    CHECK_THROWS_AS(QUANTILE_NORM_UNIFORM(k.close(), 0.0, 0.99), std::exception);
    CHECK_THROWS_AS(QUANTILE_NORM_UNIFORM(k.close(), 0.01, 1.0), std::exception);
    CHECK_THROWS_AS(QUANTILE_NORM_UNIFORM(k.close(), 0.9, 0.1), std::exception);

    /** @arg 正常计算 */
    result = QUANTILE_NORM_UNIFORM(k.close());
    CHECK_EQ(result.name(), "QUANTILE_NORM_UNIFORM");
    CHECK_UNARY(!result.empty());
    CHECK_EQ(result.size(), k.size());
    CHECK_EQ(result.discard(), 0);
    Indicator close = k.close();

    for (size_t i = 0; i < result.size(); i++) {
        // HKU_INFO("result[{}]: {}, expect: {}", i, result[i], close[i]);
        if (result[i] >= 2529.1328) {
            HKU_INFO("max i: {}", i);
        }
        if (result[i] <= 2317.275) {
            HKU_INFO("min i: {}", i);
        }
    }

    // PriceList expect{-0.573824, 1.52671, 0.497154, -0.581095, -0.868942};
    // for (size_t i = result.discard(), total = result.size(); i < total; i++) {
    //     CHECK_EQ(result[i], doctest::Approx(expect[i]));
    // }

    // /** @arg 过滤异常值，不递归*/
    // k = getKData("SH000001", KQuery(3600, 4000));
    // Indicator c = k.close();
    // result = QUANTILE_NORM_UNIFORM(c, true, 3.0, false);
    // expect = {-0.87128, -0.90351, -0.87397, -0.87383};
    // for (size_t i = 0; i < expect.size(); i++) {
    //     CHECK_EQ(result[i], doctest::Approx(expect[i]));
    // }

    // /** @arg 过滤异常值，递归*/
    // k = getKData("SH000001", KQuery(3600, 4000));
    // c = k.close();
    // auto result2 = QUANTILE_NORM_UNIFORM(c, true, 3.0, true);
    // expect = {-0.87128, -0.90354, -0.87399, -0.87383};
    // for (size_t i = 0; i < expect.size(); i++) {
    //     CHECK_EQ(result2[i], doctest::Approx(expect[i]));
    // }
    // CHECK_EQ(result[16], doctest::Approx(-0.95994));
    // CHECK_EQ(result2[16], doctest::Approx(-0.95996));
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_QUANTILE_NORM_UNIFORM_benchmark") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0));
    Indicator c = kdata.close();
    int cycle = 1000;  // 测试循环次数

    {
        BENCHMARK_TIME_MSG(test_QUANTILE_NORM_UNIFORM_benchmark, cycle,
                           fmt::format("data len: {}", c.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator ind = QUANTILE_NORM_UNIFORM();
            Indicator result = ind(c);
        }
    }
}
#endif

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_QUANTILE_NORM_UNIFORM_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/QUANTILE_NORM_UNIFORM.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = QUANTILE_NORM_UNIFORM(CLOSE(kdata));
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

    CHECK_EQ(x1.name(), x2.name());
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
