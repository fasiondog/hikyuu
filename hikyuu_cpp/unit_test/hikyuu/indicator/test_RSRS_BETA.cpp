/*
 * test_RSRS_BETA.cpp
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-07-12
 *  Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/RSRS_BETA.h>
#include <hikyuu/indicator/crt/KDATA.h>

using namespace hku;

/**
 * @defgroup test_indicator_RSRS_BETA test_indicator_RSRS_BETA
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_RSRS_BETA") {
    // 非法参数 n
    CHECK_THROWS_AS(RSRS_BETA(1), std::exception);
    CHECK_THROWS_AS(RSRS_BETA(0), std::exception);
}

TEST_CASE("test_RSRS_BETA_kdata") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-100));

    // 正常情况，n = 20
    Indicator result = RSRS_BETA(kdata, 20);
    CHECK_EQ(result.name(), "RSRS_BETA");
    CHECK_EQ(result.size(), kdata.size());
    CHECK_EQ(result.discard(), 19);

    // 验证前几个值为 nan（需要足够数据才能计算）
    for (size_t i = 0; i < result.discard(); ++i) {
        CHECK_UNARY(std::isnan(result[i]));
    }

    // 验证具体计算结果（固定值检查）
    CHECK_EQ(result[19], doctest::Approx(0.791453).epsilon(0.001));
    CHECK_EQ(result[39], doctest::Approx(0.711802).epsilon(0.001));
    CHECK_EQ(result[59], doctest::Approx(0.882348).epsilon(0.001));
    CHECK_EQ(result[79], doctest::Approx(0.824682).epsilon(0.001));
    CHECK_EQ(result[99], doctest::Approx(0.938075).epsilon(0.001));
}

TEST_CASE("test_RSRS_BETA_consistency") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-50));

    // 多次计算相同输入应得到相同结果
    Indicator result1 = RSRS_BETA(kdata, 20);
    Indicator result2 = RSRS_BETA(kdata, 20);

    CHECK_EQ(result1.size(), result2.size());
    CHECK_EQ(result1.discard(), result2.discard());
    for (size_t i = result1.discard(); i < result1.size(); ++i) {
        CHECK_EQ(result1[i], doctest::Approx(result2[i]));
    }
}

TEST_CASE("test_RSRS_BETA_different_n") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-100));

    // 不同窗口大小的测试
    Indicator result10 = RSRS_BETA(kdata, 10);
    Indicator result20 = RSRS_BETA(kdata, 20);
    Indicator result30 = RSRS_BETA(kdata, 30);

    CHECK_EQ(result10.discard(), 9);
    CHECK_EQ(result20.discard(), 19);
    CHECK_EQ(result30.discard(), 29);
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_RSRS_BETA_benchmark") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0));
    int cycle = 1000;

    {
        BENCHMARK_TIME_MSG(test_RSRS_BETA_benchmark, cycle,
                           fmt::format("data len: {}", kdata.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator result = RSRS_BETA(kdata, 20);
        }
    }
}
#endif

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_RSRS_BETA_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/RSRS_BETA.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-30));
    Indicator x1 = RSRS_BETA(kdata, 10);
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

    CHECK_EQ(x2.name(), "RSRS_BETA");
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