/*
 * test_VAR.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/VAR.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_VAR test_indicator_VAR
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_VAR") {
    /** @arg n > 1 的正常情况 */
    PriceList d;
    for (size_t i = 0; i < 15; ++i) {
        d.push_back(i + 1);
    }
    d[5] = 4.0;
    d[7] = 4.0;
    d[11] = 6.0;

    Indicator ind = PRICELIST(d);
    Indicator dev = VAR(ind, 10);
    CHECK_EQ(dev.name(), "VAR");
    CHECK_EQ(dev.size(), 15);
    CHECK_EQ(dev.discard(), 9);

    price_t nan = Null<price_t>();
    vector<price_t> expected{nan, nan,     nan,     nan,     nan,     nan,     nan,    nan,
                             nan, 8.54444, 9.87778, 8.01111, 10.6778, 13.3444, 16.0111};
    for (size_t i = 0; i < dev.discard(); i++) {
        CHECK_UNARY(std::isnan(dev[i]));
    }
    for (size_t i = dev.discard(); i < dev.size(); i++) {
        CHECK_EQ(dev[i], doctest::Approx(expected[i]).epsilon(0.001));
    }

    /** @arg 非法参数 n = 1时 */
    CHECK_THROWS_AS(VAR(ind, 1), std::exception);

    /** @arg operator() */
    Indicator expect = VAR(ind, 10);
    dev = VAR(10);
    CHECK_EQ(dev.name(), "VAR");
    Indicator result = dev(ind);
    CHECK_EQ(result.size(), expect.size());
    for (size_t i = expect.discard(); i < expect.size(); ++i) {
        CHECK_EQ(result[i], expect[i]);
    }

    /** @arg n =0 时 */
    dev = VAR(ind, 0);
    CHECK_EQ(dev.size(), 15);
    CHECK_EQ(dev.discard(), 14);
    CHECK_EQ(dev[14], doctest::Approx(20.457143).epsilon(0.0001));
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_VAR_benchmark") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0));
    Indicator c = kdata.close();
    int cycle = 1000;  // 测试循环次数

    {
        BENCHMARK_TIME_MSG(test_VAR_benchmark, cycle, fmt::format("data len: {}", c.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator ind = VAR();
            Indicator result = ind(c);
        }
    }
}
#endif

/** @par 检测点 */
TEST_CASE("test_VAR_dyn") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-30));
    // KData kdata = stock.getKData(KQuery(0, Null<size_t>(), KQuery::MIN));
    Indicator c = CLOSE(kdata);
    Indicator expect = VAR(c, 10);
    Indicator result = VAR(c, CVAL(c, 10));
    // CHECK_EQ(expect.discard(), result.discard());
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    result = VAR(c, IndParam(CVAL(c, 10)));
    // CHECK_EQ(expect.discard(), result.discard());
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_VAR_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/VAR.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = VAR(CLOSE(kdata), 10);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(ma1);
    }

    Indicator ma2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(ma2);
    }

    CHECK_EQ(ma2.name(), "VAR");
    CHECK_EQ(ma1.size(), ma2.size());
    CHECK_EQ(ma1.discard(), ma2.discard());
    CHECK_EQ(ma1.getResultNumber(), ma2.getResultNumber());
    for (size_t i = ma1.discard(); i < ma1.size(); ++i) {
        CHECK_EQ(ma1[i], doctest::Approx(ma2[i]).epsilon(0.00001));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
