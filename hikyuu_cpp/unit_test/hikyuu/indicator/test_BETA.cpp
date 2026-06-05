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

/** @par 检测点 */
TEST_CASE("test_BETA") {
    Indicator result;

    // 空指标
    result = BETA(Indicator(), Indicator(), 60);
    CHECK_UNARY(result.empty());

    // 创建测试数据
    // 股票收益率和市场收益率
    PriceList stock_returns{0.01, 0.02, -0.01, 0.015, 0.005, -0.02, 0.012, 0.018, -0.008, 0.02};
    PriceList market_returns{0.008,  0.015, -0.005, 0.012,  0.003,
                             -0.015, 0.008, 0.012,  -0.003, 0.015};

    Indicator stock_ind = PRICELIST(stock_returns);
    Indicator market_ind = PRICELIST(market_returns);

    // 非法参数 n
    CHECK_THROWS_AS(BETA(stock_ind, market_ind, -1), std::exception);
    CHECK_THROWS_AS(BETA(stock_ind, market_ind, 1), std::exception);

    // 正常情况，n = 0（全样本计算）
    result = BETA(stock_ind, market_ind, 0);
    CHECK_EQ(result.name(), "BETA");
    CHECK_EQ(result.size(), stock_returns.size());
    CHECK_EQ(result.discard(), 9);
    // 根据 IBeta 实现，kx = first_stock_value, ky = first_market_value
    // 从第二个数据点开始累加计算
    // stock: [0.01, 0.02, -0.01, 0.015, 0.005, -0.02, 0.012, 0.018, -0.008, 0.02]
    // market: [0.008, 0.015, -0.005, 0.012, 0.003, -0.015, 0.008, 0.012, -0.003, 0.015]
    CHECK_EQ(result[9], doctest::Approx(1.41516).epsilon(0.001));

    // 测试滚动窗口 n = 8
    result = BETA(stock_ind, market_ind, 8);
    CHECK_EQ(result.name(), "BETA");
    CHECK_EQ(result.size(), stock_returns.size());
    CHECK_EQ(result.discard(), 7);

    // 验证前几个值为 nan（需要足够数据才能计算）
    for (size_t i = 0; i < result.discard(); ++i) {
        CHECK_UNARY(std::isnan(result[i]));
    }

    // 验证具体计算结果
    // 窗口 [0:8) 的 Beta 值
    CHECK_EQ(result[7], doctest::Approx(1.38916).epsilon(0.001));
    // 窗口 [1:9) 的 Beta 值
    CHECK_EQ(result[8], doctest::Approx(1.42331).epsilon(0.001));
    // 窗口 [2:10) 的 Beta 值
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

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_BETA_benchmark") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0));
    Indicator c = kdata.close();
    Indicator h = kdata.close();
    int cycle = 1000;  // 测试循环次数

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

/** @par 检测点 */
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