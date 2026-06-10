/*
 * test_COV.cpp
 *
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/COV.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_COV test_indicator_COV
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_COV") {
    Indicator result;

    // 空指标
    result = COV(Indicator(), Indicator(), 10);
    CHECK_UNARY(result.empty());

    PriceList a{0.8005901401350127,  0.6634650046370404,  -1.8000894387683657, 0.3878161916948592,
                -0.5171110201088346, -1.7905249485540904, 0.737991201931744,   0.8574053319765023,
                -0.5832251010003247, -0.1394762212893227};
    PriceList b{-0.1835983655705932, 0.02319229786147859, -0.08242549911752291,
                0.6359545988575381,  0.6421570840387792,  -0.28347907849066045,
                0.2931449663168225,  0.38137876882777144, -0.6446198144780273,
                0.2433169061114869};

    Indicator x = PRICELIST(a);
    Indicator y = PRICELIST(b);

    // 非法参数 n
    CHECK_THROWS_AS(COV(x, y, -1), std::exception);
    CHECK_THROWS_AS(COV(x, y, 1), std::exception);

    // 正常情况，n = 0
    result = COV(x, y, 0);
    CHECK_EQ(result.name(), "COV");
    CHECK_EQ(result.discard(), 9);
    CHECK_EQ(result.size(), a.size());

    price_t nan = Null<price_t>();
    Indicator expect_cov =
      PRICELIST(PriceList{nan, nan, nan, nan, nan, nan, nan, nan, nan, 0.162894});

    CHECK_UNARY(std::isnan(result[0]));
    CHECK_UNARY(std::isnan(result[1]));
    for (int i = result.discard(); i < expect_cov.size(); ++i) {
        CHECK_EQ(result[i], doctest::Approx(expect_cov[i]).epsilon(0.00001));
    }

    // 正常情况，n = 8
    result = COV(x, y, 8);
    CHECK_EQ(result.name(), "COV");
    CHECK_EQ(result.discard(), 7);
    CHECK_EQ(result.size(), a.size());

    expect_cov =
      PRICELIST(PriceList{nan, nan, nan, nan, nan, nan, nan, 0.157144, 0.250266, 0.268253}, 7);

    CHECK_UNARY(std::isnan(result[0]));
    CHECK_UNARY(std::isnan(result[1]));
    for (int i = result.discard(); i < expect_cov.size(); ++i) {
        CHECK_EQ(result[i], doctest::Approx(expect_cov[i]).epsilon(0.00001));
    }

    // 测试函数式调用方式 COV(ref_ind, n)(ind)
    result = COV(y, 8)(x);
    CHECK_EQ(result.name(), "COV");
    CHECK_EQ(result.discard(), 7);
    CHECK_EQ(result.size(), a.size());
    for (int i = result.discard(); i < expect_cov.size(); ++i) {
        CHECK_EQ(result[i], doctest::Approx(expect_cov[i]).epsilon(0.00001));
    }
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_COV_benchmark") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0));
    Indicator c = kdata.close();
    Indicator h = kdata.close();
    int cycle = 1000;  // 测试循环次数

    {
        BENCHMARK_TIME_MSG(test_COV_benchmark, cycle, fmt::format("data len: {}", c.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator result = COV(c, h, 200);
        }
    }
}
#endif

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_COV_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/COV.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = COV(CLOSE(kdata), OPEN(kdata), 10);
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

    CHECK_EQ(x2.name(), "COV");
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