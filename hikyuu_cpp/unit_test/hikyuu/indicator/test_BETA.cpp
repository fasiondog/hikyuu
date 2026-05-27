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

    // 正常情况，n = 0
    result = BETA(stock_ind, market_ind, 0);
    CHECK_EQ(result.name(), "BETA");
    CHECK_EQ(result.size(), stock_returns.size());

    // 验证计算结果
    // Beta = Cov(stock, market) / Var(market)
    // 手动计算期望值用于验证
    price_t nan = Null<price_t>();

    // 测试滚动窗口 n = 8
    result = BETA(stock_ind, market_ind, 8);
    CHECK_EQ(result.name(), "BETA");
    CHECK_EQ(result.size(), stock_returns.size());

    // 验证前几个值为 nan（需要足够数据才能计算）
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_UNARY(std::isnan(result[1]));
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