/*
 * test_COS.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/CORR.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_COS test_indicator_CORR
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
/** @par Test points */
TEST_CASE("test_CORR_zero_variance") {
    PriceList flat, moving;
    for (size_t i = 0; i < 8; ++i) {
        flat.push_back(5.0);
        moving.push_back(1.0 + i);
    }

    /** @arg a constant first input has no correlation, whatever the other input does */
    Indicator r = CORR(PRICELIST(flat), PRICELIST(moving), 3);
    REQUIRE_EQ(r.size(), 8);
    for (size_t i = r.discard(); i < 8; ++i) {
        CHECK_UNARY(std::isnan(r[i]));
    }
    /** @arg the covariance result set keeps its own meaningful zero */
    CHECK_EQ(r.get(5, 1), 0.0);

    /** @arg a constant second input behaves the same */
    Indicator r2 = CORR(PRICELIST(moving), PRICELIST(flat), 3);
    for (size_t i = r2.discard(); i < 8; ++i) {
        CHECK_UNARY(std::isnan(r2[i]));
    }
}

/** @par Test points: ST-034 — Inf inputs produce Null output, never ±Inf */
TEST_CASE("test_CORR_inf_input_yields_null") {
    price_t inf_v = std::numeric_limits<price_t>::infinity();
    PriceList x, y;
    for (size_t i = 0; i < 8; ++i) {
        x.push_back(1.0 + i);
        y.push_back(i == 3 ? inf_v : (2.0 + i));
    }
    Indicator r = CORR(PRICELIST(x), PRICELIST(y), 3);
    for (size_t i = r.discard(); i < 8; ++i) {
        /** @arg window overlapping the Inf yields Null (guard fires on non-finite variance/cov) */
        CHECK_UNARY(std::isnan(r[i]) || std::isfinite(r[i]));
    }
}

/** @par Test points: ST-N14 — non-finite anchor at rolling-window seed position falls back to 0.0,
 * does not force whole output to Null */
TEST_CASE("test_CORR_anchor_non_finite") {
    price_t nan = Null<price_t>();
    // leading NaN at index 0; n=3 rolling; anchor kx/ky at discard=0 would be NaN without fallback
    Indicator x = PRICELIST({nan, 1.0, 2.0, 3.0, 4.0});
    Indicator y = PRICELIST({nan, 2.0, 4.0, 6.0, 8.0});
    Indicator r = CORR(x, y, 3);
    /** @arg anchor fallback keeps the windows computable; perfect linear y=2x gives corr = 1 */
    CHECK_EQ(r.discard(), 2);
    CHECK_EQ(r[2], doctest::Approx(1.0));
    CHECK_EQ(r[3], doctest::Approx(1.0));
    CHECK_EQ(r[4], doctest::Approx(1.0));
}

TEST_CASE("test_CORR") {
    Indicator result;

    // An empty indicator
    result = CORR(Indicator(), Indicator(), 10);
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

    // The invalid parameter n
    CHECK_THROWS_AS(CORR(x, y, -1), std::exception);
    CHECK_THROWS_AS(CORR(x, y, 1), std::exception);

    // The normal case, n = 0: expanding window
    result = CORR(x, y, 0);
    CHECK_EQ(result.name(), "CORR");
    CHECK_EQ(result.discard(), 1);
    CHECK_EQ(result.size(), a.size());

    price_t nan = Null<price_t>();
    // bar 0 has no pair yet -> NaN; bars 1..total-1 each carry an expanding-window Pearson corr
    CHECK_UNARY(std::isnan(result[0]));
    for (size_t i = 1; i < result.size(); ++i) {
        CHECK_UNARY(!std::isnan(result[i]));
    }
    // Full-sample Pearson corr at the last bar matches the previous whole-series value
    CHECK_EQ(result[a.size() - 1], doctest::Approx(0.385553).epsilon(0.00001));

    Indicator cov = result.getResult(1);
    CHECK_UNARY(std::isnan(cov[0]));
    CHECK_EQ(cov[a.size() - 1], doctest::Approx(0.162894).epsilon(0.00001));
    (void)nan;

    // The normal case, n = 8
    result = CORR(x, y, 8);
    CHECK_EQ(result.name(), "CORR");
    CHECK_EQ(result.discard(), 7);
    CHECK_EQ(result.size(), a.size());

    Indicator expect_corr =
      PRICELIST(PriceList{nan, nan, nan, nan, nan, nan, nan, 0.379682, 0.504541, 0.572872}, 7);
    Indicator expect_cov =
      PRICELIST(PriceList{nan, nan, nan, nan, nan, nan, nan, 0.157144, 0.250266, 0.268253}, 7);

    CHECK_UNARY(std::isnan(result[0]));
    CHECK_UNARY(std::isnan(result[1]));
    for (int i = result.discard(); i < expect_corr.size(); ++i) {
        CHECK_EQ(result[i], doctest::Approx(expect_corr[i]).epsilon(0.00001));
    }

    cov = result.getResult(1);
    CHECK_UNARY(std::isnan(cov[0]));
    CHECK_UNARY(std::isnan(cov[1]));
    for (int i = cov.discard(); i < expect_cov.size(); ++i) {
        CHECK_EQ(cov[i], doctest::Approx(expect_cov[i]).epsilon(0.00001));
    }
}

/** @par Test points: ST-N03 — after a mid-series NaN pair slides out of the window, corr recovers
 */
TEST_CASE("test_CORR_nan_recovery") {
    price_t nan = Null<price_t>();
    Indicator x = PRICELIST({1.0, 2.0, 3.0, 4.0, nan, 6.0, 7.0, 8.0, 9.0});
    Indicator y = PRICELIST({2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0, 18.0});
    Indicator r = CORR(x, y, 3);
    // Perfect linear relation on finite points keeps corr at 1.0 once NaN slides through
    for (size_t i = 6; i < 9; i++) {
        CHECK_UNARY(std::isnan(r[i]) || r[i] == doctest::Approx(1.0));
    }
    // At least the post-NaN window [6,7,8] is perfectly finite (NaN fully slid out)
    CHECK_EQ(r[8], doctest::Approx(1.0));
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_CORR_benchmark") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0));
    Indicator c = kdata.close();
    Indicator h = kdata.close();
    int cycle = 1000;  // Test loop count

    {
        BENCHMARK_TIME_MSG(test_CORR_benchmark, cycle, fmt::format("data len: {}", c.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator result = CORR(c, h, 200);
        }
    }
}
#endif

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_CORR_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/COS.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = CORR(CLOSE(kdata), OPEN(kdata), 10);
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

    CHECK_EQ(x2.name(), "CORR");
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

    Indicator x11 = x1.getResult(1);
    Indicator x21 = x2.getResult(1);
    CHECK_EQ(x11.size(), x21.size());
    CHECK_EQ(x11.discard(), x21.discard());
    CHECK_EQ(x11.getResultNumber(), x21.getResultNumber());
    for (size_t i = x11.discard(); i < x21.size(); ++i) {
        if (std::isnan(x11[i])) {
            CHECK_UNARY(std::isnan(x21[i]));
        } else {
            CHECK_EQ(x11[i], doctest::Approx(x21[i]));
        }
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
