/*
 * test_SLOPE.cpp
 *
 *  Created on: 2013-2-12
 *      Author: fasiondog
 */
#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/SLOPE.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_SLOPE test_indicator_SLOPE
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_SLOPE") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData kdata;
    Indicator slope;

    /** @arg An empty indicator */
    slope = SLOPE();
    CHECK_EQ(slope.size(), 0);
    CHECK_EQ(slope.name(), "SLOPE");

    /** @arg n = 0, 1 */
    kdata = stock.getKData(KQuery(-100));
    Indicator c = CLOSE(kdata);
    slope = SLOPE(c, 0);
    CHECK_EQ(slope.size(), c.size());
    CHECK_EQ(slope.name(), "SLOPE");
    CHECK_EQ(slope.discard(), 1);
    CHECK_UNARY(std::isnan(slope[0]));
    for (size_t i = slope.discard(), len = slope.size(); i < len; i++) {
        CHECK_EQ(slope[i], 0.);
    }

    slope = SLOPE(c, 1);
    CHECK_EQ(slope.size(), c.size());
    CHECK_EQ(slope.name(), "SLOPE");
    CHECK_EQ(slope.discard(), 1);
    CHECK_UNARY(std::isnan(slope[0]));
    for (size_t i = slope.discard(), len = slope.size(); i < len; i++) {
        CHECK_EQ(slope[i], 0.0);
    }

    /** @arg n = 2 */
    slope = SLOPE(c, 2);
    CHECK_EQ(slope.discard(), 1);
    CHECK_EQ(slope.size(), c.size());
    CHECK_UNARY(std::isnan(slope[0]));
    CHECK_EQ(slope[1], doctest::Approx(40.894).epsilon(0.0001));
    CHECK_EQ(slope[2], doctest::Approx(14.968).epsilon(0.0001));
    CHECK_EQ(slope[3], doctest::Approx(9.725).epsilon(0.0001));

    /** @arg n = 3 */
    slope = SLOPE(c, 3);
    CHECK_EQ(slope.discard(), 1);
    CHECK_EQ(slope.size(), c.size());
    CHECK_UNARY(std::isnan(slope[0]));
    CHECK_EQ(slope[1], doctest::Approx(40.894).epsilon(0.0001));
    CHECK_EQ(slope[2], doctest::Approx(27.931).epsilon(0.0001));
    CHECK_EQ(slope[3], doctest::Approx(12.347).epsilon(0.0001));
}

/** @par Test points */
TEST_CASE("test_SLOPE_dyn") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-30));
    Indicator c = CLOSE(kdata);
    Indicator expect = SLOPE(c, 10);
    Indicator result = SLOPE(c, CVAL(c, 10));
    CHECK_EQ(expect.size(), result.size());
    CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    result = SLOPE(c, IndParam(CVAL(c, 10)));
    CHECK_EQ(expect.size(), result.size());
    CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    expect = SLOPE(c, 0);
    result = SLOPE(c, CVAL(c, 0));
    CHECK_EQ(expect.size(), result.size());
    CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }
}

/** @par Test points: verify full vs incremental path equivalence on all 3 result sets */
TEST_CASE("test_SLOPE_increment_equivalence") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData k_full = stock.getKData(KQuery(0, 30));
    REQUIRE_EQ(k_full.size(), 30);

    int n = 10;
    Indicator expect = SLOPE(CLOSE(), n);
    expect.setContext(k_full);

    auto check_increment = [&](size_t history) {
        Indicator got = SLOPE(CLOSE(), n);
        got.setContext(stock.getKData(KQuery(0, history)));
        got.setContext(k_full);
        REQUIRE_EQ(got.size(), expect.size());
        REQUIRE_EQ(got.discard(), expect.discard());
        for (size_t r = 0; r < 3; r++) {
            for (size_t i = got.discard(); i < got.size(); i++) {
                double a = expect.get(i, r);
                double b = got.get(i, r);
                if (std::isnan(a) && std::isnan(b))
                    continue;
                CHECK_EQ(b, doctest::Approx(a).epsilon(0.0001));
            }
        }
    };

    /** @arg history == n: incremental rejected by min_increment_start, falls back to full calc */
    check_increment(10);

    /** @arg history == n+1: start_pos == n, minimal incremental case */
    check_increment(11);

    /** @arg history == n+2: start_pos == n+1, first steady-state rolling */
    check_increment(12);

    /** @arg typical incremental case: history >> n */
    check_increment(20);
}

/** @par Test points: known linear series should yield exact slope and r2=1 */
TEST_CASE("test_SLOPE_linear") {
    // y = 3*x + 5, slope should be 3, r2 should be 1
    size_t total = 20;
    std::vector<double> data(total);
    for (size_t i = 0; i < total; i++) {
        data[i] = 3.0 * (double)i + 5.0;
    }
    Indicator x = PRICELIST(data);
    int n = 10;
    Indicator slope = SLOPE(x, n);

    /** @arg All valid bars have slope == 3.0 */
    for (size_t i = slope.discard(); i < slope.size(); i++) {
        CHECK_EQ(slope.get(i, 0), doctest::Approx(3.0).epsilon(1e-9));
    }

    /** @arg r2 == 1.0 (perfect fit) */
    for (size_t i = slope.discard(); i < slope.size(); i++) {
        CHECK_EQ(slope.get(i, 1), doctest::Approx(1.0).epsilon(1e-9));
    }

    /** @arg relmaxres == 0 (no residual) */
    for (size_t i = slope.discard(); i < slope.size(); i++) {
        CHECK_EQ(slope.get(i, 2), doctest::Approx(0.0).epsilon(1e-9));
    }
}

