/*
 * test_SMA.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-16
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/SMA.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/SLICE.h>

using namespace hku;

/**
 * @defgroup test_indicator_SMA test_indicator_SMA
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_SMA") {
    Indicator result;
    PriceList a;
    Indicator data = PRICELIST(a);

    /** @arg The input indicator length is 0 */
    result = SMA(data);
    CHECK_EQ(result.name(), "SMA");
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), 0);

    /** @arg The input indicator length is 1, n = 2 and m=2 */
    a.push_back(10);
    data = PRICELIST(a);
    result = SMA(data, 2, 2);
    CHECK_EQ(result.name(), "SMA");
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], 10);

    /** @arg The input indicator length is 1, n = 1 and m=2 */
    a.clear();
    a.push_back(10);
    data = PRICELIST(a);
    result = SMA(data, 1, 2);
    CHECK_EQ(result.name(), "SMA");
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], 10);

    /** @arg The input indicator length is 10, n = 1 and m=1 */
    a.clear();
    for (int i = 0; i < 10; ++i) {
        a.push_back(i / 10.0);
    }
    data = PRICELIST(a);
    result = SMA(data, 1, 1);
    CHECK_EQ(result.name(), "SMA");
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), data.size());
    for (size_t i = result.discard(); i < data.size(); i++) {
        CHECK_EQ(result[i], data[i]);
    }

    /** @arg The input indicator length is 1, n = 4 and m=2 */
    result = SMA(data, 4, 2);
    CHECK_EQ(result.name(), "SMA");
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result[0], 0);
    CHECK_EQ(result[1], doctest::Approx(0.05));
    CHECK_EQ(result[5], doctest::Approx(0.4031).epsilon(0.01));
    CHECK_EQ(result[9], doctest::Approx(0.8002).epsilon(0.01));
}

/** @par Test points */
TEST_CASE("test_SMA_dyn") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-33));
    Indicator c = CLOSE(kdata);
    Indicator expect = SMA(c, 22, 2.0);
    Indicator result = SMA(c, CVAL(c, 22), CVAL(c, 2.0));
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = result.discard(); i < result.size(); i++) {
        CHECK_EQ(expect.get(i, 0), doctest::Approx(result.get(i, 0)));
    }

    result = SMA(c, IndParam(CVAL(c, 22)), IndParam(CVAL(c, 2.0)));
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = result.discard(); i < result.size(); i++) {
        CHECK_EQ(expect.get(i, 0), doctest::Approx(result.get(i, 0)));
    }

    /** @arg Mixed dynamic and static params (ISS-035: getParam<int>("m") threw bad_cast) */
    result = SMA(c, CVAL(c, 22), 2.0);
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = result.discard(); i < result.size(); i++) {
        CHECK_EQ(expect.get(i, 0), doctest::Approx(result.get(i, 0)));
    }

    /** @arg The dynamic n varies per bar: value at bar i must equal SMA of the [0, i] prefix */
    PriceList raw;
    for (int i = 0; i < 12; ++i) {
        raw.push_back(10.0 + i * i * 0.37);
    }
    Indicator src = PRICELIST(raw);
    PriceList n_values;
    for (int i = 0; i < 12; ++i) {
        n_values.push_back(i < 4 ? 3.0 : (i < 8 ? 5.0 : 2.0));
    }
    result = SMA(src, PRICELIST(n_values), CVAL(src, 2.0));
    CHECK_EQ(result.size(), src.size());
    for (size_t i = 0; i < src.size(); ++i) {
        Indicator expect_prefix = SMA(SLICE(src, 0, i + 1), int(n_values[i]), 2.0);
        CHECK_EQ(expect_prefix[expect_prefix.size() - 1], doctest::Approx(result[i]));
    }

    /** @arg The dynamic n below 1 yields Null at those bars */
    PriceList invalid_n;
    for (int i = 0; i < 12; ++i) {
        invalid_n.push_back(i < 3 ? 0.0 : 2.0);
    }
    result = SMA(src, PRICELIST(invalid_n), CVAL(src, 2.0));
    CHECK_EQ(result.size(), src.size());
    for (size_t i = 0; i < 3; ++i) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = 3; i < src.size(); ++i) {
        Indicator expect_prefix = SMA(SLICE(src, 0, i + 1), 2, 2.0);
        CHECK_EQ(expect_prefix[expect_prefix.size() - 1], doctest::Approx(result[i]));
    }
}

