/*
 * test_MACD.cpp
 *
 *  Created on: 2013-4-11
 *      Author: fasiondog
 */

#include <climits>
#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/MACD.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/SLICE.h>
#include <hikyuu/indicator/crt/EMA.h>

using namespace hku;

/**
 * @defgroup test_indicator_MACD test_indicator_MACD
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_MACD") {
    PriceList d;
    for (size_t i = 0; i < 20; ++i) {
        d.push_back(i);
    }

    Indicator ind = PRICELIST(d);
    Indicator macd, bar, diff, dea;
    Indicator ema1, ema2, fast, slow, bmacd;

    /** @arg The source data is empty */
    macd = MACD(Indicator(), 12, 26, 9);
    CHECK_EQ(macd.size(), 0);
    CHECK_EQ(macd.empty(), true);

    /** @arg n1 / n2 / n3 must be >= 1 (n == 0 makes the EMA coefficient 2.0 and diverges) */
    CHECK_THROWS_AS(MACD(0, 26, 9), std::exception);
    CHECK_THROWS_AS(MACD(12, 0, 9), std::exception);
    CHECK_THROWS_AS(MACD(12, 26, 0), std::exception);
    CHECK_THROWS_AS(MACD(-1, 26, 9), std::exception);

    /** @arg n1 / n2 / n3 < INT_MAX (would overflow the 2.0 / (n + 1) expression) */
    CHECK_THROWS_AS(MACD(INT_MAX, 26, 9), std::exception);
    CHECK_THROWS_AS(MACD(12, INT_MAX, 9), std::exception);
    CHECK_THROWS_AS(MACD(12, 26, INT_MAX), std::exception);

    /** @arg n1 = n2 = n3 = 1*/
    macd = MACD(ind, 1, 1, 1);
    CHECK_EQ(macd.getResultNumber(), 3);
    bar = macd.getResult(0);
    diff = macd.getResult(1);
    dea = macd.getResult(2);
    CHECK_EQ(bar.size(), 20);
    CHECK_EQ(diff.size(), 20);
    CHECK_EQ(dea.size(), 20);

    CHECK_EQ(diff[0], 0);
    CHECK_EQ(diff[1], 0);
    CHECK_EQ(diff[19], 0);

    CHECK_EQ(dea[0], 0);
    CHECK_EQ(dea[1], 0);
    CHECK_EQ(dea[19], 0);

    CHECK_EQ(bar[0], 0);
    CHECK_EQ(bar[1], 0);
    CHECK_EQ(bar[19], 0);

    /** @arg n1 = 1 n2 = 2 n3 = 3*/
    macd = MACD(ind, 1, 2, 3);
    CHECK_EQ(macd.size(), 20);
    CHECK_EQ(macd.discard(), 0);
    bar = macd.getResult(0);
    diff = macd.getResult(1);
    dea = macd.getResult(2);
    ema1 = EMA(ind, 1);
    ema2 = EMA(ind, 2);
    fast = ema1 - ema2;
    slow = EMA(fast, 3);
    bmacd = fast - slow;
    CHECK_EQ(bar.size(), 20);
    CHECK_EQ(diff.size(), 20);
    CHECK_EQ(dea.size(), 20);

    CHECK_EQ(diff[0], doctest::Approx(fast[0]));
    CHECK_EQ(diff[1], doctest::Approx(fast[1]));
    CHECK_EQ(diff[19], doctest::Approx(fast[19]));

    CHECK_EQ(dea[0], slow[0]);
    CHECK_LT(std::fabs(dea[1] - slow[1]), 0.0001);
    CHECK_EQ(dea[19], slow[19]);

    CHECK_EQ(bar[0], doctest::Approx(bmacd[0]));
    CHECK_EQ(bar[1], doctest::Approx(bmacd[1]));
    CHECK_EQ(bar[19], doctest::Approx(bmacd[19]));

    /** @arg n1 = 3 n2 = 2 n3 = 1*/
    macd = MACD(ind, 3, 2, 1);
    CHECK_EQ(macd.size(), 20);
    CHECK_EQ(macd.discard(), 0);
    bar = macd.getResult(0);
    diff = macd.getResult(1);
    dea = macd.getResult(2);
    ema1 = EMA(ind, 3);
    ema2 = EMA(ind, 2);
    fast = ema1 - ema2;
    slow = EMA(fast, 1);
    bmacd = fast - slow;
    CHECK_EQ(bar.size(), 20);
    CHECK_EQ(diff.size(), 20);
    CHECK_EQ(dea.size(), 20);

    CHECK_EQ(diff[0], doctest::Approx(fast[0]));
    CHECK_EQ(diff[1], doctest::Approx(fast[1]));
    CHECK_EQ(diff[19], doctest::Approx(fast[19]));

    CHECK_EQ(dea[0], doctest::Approx(slow[0]));
    CHECK_EQ(dea[1], doctest::Approx(slow[1]));
    CHECK_EQ(dea[19], doctest::Approx(slow[19]));

    CHECK_EQ(bar[0], doctest::Approx(bmacd[0]));
    CHECK_EQ(bar[1], doctest::Approx(bmacd[1]));
    CHECK_EQ(bar[19], doctest::Approx(bmacd[19]));

    /** @arg n1 = 3 n2 = 5 n3 = 2*/
    macd = MACD(ind, 3, 5, 2);
    CHECK_EQ(macd.size(), 20);
    CHECK_EQ(macd.discard(), 0);
    bar = macd.getResult(0);
    diff = macd.getResult(1);
    dea = macd.getResult(2);
    ema1 = EMA(ind, 3);
    ema2 = EMA(ind, 5);
    fast = ema1 - ema2;
    slow = EMA(fast, 2);
    bmacd = fast - slow;
    CHECK_EQ(bar.size(), 20);
    CHECK_EQ(diff.size(), 20);
    CHECK_EQ(dea.size(), 20);

    CHECK_EQ(diff[0], doctest::Approx(fast[0]));
    CHECK_EQ(diff[1], doctest::Approx(fast[1]));
    CHECK_EQ(diff[19], doctest::Approx(fast[19]));

    CHECK_EQ(dea[0], doctest::Approx(slow[0]));
    CHECK_EQ(dea[1], doctest::Approx(slow[1]));
    CHECK_EQ(dea[19], doctest::Approx(slow[19]));

    CHECK_EQ(bar[0], doctest::Approx(bmacd[0]));
    CHECK_EQ(bar[1], doctest::Approx(bmacd[1]));
    CHECK_EQ(bar[19], doctest::Approx(bmacd[19]));

    /** @arg operator() */
    Indicator expect = MACD(ind, 3, 5, 2);
    Indicator tmp = MACD(3, 5, 2);
    Indicator result = tmp(ind);
    CHECK_EQ(result.size(), expect.size());
    for (size_t i = 0; i < expect.size(); ++i) {
        CHECK_EQ(result.get(i, 0), expect.get(i, 0));
        CHECK_EQ(result.get(i, 1), expect.get(i, 1));
        CHECK_EQ(result.get(i, 2), expect.get(i, 2));
    }
}

