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

/** @par Test points */
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

/** @par Test points */
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
    int cycle = 1000;  // Test loop count

    {
        BENCHMARK_TIME_MSG(test_WMA_benchmark, cycle, fmt::format("data len: {}", c.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator result = WMA(c);
        }
    }
}
#endif

/**
 * @par Test points
 * Incremental calculate must match a full calculation, including the narrow boundary where the
 * framework puts start_pos at n-1.
 *
 * Background: min_increment_start() returned n-1 while _increment_calculate seeded at
 * src[start_pos-n], which needs n elements before start_pos. At start_pos == n-1 that index became
 * (size_t)-1: the seed loop was skipped (SIZE_MAX < start_pos) and src[trailingIdx++] read
 * src[-1] before wrapping back to 0, so every value from start_pos on diverged.
 */
TEST_CASE("test_WMA_increment_equivalence") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData k_full = stock.getKData(KQuery(0, 20));
    CHECK_EQ(k_full.size(), 20);

    Indicator expect = WMA(CLOSE(), 10);
    expect.setContext(k_full);

    auto check_with_history = [&](size_t history) {
        Indicator got = WMA(CLOSE(), 10);
        got.setContext(stock.getKData(KQuery(0, history)));  // cache m_old_context
        got.setContext(k_full);                              // extended at the tail
        CHECK_EQ(got.size(), expect.size());
        CHECK_EQ(got.discard(), expect.discard());
        for (size_t i = 0; i < expect.size(); ++i) {
            double a = expect[i];
            double b = got[i];
            if (std::isnan(a) && std::isnan(b)) {
                continue;
            }
            CHECK_EQ(b, doctest::Approx(a).epsilon(0.0001));
        }
    };

    /** @arg The old context holds exactly n bars, so start_pos == n - 1 (the boundary) */
    check_with_history(10);

    /** @arg The old context holds more than n bars, the usual incremental path */
    check_with_history(15);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
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
