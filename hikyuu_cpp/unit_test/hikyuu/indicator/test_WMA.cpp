/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-15
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/WMA.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_WMA test_indicator_WMA
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_WMA") {
    Indicator result;

    PriceList a;
    for (int i = 0; i < 10; ++i) {
        a.push_back(i);
    }

    Indicator data = PRICELIST(a);

    /** @arg n <= 0 */
    CHECK_THROWS(WMA(0));
    CHECK_THROWS(WMA(-1));
    CHECK_THROWS(WMA(data, 0));
    CHECK_THROWS(WMA(data, -1));

    /** @arg n = 1 */
    result = WMA(data, 1);
    CHECK_EQ(result.name(), "WMA");
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 0);
    for (int i = 0; i < data.size(); ++i) {
        CHECK_EQ(result[i], data[i]);
    }

    /** @arg n = 2 */
    result = WMA(data, 2);
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 1);
    for (int i = 0; i < result.discard(); ++i) {
        CHECK_UNARY(std::isnan(result[i]));
    }

    vector<Indicator::value_t> expect{0.0,      2. / 3.,  5. / 3.,  8. / 3.,  11. / 3,
                                      14. / 3., 17. / 3., 20. / 3., 23. / 3., 26. / 3.};
    for (int i = result.discard(); i < result.size(); ++i) {
        CHECK_EQ(result[i], doctest::Approx(expect[i]).epsilon(0.00001));
    }

    /** @arg n = 9 */
    result = WMA(data, 9);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.discard(), 8);
    for (int i = 0; i < result.discard(); ++i) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    CHECK_EQ(result[8], doctest::Approx(5.33333).epsilon(0.0001));
    CHECK_EQ(result[9], doctest::Approx(6.33333).epsilon(0.0001));

    /** @arg n = 10 */
    result = WMA(data, 10);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.discard(), 9);
    CHECK_EQ(result[9], doctest::Approx(6.).epsilon(0.0001));
}

/** @par 检测点 */
TEST_CASE("test_WMA_dyn") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-30));
    // KData kdata = stock.getKData(KQuery(0, Null<size_t>(), KQuery::MIN));
    Indicator c = CLOSE(kdata);
    Indicator expect = WMA(c, 10);
    Indicator result = WMA(c, CVAL(c, 10));
    CHECK_EQ(expect.size(), result.size());
    CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    result = WMA(c, IndParam(CVAL(c, 10)));
    CHECK_EQ(expect.size(), result.size());
    CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_WMA_benchmark") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0));
    Indicator c = kdata.close();
    int cycle = 1000;  // 测试循环次数

    {
        BENCHMARK_TIME_MSG(test_WMA_benchmark, cycle, fmt::format("data len: {}", c.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator result = WMA(c);
        }
    }
}
#endif

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_WMA_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/WMA.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = WMA(CLOSE(kdata), 3);
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

    CHECK_EQ(x1.name(), "WMA");
    CHECK_EQ(x1.name(), x2.name());
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = x1.discard(); i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]).epsilon(0.00001));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