/** @par Test points */
TEST_CASE("test_MACD_dyn") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-50));
    // KData kdata = stock.getKData(KQuery(0, Null<size_t>(), KQuery::MIN));
    Indicator c = CLOSE(kdata);
    Indicator expect = MACD(c, 12, 26, 9);
    Indicator result = MACD(c, CVAL(c, 12), CVAL(c, 26), CVAL(c, 9));
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = result.discard(); i < result.size(); i++) {
        CHECK_EQ(expect.get(i, 0), doctest::Approx(result.get(i, 0)));
        CHECK_EQ(expect.get(i, 1), doctest::Approx(result.get(i, 1)));
        CHECK_EQ(expect.get(i, 2), doctest::Approx(result.get(i, 2)));
    }

    result = MACD(c, IndParam(CVAL(c, 12)), IndParam(CVAL(c, 26)), IndParam(CVAL(c, 9)));
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = result.discard(); i < result.size(); i++) {
        CHECK_EQ(expect.get(i, 0), doctest::Approx(result.get(i, 0)));
        CHECK_EQ(expect.get(i, 1), doctest::Approx(result.get(i, 1)));
        CHECK_EQ(expect.get(i, 2), doctest::Approx(result.get(i, 2)));
    }

    /** @arg The dynamic n1/n2/n3 vary per bar: each output must equal MACD of the [0, i] prefix */
    PriceList raw;
    for (int i = 0; i < 14; ++i) {
        raw.push_back(10.0 + i * i * 0.37);
    }
    Indicator src = PRICELIST(raw);
    PriceList v1, v2, v3;
    for (int i = 0; i < 14; ++i) {
        v1.push_back(i % 2 == 0 ? 3.0 : 6.0);
        v2.push_back(i < 7 ? 8.0 : 12.0);
        v3.push_back(i % 3 == 0 ? 2.0 : 4.0);
    }
    result = MACD(src, IndParam(PRICELIST(v1)), IndParam(PRICELIST(v2)), IndParam(PRICELIST(v3)));
    CHECK_EQ(result.size(), src.size());
    CHECK_EQ(result.getResultNumber(), 3);
    for (size_t i = 0; i < src.size(); ++i) {
        Indicator expect_prefix = MACD(SLICE(src, 0, i + 1), int(v1[i]), int(v2[i]), int(v3[i]));
        CHECK_EQ(expect_prefix.get(i, 0), doctest::Approx(result.get(i, 0)));
        CHECK_EQ(expect_prefix.get(i, 1), doctest::Approx(result.get(i, 1)));
        CHECK_EQ(expect_prefix.get(i, 2), doctest::Approx(result.get(i, 2)));
    }

    /** @arg The first valid bar seeds bar/diff/dea to zero */
    CHECK_EQ(result.get(0, 0), 0.0);
    CHECK_EQ(result.get(0, 1), 0.0);
    CHECK_EQ(result.get(0, 2), 0.0);

    /** @arg The dynamic n1/n2/n3 below or equal to 0 yield Null at those bars */
    PriceList bad1, bad2;
    for (int i = 0; i < 14; ++i) {
        bad1.push_back(i < 2 ? 0.0 : 3.0);
        bad2.push_back(i < 2 ? -1.0 : 8.0);
    }
    result = MACD(src, PRICELIST(bad1), PRICELIST(bad2), CVAL(src, 2));
    for (size_t i = 0; i < 2; ++i) {
        CHECK_UNARY(std::isnan(result.get(i, 0)));
        CHECK_UNARY(std::isnan(result.get(i, 1)));
        CHECK_UNARY(std::isnan(result.get(i, 2)));
    }
    for (size_t i = 2; i < src.size(); ++i) {
        Indicator expect_prefix = MACD(SLICE(src, 0, i + 1), 3, 8, 2);
        CHECK_EQ(expect_prefix.get(i, 0), doctest::Approx(result.get(i, 0)));
        CHECK_EQ(expect_prefix.get(i, 1), doctest::Approx(result.get(i, 1)));
        CHECK_EQ(expect_prefix.get(i, 2), doctest::Approx(result.get(i, 2)));
    }
}

