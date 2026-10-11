/*
 * test_STDP.cpp
 *
 *  Created on: 2019-4-18
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/STDP.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_STDP test_indicator_STDP
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_STDP") {
    /** @arg The normal case with n > 1 */
    PriceList d;
    for (size_t i = 0; i < 15; ++i) {
        d.push_back(i + 1);
    }
    d[5] = 4.0;
    d[7] = 4.0;
    d[11] = 6.0;

    Indicator ind = PRICELIST(d);
    Indicator dev = STDP(ind, 10);
    CHECK_EQ(dev.name(), "STDP");
    CHECK_EQ(dev.size(), 15);

    CHECK_EQ(dev.discard(), 9);
    for (size_t i = 0; i < 9; ++i) {
        CHECK_UNARY(std::isnan(dev[i]));
    }
    vector<price_t> expected{2.77308, 2.98161, 2.68514, 3.1, 3.46554, 3.79605};
    for (size_t i = 0; i < expected.size(); ++i) {
        CHECK_EQ(dev[9 + i], doctest::Approx(expected[i]).epsilon(0.0001));
    }

    /** @arg When n = 1 */
    CHECK_THROWS_AS(STDP(ind, 1), std::exception);

    /** @arg operator() */
    Indicator expect = STDP(ind, 10);
    dev = STDP(10);
    Indicator result = dev(ind);
    CHECK_EQ(result.size(), expect.size());
    for (size_t i = result.discard(); i < expect.size(); ++i) {
        CHECK_EQ(result[i], expect[i]);
    }
}

/** @par Test points */
TEST_CASE("test_STDP_dyn") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-30));
    // KData kdata = stock.getKData(KQuery(0, Null<size_t>(), KQuery::MIN));
    Indicator c = CLOSE(kdata);
    Indicator expect = STDP(c, 10);
    Indicator result = STDP(c, CVAL(c, 10));
    CHECK_EQ(expect.discard(), result.discard());
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    result = STDP(c, IndParam(CVAL(c, 10)));
    CHECK_EQ(expect.discard(), result.discard());
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }
}

/** @par Test points: static-n full vs incremental equivalence (ring-buffer rewrite) */
TEST_CASE("test_STDP_increment_equivalence") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData k_full = stock.getKData(KQuery(0, 30));
    CHECK_EQ(k_full.size(), 30);

    Indicator expect = STDP(CLOSE(), 10);
    expect.setContext(k_full);

    auto check = [&](size_t history) {
        Indicator got = STDP(CLOSE(), 10);
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

/** @par Test points */
TEST_CASE("test_STDP_warm_up_discard") {
    PriceList px{10., 12., 9., 11., 8., 13., 10., 12., 7., 11.};
    Indicator src = PRICELIST(px);

    Indicator r = STDP(src, 5);
    CHECK_EQ(r.size(), 10);

    /** @arg the bars without a full window are discarded */
    CHECK_EQ(r.discard(), 4);

    /** @arg the first kept value is the population std of the first full window */
    price_t mean = (10. + 12. + 9. + 11. + 8.) / 5.;
    price_t var = 0.0;
    for (price_t v : {10., 12., 9., 11., 8.}) {
        var += (v - mean) * (v - mean);
    }
    CHECK_EQ(r[4], doctest::Approx(std::sqrt(var / 5.)).epsilon(0.00001));
    /** @arg a later kept value is the population std of its own window */
    price_t mean2 = (13. + 10. + 12. + 7. + 11.) / 5.;
    price_t var2 = 0.0;
    for (price_t v : {13., 10., 12., 7., 11.}) {
        var2 += (v - mean2) * (v - mean2);
    }
    CHECK_EQ(r[9], doctest::Approx(std::sqrt(var2 / 5.)).epsilon(0.00001));
}

/** @par Test points: ST-N03 — after a mid-series NaN slides out of the window, subsequent bars
 * recover to finite */
TEST_CASE("test_STDP_nan_recovery") {
    price_t nan = Null<price_t>();
    Indicator data = PRICELIST({1.0, 2.0, 3.0, nan, 5.0, 6.0});
    Indicator r = STDP(data, 3);
    for (size_t i = 3; i < 6; i++) {
        CHECK_UNARY(std::isfinite(r[i]));
    }
}

/** @par Test points: ST-N13 — dynamic path returns Null (never a fake 0.0) for an all-non-finite
 * window */
TEST_CASE("test_STDP_dynamic_all_nan_window") {
    price_t nan = Null<price_t>();
    Indicator data = PRICELIST({nan, nan, nan, nan});
    Indicator dyn = STDP(data, CVAL(data, 3));
    for (size_t i = 0; i < data.size(); i++) {
        CHECK_UNARY(std::isnan(dyn[i]));
    }
}

/** @par Test points: ST-N14 — non-finite anchor falls back to 0.0 instead of poisoning the whole
 * run */
TEST_CASE("test_STDP_anchor_non_finite") {
    price_t nan = Null<price_t>();
    Indicator data = PRICELIST({nan, 1.0, 2.0, 3.0});
    Indicator r = STDP(data, 3);
    // i=2 window [NaN,1,2]: valid=2, population std of {1,2} = 0.5
    CHECK_EQ(r[2], doctest::Approx(0.5));
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------

#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_STDP_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/STDP.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = STDP(CLOSE(kdata), 10);
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

    CHECK_EQ(ma2.name(), "STDP");
    CHECK_EQ(ma1.size(), ma2.size());
    CHECK_EQ(ma1.discard(), ma2.discard());
    CHECK_EQ(ma1.getResultNumber(), ma2.getResultNumber());
    for (size_t i = 0; i < ma1.discard(); i++) {
        CHECK_UNARY(std::isnan(ma2[i]));
    }
    for (size_t i = ma1.discard(); i < ma1.size(); ++i) {
        CHECK_EQ(ma1[i], doctest::Approx(ma2[i]).epsilon(0.00001));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