/** @par Test points: constant series should not produce NaN from zero-variance division */
TEST_CASE("test_SLOPE_constant") {
    size_t total = 15;
    std::vector<double> data(total, 42.0);
    Indicator x = PRICELIST(data);
    Indicator slope = SLOPE(x, 5);

    /** @arg slope == 0 for constant series */
    for (size_t i = slope.discard(); i < slope.size(); i++) {
        CHECK_EQ(slope.get(i, 0), doctest::Approx(0.0).epsilon(1e-9));
    }

    /** @arg r2 == 0 (guard against 0/0), not NaN */
    for (size_t i = slope.discard(); i < slope.size(); i++) {
        CHECK_UNARY(!std::isnan(slope.get(i, 1)));
    }

    /** @arg relmaxres == 0 */
    for (size_t i = slope.discard(); i < slope.size(); i++) {
        CHECK_EQ(slope.get(i, 2), doctest::Approx(0.0).epsilon(1e-9));
    }
}

/** @par Test points: large bar index should not degrade regression accuracy */
TEST_CASE("test_SLOPE_large_index") {
    // Simulate large index: PRICELIST with many bars, verify slope accuracy at tail
    int n = 5;
    size_t total = 5000;
    std::vector<double> data(total);
    // Linear y = 2*i + 100 over 5000 bars (large absolute index)
    for (size_t i = 0; i < total; i++) {
        data[i] = 2.0 * (double)i + 100.0;
    }
    Indicator x = PRICELIST(data);
    Indicator slope = SLOPE(x, n);

    /** @arg slope is still accurate at large bar indices */
    size_t tail = total - 1;
    CHECK_EQ(slope.get(tail, 0), doctest::Approx(2.0).epsilon(1e-12));
    CHECK_EQ(slope.get(tail, 1), doctest::Approx(1.0).epsilon(1e-12));
}

/** @par Test points: verify warmup on an input indicator with non-zero discard */
TEST_CASE("test_SLOPE_with_discard_input") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData k = stock.getKData(KQuery(0, 40));
    REQUIRE_EQ(k.size(), 40);

    Indicator input = MA(CLOSE(), 12);
    input.setContext(k);
    REQUIRE(input.discard() > 0);

    int n = 5;
    Indicator expect = SLOPE(input, n);

    /** @arg warmup cnt fix: each valid bar equals the O(n) brute-force reference */
    auto const* src = input.data();
    size_t base = input.discard();
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        size_t wstart = (i + 1 > (size_t)n + base) ? (i + 1 - (size_t)n) : base;
        size_t cnt = i - wstart + 1;
        double fcnt = (double)cnt;
        double sum_x = fcnt * (fcnt - 1.0) / 2.0;
        double denom = fcnt * fcnt * (fcnt * fcnt - 1.0) / 12.0;
        double S_y = 0.0, S_xy = 0.0, S_y2 = 0.0;
        for (size_t k2 = 0; k2 < cnt; k2++) {
            double y = src[wstart + k2];
            S_y += y;
            S_xy += (double)k2 * y;
            S_y2 += y * y;
        }
        double ref_slope = (fcnt * S_xy - sum_x * S_y) / denom;
        CHECK_EQ(expect.get(i, 0), doctest::Approx(ref_slope).epsilon(1e-9));
    }
}

/** @par Test points: undefined relative residual when the window mean is 0 */
TEST_CASE("test_SLOPE_zero_mean") {
    // A periodic window sums to exactly 0, so ȳ == 0 makes RelMaxRes undefined
    std::vector<double> data = {1.0, -1.0, 0.0, 1.0, -1.0, 0.0, 1.0, -1.0, 0.0};
    Indicator x = PRICELIST(data);
    Indicator slope = SLOPE(x, 3);

    /** @arg result(0) slope stays finite (defined even when ȳ == 0) */
    for (size_t i = slope.discard(); i < slope.size(); i++) {
        CHECK_UNARY(!std::isnan(slope.get(i, 0)));
    }

    /** @arg result(2) is Null when the window mean is exactly 0 */
    for (size_t i = slope.discard(); i < slope.size(); i++) {
        CHECK_UNARY(std::isnan(slope.get(i, 2)));
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_SLOPE_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/SLOPE.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = SLOPE(CLOSE(kdata), 10);
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

    CHECK_EQ(ma2.name(), "SLOPE");
    CHECK_EQ(ma1.size(), ma2.size());
    CHECK_EQ(ma1.discard(), ma2.discard());
    CHECK_EQ(ma1.getResultNumber(), ma2.getResultNumber());
    for (size_t i = ma1.discard(); i < ma1.size(); ++i) {
        CHECK_EQ(ma1[i], doctest::Approx(ma2[i]));
    }
}

#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