/**
 * @par Test points
 * Reusing one Indicator instance with consecutive setContext (tail extended) must match a full
 * calculation.
 *
 * Background: MACD used to declare incremental support, but resuming DIF needs ema1/ema2
 * separately while its outputs only keep their difference, so the state could not be resumed and
 * the tail diverged. Incremental calculate is now disabled; this case guards that re-enabling it
 * means carrying the EMA state, not inferring it from the results.
 */
/** @par Test points: ST-N15 — leading non-finite yields Null (never a fake 0/0/0 line) until
 * the first finite observation seeds the internal EMAs; dynamic path must match the static prefix
 */
TEST_CASE("test_MACD_leading_nan_null") {
    price_t nan = Null<price_t>();
    Indicator data = PRICELIST({1.0, nan, nan, nan, 5.0, 6.0});
    Indicator r = MACD(data, 2, 3, 2);
    // bars 2,3 have no valid post-seed observation yet: must be Null (not fake 0/0/0)
    CHECK_UNARY(std::isnan(r[2]));
    CHECK_UNARY(std::isnan(r.get(2, 1)));
    CHECK_UNARY(std::isnan(r.get(3, 2)));
    // bar 4 (first finite after gap) resumes recursion, outputs finite
    CHECK_UNARY(std::isfinite(r[4]));
    CHECK_UNARY(std::isfinite(r.get(4, 1)));
    CHECK_UNARY(std::isfinite(r.get(4, 2)));

    /** @arg Dynamic path: every bar must equal the static MACD of the [0, i] prefix (bitwise
     * identical, including the leading-NaN Null region). Regression guard for _dyn_one_circle
     * bailing out early on the first non-finite bar. */
    Indicator dyn =
      MACD(data, IndParam(CVAL(data, 2)), IndParam(CVAL(data, 3)), IndParam(CVAL(data, 2)));
    auto same = [](price_t a, price_t b) {
        if (std::isnan(a) || std::isnan(b))
            return std::isnan(a) && std::isnan(b);
        return std::abs(a - b) < 1e-9 * std::max(1.0, std::abs(a));
    };
    for (size_t i = 0; i < data.size(); ++i) {
        Indicator prefix = MACD(SLICE(data, 0, i + 1), 2, 3, 2);
        for (size_t col = 0; col < 3; ++col) {
            CHECK_UNARY(same(prefix.get(i, col), dyn.get(i, col)));
        }
    }
    // the resumption bar must be finite in the dynamic path too (not Null from early return)
    CHECK_UNARY(std::isfinite(dyn.get(4, 0)));
    CHECK_UNARY(std::isfinite(dyn.get(4, 1)));
    CHECK_UNARY(std::isfinite(dyn.get(4, 2)));
}

