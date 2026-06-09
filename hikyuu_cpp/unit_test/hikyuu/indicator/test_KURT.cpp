/*
 * test_KURT.cpp
 *
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/KURT.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_KURT test_indicator_KURT
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_KURT") {
    Indicator result;

    // 空指标
    result = KURT(Indicator(), 10);
    CHECK_UNARY(result.empty());

    // 测试数据：等差数列（均匀分布）
    PriceList uniform{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    Indicator x = PRICELIST(uniform);

    // 非法参数 n
    CHECK_THROWS_AS(KURT(x, -1), std::exception);
    CHECK_THROWS_AS(KURT(x, 1), std::exception);
    CHECK_THROWS_AS(KURT(x, 2), std::exception);
    CHECK_THROWS_AS(KURT(x, 3), std::exception);

    // 正常情况，n = 0（使用全部数据）
    result = KURT(x, 0);
    CHECK_EQ(result.name(), "KURT");
    CHECK_EQ(result.discard(), uniform.size() - 1);
    CHECK_EQ(result.size(), uniform.size());

    price_t nan = Null<price_t>();
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_UNARY(std::isnan(result[1]));

    // 均匀分布的超额峰度应接近 -1.2（有限样本会有偏差）
    CHECK_EQ(result[result.discard()], doctest::Approx(-1.2).epsilon(0.1));

    // 测试数据：相同值序列
    // 所有值相同时，超额峰度为 -3.0
    PriceList same_values{5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0};
    Indicator x_same = PRICELIST(same_values);
    result = KURT(x_same, 0);
    CHECK_EQ(result.name(), "KURT");
    CHECK_EQ(result[result.discard()], -3.0);

    // 测试数据：高度集中的尖峰分布
    PriceList leptokurtic{1.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 10.0};
    Indicator x_lepto = PRICELIST(leptokurtic);
    result = KURT(x_lepto, 0);
    CHECK_EQ(result.name(), "KURT");
    // 高度集中的分布应有正的超额峰度
    CHECK_GT(result[result.discard()], 0.0);

    // 测试固定窗口 n = 5
    PriceList data{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
    Indicator x_data = PRICELIST(data);

    result = KURT(x_data, 5);
    CHECK_EQ(result.name(), "KURT");
    CHECK_EQ(result.discard(), 4);
    CHECK_EQ(result.size(), data.size());

    // 均匀分布的超额峰度应在 -1.5 到 -1.0 之间
    for (size_t i = result.discard(); i < result.size(); ++i) {
        CHECK(result[i] > -1.5);
        CHECK(result[i] < -1.0);
    }

    // 测试函数式调用方式 KURT(n)(ind)
    result = KURT(5)(x_data);
    CHECK_EQ(result.name(), "KURT");
    CHECK_EQ(result.discard(), 4);
    CHECK_EQ(result.size(), data.size());

    // 验证函数式调用结果与直接调用一致
    for (size_t i = result.discard(); i < result.size(); ++i) {
        CHECK(result[i] > -1.5);
        CHECK(result[i] < -1.0);
    }
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_KURT_benchmark") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0));
    Indicator c = kdata.close();
    int cycle = 1000;  // 测试循环次数

    {
        BENCHMARK_TIME_MSG(test_KURT_benchmark, cycle, fmt::format("data len: {}", c.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator result = KURT(c, 200);
        }
    }
}
#endif

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_KURT_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/KURT.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = KURT(CLOSE(kdata), 10);
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

    CHECK_EQ(x2.name(), "KURT");
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