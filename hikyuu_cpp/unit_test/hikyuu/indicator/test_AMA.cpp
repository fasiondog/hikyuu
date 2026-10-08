/*
 * test_AMA.cpp
 *
 *  Created on: 2013-4-10
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/AMA.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/SLICE.h>

using namespace hku;

/**
 * @defgroup test_indicator_AMA test_indicator_AMA
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_AMA") {
    Indicator result;

    PriceList d;
    d.push_back(6063);
    d.push_back(6041);
    d.push_back(6065);
    d.push_back(6078);
    d.push_back(6114);
    d.push_back(6121);
    d.push_back(6106);
    d.push_back(6101);
    d.push_back(6166);
    d.push_back(6169);
    d.push_back(6195);
    d.push_back(6222);
    d.push_back(6186);
    d.push_back(6214);
    d.push_back(6185);
    d.push_back(6209);
    d.push_back(6221);
    d.push_back(6278);
    d.push_back(6326);
    d.push_back(6347);

    Indicator ind = PRICELIST(d);
    CHECK_EQ(ind.size(), 20);
    result = AMA(ind, 10, 2, 30);
    CHECK_EQ(result.getParam<int>("n"), 10);
    CHECK_EQ(result.getParam<int>("fast_n"), 2);
    CHECK_EQ(result.getParam<int>("slow_n"), 30);
    CHECK_EQ(result.size(), 20);
    CHECK_EQ(result.empty(), false);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.getResultNumber(), 2);
    CHECK_EQ(result[0], 6063);
    CHECK_EQ(result[9], doctest::Approx(6103.6781));
    CHECK_EQ(result[10], doctest::Approx(6120.760197));
    /** @arg rolling-window phase values follow the corrected n-difference window */
    CHECK_EQ(result[18], doctest::Approx(6214.068219657));
    CHECK_EQ(result[19], doctest::Approx(6236.688732964));

    CHECK_EQ(result.get(0, 1), 1.0);
    CHECK_EQ(result.get(9, 1), doctest::Approx(0.557895));
    CHECK_EQ(result.get(10, 1), doctest::Approx(0.611111));
    CHECK_EQ(result.get(11, 1), doctest::Approx(0.819004525));
    CHECK_EQ(result.get(18, 1), doctest::Approx(0.551724138));
    CHECK_EQ(result.get(19, 1), doctest::Approx(0.577922078));

    /** @arg operator() */
    Indicator ama = AMA(10, 2, 30);
    CHECK_EQ(ama.size(), 0);
    Indicator expect = AMA(ind, 10, 2, 30);
    result = ama(ind);
    CHECK_EQ(result.size(), expect.size());
    CHECK_EQ(result.getResultNumber(), expect.getResultNumber());
    CHECK_EQ(result.discard(), expect.discard());
    for (size_t i = 0; i < expect.size(); ++i) {
        CHECK_EQ(result[i], expect[i]);
        CHECK_EQ(result.get(i, 1), expect.get(i, 1));
    }

    /** The incremental calculation */
    Stock stk = getStock("sh000001");
    auto k1 = stk.getKData(KQuery(0, 20));
    auto k2 = stk.getKData(KQuery(0, 21));
    ama = AMA(CLOSE(), 1, 1, 1)(k1);
    ama = ama(k2);
    auto k3 = stk.getKData(KQuery(19, 25));
    ama = ama(k3);
}

/** @par Test points */
TEST_CASE("test_AMA_er_lower_clamp") {
    // a strictly monotonic series can push the warm-up phase er a hair below -1 through float
    // summation (the triangle inequality bounds it at -1 mathematically); the ER column must stay
    // within [-1, 1] exactly like the rolling phase does, and c must not exceed 1
    PriceList d;
    d.push_back(108.05521983981556);
    d.push_back(10.40404090056498);
    d.push_back(-100.85878258625682);
    d.push_back(-240.14421907974634);
    d.push_back(-504.1348659350719);
    Indicator result = AMA(PRICELIST(d), 4, 2, 30);
    CHECK_EQ(result.size(), 5);

    /** @arg bar 4 lies in the warm-up phase; its float er is -1.0000000000000002 unclamped */
    CHECK_UNARY(result.get(4, 1) >= -1.0);
    CHECK_UNARY(result.get(4, 1) <= 1.0);
    /** @arg every warm-up bar keeps ER within [-1, 1] */
    for (size_t i = 1; i < result.size(); ++i) {
        CHECK_UNARY(result.get(i, 1) >= -1.0);
        CHECK_UNARY(result.get(i, 1) <= 1.0);
    }
}

