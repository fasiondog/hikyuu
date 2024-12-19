/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-18
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator_talib/ta_crt.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/CVAL.h>

using namespace hku;

/**
 * @defgroup test_indicator_TA_CMO test_indicator_TA_CMO
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_TA_CMO") {
    KData kdata = getKData("sh000001", KQuery(-10));

    Indicator c = CLOSE(kdata);

    /** @arg 非法 n < 2 || n > 100000 */
    CHECK_THROWS(TA_CMO(c, 1));
    CHECK_THROWS(TA_CMO(c, 100001));

    /** @arg 正常情况 */
    Indicator result = TA_CMO(CLOSE(kdata), 2);
    CHECK_EQ(result.name(), "TA_CMO");
    CHECK_EQ(result.discard(), 2);
    CHECK_EQ(result.size(), kdata.size());
    CHECK_EQ(result[2], doctest::Approx(-74.8826).epsilon(0.0001));
    CHECK_EQ(result[9], doctest::Approx(-61.9996).epsilon(0.0001));
}

/** @par 检测点 */
TEST_CASE("test_TA_CMO_dyn") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-30));
    // KData kdata = stock.getKData(KQuery(0, Null<size_t>(), KQuery::MIN));
    Indicator c = CLOSE(kdata);
    Indicator expect = TA_CMO(c, 10);
    Indicator result = TA_CMO(c, CVAL(c, 10));
    CHECK_EQ(expect.size(), result.size());
    CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        // HKU_INFO("{}: {}, {}", i, result[i], expect[i]);
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    result = TA_CMO(c, IndParam(CVAL(c, 10)));
    CHECK_EQ(expect.size(), result.size());
    // CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    // expect = EMA(c, 2);
    // result = EMA(c, CVAL(c, 2));
    // CHECK_EQ(expect.size(), result.size());
    // // CHECK_EQ(expect.discard(), result.discard());
    // for (size_t i = 0; i < result.discard(); i++) {
    //     CHECK_UNARY(std::isnan(result[i]));
    // }
    // for (size_t i = expect.discard(); i < expect.size(); i++) {
    //     CHECK_EQ(expect[i], doctest::Approx(result[i]));
    // }
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_TA_CMO_benchmark") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0));
    Indicator c = kdata.close();
    int cycle = 1000;  // 测试循环次数

    {
        BENCHMARK_TIME_MSG(test_TA_CMO_benchmark, cycle, fmt::format("data len: {}", c.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator result = TA_CMO(c);
        }
    }
}
#endif

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_TA_CMO_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/TA_CMO.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = TA_CMO(CLOSE(kdata));
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
    CHECK_UNARY(x1.size() == x2.size());
    CHECK_UNARY(x1.discard() == x2.discard());
    CHECK_UNARY(x1.getResultNumber() == x2.getResultNumber());
    for (size_t i = x1.discard(); i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]).epsilon(0.00001));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
