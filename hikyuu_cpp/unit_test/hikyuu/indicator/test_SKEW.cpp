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

/** @par 检测点 */
TEST_CASE("test_SKEW") {
    Indicator result;

    // 空指标
    result = SKEW(Indicator(), 10);
    CHECK_UNARY(result.empty());

    // 测试数据：完美对称分布（偶数个点），偏度应为0
    PriceList symmetric{1.0, 2.0, 3.0, 4.0, 4.0, 3.0, 2.0, 1.0};
    Indicator x = PRICELIST(symmetric);

    // 非法参数 n
    CHECK_THROWS_AS(SKEW(x, -1), std::exception);
    CHECK_THROWS_AS(SKEW(x, 1), std::exception);
    CHECK_THROWS_AS(SKEW(x, 2), std::exception);

    // 正常情况，n = 0（使用全部数据）
    result = SKEW(x, 0);
    CHECK_EQ(result.name(), "SKEW");
    CHECK_EQ(result.discard(), symmetric.size() - 1);
    CHECK_EQ(result.size(), symmetric.size());

    price_t nan = Null<price_t>();
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_UNARY(std::isnan(result[1]));

    // 对称分布的偏度应接近0
    CHECK_EQ(result[result.discard()], doctest::Approx(0.0).epsilon(0.001));

    // 测试数据：右偏分布（正偏）- 大尾巴在右边
    PriceList right_skewed{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 100.0};
    Indicator x_right = PRICELIST(right_skewed);

    result = SKEW(x_right, 0);
    CHECK_EQ(result.name(), "SKEW");
    // 右偏分布的偏度应为正值
    CHECK_GT(result[result.discard()], 0.0);

    // 测试数据：左偏分布（负偏）- 小尾巴在左边，大值集中在右边
    PriceList left_skewed{1.0, 92.0, 93.0, 94.0, 95.0, 96.0, 97.0, 98.0, 99.0, 100.0};
    Indicator x_left = PRICELIST(left_skewed);

    result = SKEW(x_left, 0);
    CHECK_EQ(result.name(), "SKEW");
    // 左偏分布的偏度应为负值
    CHECK_LT(result[result.discard()], 0.0);

    // 测试固定窗口 n = 5
    PriceList data{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    Indicator x_data = PRICELIST(data);

    result = SKEW(x_data, 5);
    CHECK_EQ(result.name(), "SKEW");
    CHECK_EQ(result.discard(), 4);
    CHECK_EQ(result.size(), data.size());

    // 等差数列是对称分布，偏度应接近0
    for (size_t i = result.discard(); i < result.size(); ++i) {
        CHECK_EQ(result[i], doctest::Approx(0.0).epsilon(0.001));
    }

    // 测试函数式调用方式 SKEW(n)(ind)
    result = SKEW(5)(x_data);
    CHECK_EQ(result.name(), "SKEW");
    CHECK_EQ(result.discard(), 4);
    CHECK_EQ(result.size(), data.size());

    // 验证函数式调用结果与直接调用一致
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
    int cycle = 1000;  // 测试循环次数

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

/** @par 检测点 */
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