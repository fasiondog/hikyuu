/*
 * test_SAFTYLOSS.cpp
 *
 *  Created on: 2013-4-12
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/SAFTYLOSS.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/SLICE.h>

using namespace hku;

/**
 * @defgroup test_indicator_SAFTYLOSS test_indicator_SAFTYLOSS
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_SAFTYLOSS") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KQuery query;
    KData kdata;
    Indicator close, result;

    /** @arg The source data is empty */
    result = SAFTYLOSS(CLOSE(kdata), 2, 1);
    CHECK_EQ(result.empty(), true);
    CHECK_EQ(result.size(), 0);
    CHECK_EQ(result.discard(), 0);

    /** @arg The parameters n1 and n2 are invalid */
    query = KQuery(0, 20);
    kdata = stock.getKData(query);
    close = CLOSE(kdata);
    CHECK_THROWS_AS(SAFTYLOSS(close, 1, 1), std::exception);
    CHECK_THROWS_AS(SAFTYLOSS(close, 0, 0), std::exception);
    CHECK_THROWS_AS(SAFTYLOSS(close, 2, 0), std::exception);

    /** @arg The normal parameters */
    result = SAFTYLOSS(close, 2, 1, 1.0);
    CHECK_EQ(result.empty(), false);
    CHECK_EQ(result.size(), close.size());
    CHECK_EQ(result.discard(), 1);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_EQ(result[1], doctest::Approx(27.67));
    CHECK_EQ(result[2], doctest::Approx(28.05));
    CHECK_EQ(result[3], doctest::Approx(27.45));
    CHECK_LT(std::fabs(result[19] - 25.54), 0.0001);

    result = SAFTYLOSS(close, 2, 2, 1.0);
    CHECK_EQ(result.empty(), false);
    CHECK_EQ(result.size(), close.size());
    CHECK_EQ(result.discard(), 2);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_UNARY(std::isnan(result[1]));
    CHECK_EQ(result[2], doctest::Approx(28.05));
    CHECK_EQ(result[3], doctest::Approx(28.05));
    CHECK_EQ(result[4], doctest::Approx(27.45));
    CHECK_LT(std::fabs(result[19] - 25.54), 0.0001);

    result = SAFTYLOSS(close, 3, 2, 2.0);
    CHECK_EQ(result.empty(), false);
    CHECK_EQ(result.size(), close.size());
    CHECK_EQ(result.discard(), 3);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_UNARY(std::isnan(result[1]));
    CHECK_UNARY(std::isnan(result[2]));
    CHECK_LT(std::fabs(result[3] - 27.97), 0.0001);
    CHECK_LT(std::fabs(result[4] - 27.15), 0.0001);
    CHECK_LT(std::fabs(result[5] - 25.05), 0.0001);
    CHECK_LT(std::fabs(result[19] - 24.84), 0.0001);

    result = SAFTYLOSS(close, 3, 3, 2.0);
    CHECK_EQ(result.empty(), false);
    CHECK_EQ(result.size(), close.size());
    CHECK_EQ(result.discard(), 4);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_UNARY(std::isnan(result[1]));
    CHECK_UNARY(std::isnan(result[2]));
    CHECK_UNARY(std::isnan(result[3]));
    CHECK_LT(std::fabs(result[4] - 27.97), 0.0001);
    CHECK_LT(std::fabs(result[5] - 27.15), 0.0001);
    CHECK_LT(std::fabs(result[6] - 26.7), 0.0001);
    CHECK_LT(std::fabs(result[19] - 25.68), 0.0001);

    result = SAFTYLOSS(close, 10, 3, 2.0);
    CHECK_EQ(result.empty(), false);
    CHECK_EQ(result.size(), close.size());
    CHECK_EQ(result.discard(), 11);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_UNARY(std::isnan(result[1]));
    CHECK_UNARY(std::isnan(result[2]));
    CHECK_UNARY(std::isnan(result[10]));
    CHECK_LT(std::fabs(result[11] - 25.7486), 0.0001);
    CHECK_LT(std::fabs(result[12] - 25.79), 0.0001);
    CHECK_LT(std::fabs(result[13] - 26.03), 0.0001);
    CHECK_LT(std::fabs(result[19] - 26.105), 0.0001);

    /** @arg operator() */
    Indicator ind = SAFTYLOSS(10, 3, 2.0);
    CHECK_EQ(ind.size(), 0);
    result = ind(close);
    Indicator expect = SAFTYLOSS(close, 10, 3, 2.0);
    CHECK_EQ(result.size(), expect.size());
    CHECK_UNARY(result.size() != 0);
    CHECK_EQ(result.discard(), expect.discard());
    for (size_t i = result.discard(); i < expect.size(); ++i) {
        CHECK_EQ(result[i], expect[i]);
    }
}

