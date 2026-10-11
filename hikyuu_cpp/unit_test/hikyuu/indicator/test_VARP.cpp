/*
 * test_VARP.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/VARP.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/MA.h>

using namespace hku;

/**
 * @defgroup test_indicator_VARP test_indicator_VARP
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_VARP") {
    /** @arg The normal case with n > 1 */
    PriceList d;
    for (size_t i = 0; i < 15; ++i) {
        d.push_back(i + 1);
    }
    d[5] = 4.0;
    d[7] = 4.0;
    d[11] = 6.0;

    Indicator ind = PRICELIST(d);
    Indicator dev = VARP(ind, 10);
    CHECK_EQ(dev.name(), "VARP");
    CHECK_EQ(dev.size(), 15);
    CHECK_EQ(dev.discard(), 9);

    price_t nan = Null<price_t>();
    vector<price_t> expected{nan, nan,  nan,  nan,  nan,  nan,   nan,  nan,
                             nan, 7.69, 8.89, 7.21, 9.61, 12.01, 14.41};
    for (size_t i = 0; i < dev.discard(); i++) {
        CHECK_UNARY(std::isnan(dev[i]));
    }
    for (size_t i = dev.discard(); i < dev.size(); i++) {
        CHECK_EQ(expected[i], doctest::Approx(dev[i]).epsilon(0.0001));
    }

    /** @arg When n = 1 */
    CHECK_THROWS_AS(VARP(ind, 1), std::exception);

    /** @arg operator() */
    Indicator expect = VARP(ind, 10);
    dev = VARP(10);
    Indicator result = dev(ind);
    CHECK_EQ(result.size(), expect.size());
    for (size_t i = result.discard(); i < expect.size(); ++i) {
        CHECK_EQ(result[i], expect[i]);
    }

    /** @arg When n = 0: expanding window, population var of [0, i], single point -> 0 */
    dev = VARP(ind, 0);
    CHECK_EQ(dev.size(), 15);
    CHECK_EQ(dev.discard(), 0);
    CHECK_EQ(dev[0], 0.0);
    for (size_t i = 1; i < dev.size(); ++i) {
        CHECK_UNARY(!std::isnan(dev[i]));
    }
    // Full-sample pop var at the last bar matches the previous whole-series value
    CHECK_EQ(dev[14], doctest::Approx(19.0933).epsilon(0.0001));
}

/** @par Test points */
TEST_CASE("test_VARP_dyn") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-30));
    // KData kdata = stock.getKData(KQuery(0, Null<size_t>(), KQuery::MIN));
    Indicator c = CLOSE(kdata);
    Indicator expect = VARP(c, 10);
    Indicator result = VARP(c, CVAL(c, 10));
    // CHECK_EQ(expect.discard(), result.discard());
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    result = VARP(c, IndParam(CVAL(c, 10)));
    // CHECK_EQ(expect.discard(), result.discard());
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }
}

/** @par Test points: static-n full vs incremental equivalence (ring-buffer rewrite) */
TEST_CASE("test_VARP_increment_equivalence") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData k_full = stock.getKData(KQuery(0, 30));
    CHECK_EQ(k_full.size(), 30);

    Indicator expect = VARP(CLOSE(), 10);
    expect.setContext(k_full);

    auto check = [&](size_t history) {
        Indicator got = VARP(CLOSE(), 10);
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

    check(10);
    check(15);
    check(13);
}

/** @par Test points: ST-N03 — after a mid-series NaN slides out of the window, subsequent bars
 * recover to finite */
TEST_CASE("test_VARP_nan_recovery") {
    price_t nan = Null<price_t>();
    Indicator data = PRICELIST({1.0, 2.0, 3.0, nan, 5.0, 6.0});
    Indicator r = VARP(data, 3);
    for (size_t i = 3; i < 6; i++) {
        CHECK_UNARY(std::isfinite(r[i]));
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------

#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_VARP_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/VARP.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = VARP(CLOSE(kdata), 10);
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

    CHECK_EQ(ma2.name(), "VARP");
    CHECK_EQ(ma1.size(), ma2.size());
    CHECK_EQ(ma1.discard(), ma2.discard());
    CHECK_EQ(ma1.getResultNumber(), ma2.getResultNumber());
    for (size_t i = ma1.discard(); i < ma1.size(); ++i) {
        CHECK_EQ(ma1[i], doctest::Approx(ma2[i]).epsilon(0.00001));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