/** @par Test points */
TEST_CASE("test_AMA_dyn") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-50));
    // KData kdata = stock.getKData(KQuery(0, Null<size_t>(), KQuery::MIN));
    Indicator c = CLOSE(kdata);
    Indicator expect = AMA(c, 10, 2, 30);
    Indicator result = AMA(c, CVAL(c, 10), CVAL(c, 2), CVAL(c, 30));
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = result.discard(); i < result.size(); i++) {
        CHECK_EQ(expect.get(i, 0), doctest::Approx(result.get(i, 0)));
        CHECK_EQ(expect.get(i, 1), doctest::Approx(result.get(i, 1)));
    }

    result = AMA(c, IndParam(CVAL(c, 10)), IndParam(CVAL(c, 2)), IndParam(CVAL(c, 30)));
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = result.discard(); i < result.size(); i++) {
        CHECK_EQ(expect.get(i, 0), doctest::Approx(result.get(i, 0)));
        CHECK_EQ(expect.get(i, 1), doctest::Approx(result.get(i, 1)));
    }

    /** @arg The dynamic n varies per bar: ama/er at bar i must equal AMA of the [0, i] prefix */
    PriceList raw;
    for (int i = 0; i < 16; ++i) {
        raw.push_back(10.0 + i * i * 0.37);
    }
    Indicator src = PRICELIST(raw);
    PriceList n_values, fast_values, slow_values;
    for (int i = 0; i < 16; ++i) {
        n_values.push_back(i < 5 ? 2.0 : (i < 10 ? 6.0 : 3.0));
        fast_values.push_back(i % 2 == 0 ? 2.0 : 3.0);
        slow_values.push_back(i < 8 ? 5.0 : 10.0);
    }
    result = AMA(src, IndParam(PRICELIST(n_values)), IndParam(PRICELIST(fast_values)),
                 IndParam(PRICELIST(slow_values)));
    CHECK_EQ(result.size(), src.size());
    CHECK_EQ(result.getResultNumber(), 2);
    for (size_t i = 0; i < src.size(); ++i) {
        Indicator expect_prefix =
          AMA(SLICE(src, 0, i + 1), int(n_values[i]), int(fast_values[i]), int(slow_values[i]));
        CHECK_EQ(expect_prefix.get(i, 0), doctest::Approx(result.get(i, 0)));
        CHECK_EQ(expect_prefix.get(i, 1), doctest::Approx(result.get(i, 1)));
    }

    /** @arg The invalid dynamic params are clamped (n<1 -> 1, fast_n<0 -> 0, slow_n<0 -> 0) */
    PriceList bad_n, bad_fast, bad_slow;
    for (int i = 0; i < 16; ++i) {
        bad_n.push_back(i < 4 ? 0.0 : 2.0);
        bad_fast.push_back(-3.0);
        bad_slow.push_back(i < 4 ? -1.0 : 4.0);
    }
    result = AMA(src, IndParam(PRICELIST(bad_n)), IndParam(PRICELIST(bad_fast)),
                 IndParam(PRICELIST(bad_slow)));
    for (size_t i = 0; i < src.size(); ++i) {
        Indicator expect_prefix = AMA(SLICE(src, 0, i + 1), i < 4 ? 1 : 2, 0, i < 4 ? 0 : 4);
        CHECK_EQ(expect_prefix.get(i, 0), doctest::Approx(result.get(i, 0)));
        CHECK_EQ(expect_prefix.get(i, 1), doctest::Approx(result.get(i, 1)));
    }
}