/** @par Test points */
TEST_CASE("test_SAFTYLOSS_dyn") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-50));
    // KData kdata = stock.getKData(KQuery(0, Null<size_t>(), KQuery::MIN));
    Indicator c = CLOSE(kdata);
    Indicator expect = SAFTYLOSS(c, 10, 3, 2.0);
    Indicator result = SAFTYLOSS(c, CVAL(c, 10), CVAL(c, 3), CVAL(c, 2.0));
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = result.discard(); i < result.size(); i++) {
        CHECK_EQ(expect.get(i, 0), doctest::Approx(result.get(i, 0)));
    }

    result = SAFTYLOSS(c, IndParam(CVAL(c, 10)), IndParam(CVAL(c, 3)), IndParam(CVAL(c, 2.0)));
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = result.discard(); i < result.size(); i++) {
        CHECK_EQ(expect.get(i, 0), doctest::Approx(result.get(i, 0)));
    }

    /** @arg Mixed dynamic and static params (ISS-035: getParam<int>("p") threw bad_cast) */
    result = SAFTYLOSS(c, CVAL(c, 10), CVAL(c, 3), 2.0);
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = result.discard(); i < result.size(); i++) {
        CHECK_EQ(expect.get(i, 0), doctest::Approx(result.get(i, 0)));
    }

    /** @arg The dynamic n1/n2/p vary per bar: value at bar i must equal SAFTYLOSS of the prefix,
     *  including the leading Null region before n1 + n2 - 2 */
    PriceList raw;
    for (int i = 0; i < 16; ++i) {
        raw.push_back(10.0 + i * i * 0.37);
    }
    Indicator src = PRICELIST(raw);
    PriceList v1, v2, v3;
    for (int i = 0; i < 16; ++i) {
        v1.push_back(i < 6 ? 2.0 : 4.0);
        v2.push_back(i % 2 == 0 ? 2.0 : 3.0);
        v3.push_back(i < 8 ? 1.0 : 2.0);
    }
    result = SAFTYLOSS(src, PRICELIST(v1), PRICELIST(v2), PRICELIST(v3));
    CHECK_EQ(result.size(), src.size());
    for (size_t i = 0; i < src.size(); ++i) {
        Indicator expect_prefix = SAFTYLOSS(SLICE(src, 0, i + 1), int(v1[i]), int(v2[i]), v3[i]);
        if (std::isnan(expect_prefix[expect_prefix.size() - 1])) {
            CHECK_UNARY(std::isnan(result[i]));
        } else {
            CHECK_EQ(expect_prefix[expect_prefix.size() - 1], doctest::Approx(result[i]));
        }
    }

    /** @arg The dynamic n1 below 2 yields Null at those bars */
    PriceList bad1, bad2;
    for (int i = 0; i < 16; ++i) {
        bad1.push_back(i < 3 ? 1.0 : 3.0);
        bad2.push_back(i == 4 ? 1.0 : 2.0);
    }
    result = SAFTYLOSS(src, PRICELIST(bad1), PRICELIST(bad2), CVAL(src, 2.0));
    for (size_t i = 0; i < 3; ++i) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    /** @arg a dynamic n2 of 1 is a window of one bar, calculated like the static path does */
    Indicator n2_one = SAFTYLOSS(SLICE(src, 0, 5), 3, 1, 2.0);
    REQUIRE_UNARY(!std::isnan(n2_one[n2_one.size() - 1]));
    CHECK_EQ(n2_one[n2_one.size() - 1], doctest::Approx(result[4]));
    for (size_t i = 5; i < src.size(); ++i) {
        Indicator expect_prefix = SAFTYLOSS(SLICE(src, 0, i + 1), 3, 2, 2.0);
        CHECK_EQ(expect_prefix[expect_prefix.size() - 1], doctest::Approx(result[i]));
    }

    /** @arg a whole dynamic n2 of 1 matches the static path, instead of a null series */
    Indicator dyn_one = SAFTYLOSS(src, CVAL(src, 3), CVAL(src, 1), CVAL(src, 2.0));
    Indicator static_one = SAFTYLOSS(src, 3, 1, 2.0);
    CHECK_EQ(dyn_one.size(), static_one.size());
    CHECK_EQ(dyn_one.discard(), static_one.discard());
    for (size_t i = dyn_one.discard(); i < dyn_one.size(); ++i) {
        CHECK_EQ(dyn_one[i], doctest::Approx(static_one[i]));
    }
}

/**
 * @par Test points
 * Incremental calculate must match a full calculation, including a first context that was fully
 * discarded.
 *
 * Background: ISaftyLoss declared incremental support but not min_increment_start(), so the
 * framework could only raise start_pos with the previous m_discard. When the first context was
 * shorter than n1 + n2 - 2, that m_discard degraded to its own size and left start_pos below the
 * real precondition, so the inner loops read src[-1] or silently wrote 0 into the range that must
 * stay NaN.
 */
TEST_CASE("test_SAFTYLOSS_increment_equivalence") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData k_full = stock.getKData(KQuery(0, 30));
    CHECK_EQ(k_full.size(), 30);

    Indicator expect = SAFTYLOSS(CLOSE(), 10, 3, 2.0);
    expect.setContext(k_full);

    auto check_with_history = [&](size_t history) {
        Indicator got = SAFTYLOSS(CLOSE(), 10, 3, 2.0);
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

    /** @arg The first context was fully discarded, so start_pos fell below n1 + n2 - 2 */
    check_with_history(5);

    /** @arg The first context already had valid results, the usual incremental path */
    check_with_history(20);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_SAFTYLOSS_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/SAFTYLOSS.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = SAFTYLOSS(CLOSE(kdata));
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
    for (size_t i = ma1.discard(); i < ma1.size(); ++i) {
        CHECK_EQ(ma1[i], doctest::Approx(ma2[i]).epsilon(0.00001));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