/** @par Test points: NaN input holds the previous three outputs instead of permanently poisoning
 * the tail */
TEST_CASE("test_MACD_nan_hold") {
    price_t nan = Null<price_t>();
    Indicator data = PRICELIST({1.0, 2.0, 3.0, nan, 5.0, 6.0});
    Indicator r = MACD(data, 2, 3, 2);
    /** @arg i=3 (NaN input) holds all three outputs at their prior values */
    CHECK_EQ(r[3], r[2]);
    CHECK_EQ(r.get(3, 1), r.get(2, 1));
    CHECK_EQ(r.get(3, 2), r.get(2, 2));
    /** @arg after the NaN, the recursion resumes from the held states (all finite) */
    for (size_t i = 4; i < 6; i++) {
        CHECK_UNARY(std::isfinite(r[i]));
        CHECK_UNARY(std::isfinite(r.get(i, 1)));
        CHECK_UNARY(std::isfinite(r.get(i, 2)));
    }
}

TEST_CASE("test_MACD_increment_equivalence") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData k_full = stock.getKData(KQuery(-50));
    KData k_partial = stock.getKData(KQuery(-50, -30));

    Indicator ind_full = MACD(CLOSE(), 12, 26, 9);
    ind_full.setContext(k_full);

    Indicator ind_inc = MACD(CLOSE(), 12, 26, 9);
    ind_inc.setContext(k_partial);  // cache m_old_context
    ind_inc.setContext(k_full);     // tail extended: must fall back to a full recalculation

    CHECK_EQ(ind_full.size(), ind_inc.size());
    CHECK_EQ(ind_full.discard(), ind_inc.discard());
    for (size_t i = 0; i < ind_full.size(); ++i) {
        for (size_t r = 0; r < 3; ++r) {
            double a = ind_full.get(i, r);
            double b = ind_inc.get(i, r);
            if (std::isnan(a) && std::isnan(b)) {
                continue;
            }
            CHECK_EQ(b, doctest::Approx(a).epsilon(0.0001));
        }
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_MACD_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/MACD.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = MACD(CLOSE(kdata));
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

    CHECK_EQ(ma1.size(), ma2.size());
    CHECK_EQ(ma1.discard(), ma2.discard());
    CHECK_EQ(ma1.getResultNumber(), ma2.getResultNumber());
    for (size_t i = 0; i < ma1.size(); ++i) {
        CHECK_EQ(ma1.get(i, 0), doctest::Approx(ma2.get(i, 0)));
        CHECK_EQ(ma1.get(i, 1), doctest::Approx(ma2.get(i, 1)));
        CHECK_EQ(ma1.get(i, 2), doctest::Approx(ma2.get(i, 2)));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