/** @par Test points: reusing one indicator across setContext extension must match full calc */
TEST_CASE("test_SMA_increment_equivalence") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm.getStock("sh600000");
    KData k_full = stk.getKData(KQuery(0, 30));
    KData k_partial = stk.getKData(KQuery(0, 15));

    Indicator expect = SMA(22, 2.0);
    expect.setContext(k_full);

    Indicator got = SMA(22, 2.0);
    got.setContext(k_partial);
    got.setContext(k_full);

    CHECK_EQ(got.size(), expect.size());
    CHECK_EQ(got.discard(), expect.discard());
    for (size_t i = got.discard(); i < got.size(); ++i) {
        CHECK_EQ(got[i], doctest::Approx(expect[i]).epsilon(0.0001));
    }

    // Chain: partial -> shorter -> full, still matches a single full calculation
    Indicator chained = SMA(22, 2.0);
    chained.setContext(stk.getKData(KQuery(0, 10)));
    chained.setContext(k_partial);
    chained.setContext(k_full);
    CHECK_EQ(chained.size(), expect.size());
    for (size_t i = chained.discard(); i < chained.size(); ++i) {
        CHECK_EQ(chained[i], doctest::Approx(expect[i]).epsilon(0.0001));
    }
}

/** @par Test points: NaN input at bar i holds the previous SMA (no permanent contamination) */
TEST_CASE("test_SMA_nan_hold") {
    price_t nan = Null<price_t>();
    Indicator data = PRICELIST({1.0, 2.0, nan, 4.0, 5.0});
    Indicator ret = SMA(data, 3, 1.0);
    // p = n - m = 2; ret[0]=1.0, ret[1] = (1*2 + 2*1)/3 = 4/3
    // ret[2]: src NaN -> hold -> 4/3
    // ret[3]: (1*4 + 2*4/3)/3 = 20/9
    // ret[4]: (1*5 + 2*20/9)/3 = 85/27
    CHECK_EQ(ret[0], 1.0);
    CHECK_EQ(ret[1], doctest::Approx(4.0 / 3.0));
    CHECK_EQ(ret[2], doctest::Approx(4.0 / 3.0));
    CHECK_EQ(ret[3], doctest::Approx(20.0 / 9.0));
    CHECK_EQ(ret[4], doctest::Approx(85.0 / 27.0));

    /** @arg Inf input is treated as non-finite and also holds (AGENTS.md contract) */
    price_t inf_v = std::numeric_limits<price_t>::infinity();
    Indicator data2 = PRICELIST({1.0, inf_v, 4.0});
    Indicator ret2 = SMA(data2, 3, 1.0);
    CHECK_EQ(ret2[1], 1.0);  // hold on Inf
    CHECK_UNARY(std::isfinite(ret2[2]));
    CHECK_EQ(ret2[2], doctest::Approx((1.0 * 4.0 + 2.0 * 1.0) / 3.0));
}

/** @par Test points */
TEST_CASE("test_SMA_param_domain") {
    PriceList a;
    for (int i = 0; i < 10; ++i) {
        a.push_back(i);
    }
    Indicator data = PRICELIST(a);

    /** @arg m below 1 is clamped to 1 */
    Indicator expect = SMA(data, 5, 1);
    Indicator result = SMA(data, 5, 0);
    for (size_t i = 0; i < data.size(); i++) {
        CHECK_EQ(result[i], doctest::Approx(expect[i]));
    }

    result = SMA(data, 5, -1);
    for (size_t i = 0; i < data.size(); i++) {
        CHECK_EQ(result[i], doctest::Approx(expect[i]));
    }

    /** @arg m above n is clamped to n, where the recursion is a pass-through */
    result = SMA(data, 5, 10);
    for (size_t i = 0; i < data.size(); i++) {
        CHECK_EQ(result[i], doctest::Approx(data[i]));
    }
}

/** @par Test points */
TEST_CASE("test_SMA_dyn_param_domain") {
    PriceList raw;
    for (int i = 0; i < 12; ++i) {
        raw.push_back(10.0 + i * i * 0.37);
    }
    Indicator src = PRICELIST(raw);

    PriceList n_values, m_values;
    for (int i = 0; i < 12; ++i) {
        n_values.push_back(5.0);
        m_values.push_back(i < 3 ? 0.0 : 2.0);
    }

    /** @arg a per-bar m below 1 is clamped to 1 as well */
    Indicator result = SMA(src, PRICELIST(n_values), PRICELIST(m_values));
    for (size_t i = 0; i < src.size(); ++i) {
        Indicator expect_prefix = SMA(SLICE(src, 0, i + 1), 5, i < 3 ? 1.0 : 2.0);
        CHECK_EQ(expect_prefix[expect_prefix.size() - 1], doctest::Approx(result[i]));
    }
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_SMA_benchmark") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0));
    Indicator c = kdata.close();
    int cycle = 1000;  // Test loop count

    {
        BENCHMARK_TIME_MSG(test_SMA_benchmark, cycle, fmt::format("data len: {}", c.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator ind = SMA();
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
TEST_CASE("test_SMA_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/SMA.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = SMA(CLOSE(kdata));
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

    CHECK_EQ(x2.name(), "SMA");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]).epsilon(0.00001));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
