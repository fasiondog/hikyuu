/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-16
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/ISLASTBAR.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_ISLASTBAR test_indicator_ISLASTBAR
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_ISLASTBAR") {
    /** @arg 正常执行 */
    Indicator data = PRICELIST({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
    Indicator result = ISLASTBAR(data);
    CHECK_EQ(result.name(), "ISLASTBAR");
    CHECK_EQ(result.discard(), 0);
    for (size_t i = 0; i < 9; ++i) {
        CHECK_EQ(result[i], 0.);
    }
    CHECK_EQ(result[9], 1.);

    auto ind = ISLASTBAR();
    result = ind(data);
    CHECK_EQ(result.name(), "ISLASTBAR");
    CHECK_EQ(result.discard(), 0);
    for (size_t i = 0; i < 9; ++i) {
        CHECK_EQ(result[i], 0.);
    }
    CHECK_EQ(result[9], 1.);

    /** @arg 输入数据全部需抛弃 */
    std::vector<Indicator::value_t> src;
    for (size_t i = 0; i < 10; ++i) {
        src.push_back(Null<Indicator::value_t>());
    }
    data = PRICELIST(src, src.size());
    result = ISLASTBAR(data);
    CHECK_EQ(result.discard(), src.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }

    result = ind(data);
    CHECK_EQ(result.discard(), src.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }

    /** @arg 输入参数为 kdata */
    auto kdata = getKData("sh000001", KQuery(-10));
    CHECK_EQ(kdata.size(), 10);
    result = ISLASTBAR(kdata);
    CHECK_EQ(result.name(), "ISLASTBAR");
    CHECK_EQ(result.discard(), 0);
    for (size_t i = 0; i < 9; ++i) {
        CHECK_EQ(result[i], 0.);
    }
    CHECK_EQ(result[9], 1.);

    result = ind(kdata);
    CHECK_EQ(result.name(), "ISLASTBAR");
    CHECK_EQ(result.discard(), 0);
    for (size_t i = 0; i < 9; ++i) {
        CHECK_EQ(result[i], 0.);
    }
    CHECK_EQ(result[9], 1.);
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_ISLASTBAR_benchmark") {
    // Stock stock = getStock("sh000001");
    // KData kdata = stock.getKData(KQuery(0));
    // Indicator c = kdata.close();
    // int cycle = 1000;  // 测试循环次数

    // {
    //     BENCHMARK_TIME_MSG(test_ABS_benchmark, cycle, fmt::format("data len: {}", c.size()));
    //     SPEND_TIME_CONTROL(false);
    //     for (int i = 0; i < cycle; i++) {
    //         Indicator ind = ABS();
    //         Indicator result = ind(c);
    //     }
    // }
}
#endif

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_ISLASTBAR_export") {
    // StockManager& sm = StockManager::instance();
    // string filename(sm.tmpdir());
    // filename += "/ISLASTBAR.xml";

    // Stock stock = sm.getStock("sh000001");
    // KData kdata = stock.getKData(KQuery(-20));
    // Indicator x1 = ABS(CLOSE(kdata));
    // {
    //     std::ofstream ofs(filename);
    //     boost::archive::xml_oarchive oa(ofs);
    //     oa << BOOST_SERIALIZATION_NVP(x1);
    // }

    // Indicator x2;
    // {
    //     std::ifstream ifs(filename);
    //     boost::archive::xml_iarchive ia(ifs);
    //     ia >> BOOST_SERIALIZATION_NVP(x2);
    // }

    // CHECK_UNARY(x1.size() == x2.size());
    // CHECK_UNARY(x1.discard() == x2.discard());
    // CHECK_UNARY(x1.getResultNumber() == x2.getResultNumber());
    // for (size_t i = 0; i < x1.size(); ++i) {
    //     CHECK_EQ(x1[i], doctest::Approx(x2[i]).epsilon(0.00001));
    // }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