/** @par Test points */
TEST_CASE("test_AMA_increment_equivalence") {
    // After fixing the sliding n-difference window, the incremental path must match a full
    // calculation bar by bar on both result sets (AMA and ER)
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData k_full = stock.getKData(KQuery(0, 30));
    CHECK_EQ(k_full.size(), 30);

    Indicator expect = AMA(CLOSE(), 10, 2, 30);
    expect.setContext(k_full);

    auto check_with_history = [&](size_t history) {
        Indicator got = AMA(CLOSE(), 10, 2, 30);
        got.setContext(stock.getKData(KQuery(0, history)));  // cache m_old_context
        got.setContext(k_full);                              // extended at the tail
        CHECK_EQ(got.size(), expect.size());
        CHECK_EQ(got.discard(), expect.discard());
        for (size_t i = 0; i < expect.size(); ++i) {
            for (size_t r = 0; r < 2; ++r) {
                double a = expect.get(i, r);
                double b = got.get(i, r);
                if (std::isnan(a) && std::isnan(b)) {
                    continue;
                }
                CHECK_EQ(b, doctest::Approx(a));
            }
        }
    };

    /** @arg the boundary falls inside the rolling phase (start_pos > n + 1) */
    check_with_history(20);

    /** @arg a small tail extension */
    check_with_history(29);

    /** @arg the phase-1 / rolling switch point (start_pos == n + 1) */
    check_with_history(12);

    /** @arg below the increment threshold: falls back to a full recalculation, still equal */
    check_with_history(11);

    /** @arg a discard-carrying input close to the discard edge: the incremental path must resume
     *  with the truncated warm-up window of the full computation instead of seeding from the NaN
     *  region before discard */
    {
        Indicator expect_ma = AMA(MA(CLOSE(), 5), 10, 2, 30);
        expect_ma.setContext(k_full);
        Indicator got = AMA(MA(CLOSE(), 5), 10, 2, 30);
        got.setContext(stock.getKData(KQuery(0, 13)));  // start_pos=12 < discard(4)+n+1
        got.setContext(k_full);
        CHECK_EQ(got.size(), expect_ma.size());
        CHECK_EQ(got.discard(), expect_ma.discard());
        for (size_t i = 0; i < expect_ma.size(); ++i) {
            for (size_t r = 0; r < 2; ++r) {
                double a = expect_ma.get(i, r);
                double b = got.get(i, r);
                if (std::isnan(a) && std::isnan(b)) {
                    continue;
                }
                CHECK_EQ(b, doctest::Approx(a));
            }
        }
    }

    /** @arg input discard >= n: the extension boundary lands at start_pos == start (no seed in the
     *  old buffer); must restart the warm-up exactly like the full computation */
    {
        Indicator expect_ma = AMA(MA(CLOSE(), 12), 10, 2, 30);
        expect_ma.setContext(k_full);
        Indicator got = AMA(MA(CLOSE(), 12), 10, 2, 30);
        got.setContext(stock.getKData(KQuery(0, 12)));  // start_pos = 11 == start(11)
        got.setContext(k_full);
        CHECK_EQ(got.size(), expect_ma.size());
        CHECK_EQ(got.discard(), expect_ma.discard());
        for (size_t i = 0; i < expect_ma.size(); ++i) {
            for (size_t r = 0; r < 2; ++r) {
                double a = expect_ma.get(i, r);
                double b = got.get(i, r);
                if (std::isnan(a) && std::isnan(b)) {
                    continue;
                }
                CHECK_EQ(b, doctest::Approx(a));
            }
        }
    }

    /** @arg the input carries no valid bar in the new context (discard >= total): all NaN */
    {
        Indicator expect_nan = AMA(MA(CLOSE(), 100), 10, 2, 30);
        expect_nan.setContext(k_full);
        Indicator got = AMA(MA(CLOSE(), 100), 10, 2, 30);
        got.setContext(stock.getKData(KQuery(0, 12)));
        got.setContext(k_full);
        CHECK_EQ(got.discard(), 30);
        CHECK_EQ(expect_nan.discard(), 30);
        for (size_t i = 0; i < got.size(); ++i) {
            CHECK_UNARY(std::isnan(got.get(i, 0)));
            CHECK_UNARY(std::isnan(got.get(i, 1)));
        }
    }

    /** @arg chained extensions (an increment on top of a previous increment) */
    {
        Indicator expect_ma = AMA(CLOSE(), 10, 2, 30);
        expect_ma.setContext(k_full);
        Indicator got = AMA(CLOSE(), 10, 2, 30);
        got.setContext(stock.getKData(KQuery(0, 15)));
        got.setContext(stock.getKData(KQuery(0, 22)));
        got.setContext(k_full);
        CHECK_EQ(got.size(), expect_ma.size());
        CHECK_EQ(got.discard(), expect_ma.discard());
        for (size_t i = 0; i < expect_ma.size(); ++i) {
            for (size_t r = 0; r < 2; ++r) {
                double a = expect_ma.get(i, r);
                double b = got.get(i, r);
                if (std::isnan(a) && std::isnan(b)) {
                    continue;
                }
                CHECK_EQ(b, doctest::Approx(a));
            }
        }
    }
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_AMA_benchmark") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0));
    Indicator c = kdata.close();
    int cycle = 1000;  // Test loop count

    {
        BENCHMARK_TIME_MSG(test_AMA_benchmark, cycle, fmt::format("data len: {}", c.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator ind = AMA();
            Indicator result = ind(c);
        }
    }
}
#endif

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_AMA_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/AMA.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = AMA(CLOSE(kdata), 10);
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
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
