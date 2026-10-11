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

/** @par Test points */
TEST_CASE("test_VAR") {
    /** @arg The normal case with n > 1 */
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

    /** @arg The invalid parameter n = 1 */
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

    /** @arg When n = 0: expanding window, sample var of [0, i], requires at least 2 points */
    dev = VAR(ind, 0);
    CHECK_EQ(dev.size(), 15);
    CHECK_EQ(dev.discard(), 1);
    CHECK_UNARY(std::isnan(dev[0]));
    for (size_t i = 1; i < dev.size(); ++i) {
        CHECK_UNARY(!std::isnan(dev[i]));
    }
    // Full-sample var at the last bar matches the previous whole-series value
    CHECK_EQ(dev[14], doctest::Approx(20.457143).epsilon(0.0001));
}

/** @par Test points: static-n full vs incremental equivalence (ring-buffer rewrite) */
TEST_CASE("test_VAR_increment_equivalence") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData k_full = stock.getKData(KQuery(0, 30));
    CHECK_EQ(k_full.size(), 30);

    Indicator expect = VAR(CLOSE(), 10);
    expect.setContext(k_full);

    auto check = [&](size_t history) {
        Indicator got = VAR(CLOSE(), 10);
        got.setContext(stock.getKData(KQuery(0, history)));
        got.setContext(k_full);
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

    /** @arg history == n, start_pos lands at the ring wrap boundary */
    check(10);
    /** @arg ordinary incremental case */
    check(15);
    /** @arg history not aligned with n, exercising ring_ix = start_pos % n at a non-zero offset */
    check(13);
}

/**
 * @par Test points
 * The bars read backwards as the seed of a rolling statistic must lie inside the valid part of the
 * input, otherwise the null prefix poisons the whole result.
 */
TEST_CASE("test_VAR_increment_seed_into_discard") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData k_old = stock.getKData(KQuery(0, 30));
    KData k_new = stock.getKData(KQuery(0, 40));
    CHECK_EQ(k_old.size(), 30);
    CHECK_EQ(k_new.size(), 40);

    // The long warm up makes the node discard end exactly at the last cached bar: resuming from it
    // would read the first bar before the valid part of the input as the seed
    auto make = []() { return VAR(MA(CLOSE(), 21), 10); };

    Indicator expect = make();
    expect.setContext(k_new);

    Indicator got = make();
    got.setContext(k_old);
    got.setContext(k_new);

    /** @arg the resumed calculation keeps the same values as a fresh one */
    CHECK_EQ(got.size(), expect.size());
    CHECK_EQ(got.discard(), expect.discard());
    for (size_t i = expect.discard(); i < expect.size(); ++i) {
        CHECK_EQ(got[i], doctest::Approx(expect[i]).epsilon(0.00001));
    }
}

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

/** @par Test points: ST-N03 — after a mid-series NaN slides out of the window, subsequent bars
 * recover to finite */
TEST_CASE("test_VAR_nan_recovery") {
    price_t nan = Null<price_t>();
    Indicator data = PRICELIST({1.0, 2.0, 3.0, nan, 5.0, 6.0});
    Indicator r = VAR(data, 3);
    // i=3 window [1,2,NaN]: valid=2 = {2,3}, sample var = 0.5
    CHECK_EQ(r[3], doctest::Approx(0.5));
    // i=4 window [2,NaN,5]: valid=2 = {3,5}, sample var = 2
    CHECK_EQ(r[4], doctest::Approx(2.0));
    // i=5 window [NaN,5,6]: valid=2 = {5,6}, sample var = 0.5 (NaN slides out, full recovery)
    CHECK_EQ(r[5], doctest::Approx(0.5));
}

/** @par Test points: ST-N14 — non-finite anchor falls back to 0.0 instead of poisoning the whole
 * run */
TEST_CASE("test_VAR_anchor_non_finite") {
    price_t nan = Null<price_t>();
    // leading NaN with discard=0: anchor src[0] is NaN, fallback k=0 keeps finite points usable
    Indicator data = PRICELIST({nan, 1.0, 2.0, 3.0});
    Indicator r = VAR(data, 3);
    // i=2 window [NaN,1,2]: valid=2, sample var of {1,2} = 0.5
    CHECK_EQ(r[2], doctest::Approx(0.5));
    // i=3 window [1,2,3]: fully finite, sample var = 1.0
    CHECK_EQ(r[3], doctest::Approx(1.0));
}

/** @par Test points: ST-N13 — dynamic path returns Null when the window holds non-finite values,
 * matching the static semantics */
TEST_CASE("test_VAR_dynamic_non_finite_window") {
    price_t nan = Null<price_t>();
    Indicator data = PRICELIST({1.0, 2.0, nan, 4.0, 5.0});
    Indicator dyn = VAR(data, CVAL(data, 3));
    Indicator stat = VAR(data, 3);
    for (size_t i = 2; i < data.size(); i++) {
        CHECK_UNARY(std::isnan(dyn[i]) == std::isnan(stat[i]));
    }
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------

#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_VAR_benchmark") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0));
    Indicator c = kdata.close();
    int cycle = 1000;  // Test loop count

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

/** @par Test points */
//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
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
