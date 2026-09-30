/*
 * test_DIFF.cpp
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/DIFF.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_DIFF test_indicator_DIFF
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_DIFF") {
    /** @arg The normal test */
    PriceList d;
    for (size_t i = 0; i < 10; ++i) {
        d.push_back(i);
    }

    Indicator ind = PRICELIST(d);
    Indicator diff = DIFF(ind);
    CHECK_EQ(diff.size(), 10);
    CHECK_EQ(diff.discard(), 1);
    CHECK_UNARY(std::isnan(diff[0]));
    for (size_t i = 1; i < 10; ++i) {
        CHECK_EQ(diff[i], d[i] - d[i - 1]);
    }

    /** @arg operator */
    diff = DIFF();
    Indicator expect = DIFF(ind);
    Indicator result = diff(ind);
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = result.discard(); i < expect.size(); ++i) {
        CHECK_EQ(result[i], expect[i]);
    }
}

/**
 * @par Test points
 * Incremental calculate must match a full calculation, including a first context that was fully
 * discarded.
 *
 * Background: IDiff declared incremental support but not min_increment_start(), so the framework
 * could only raise start_pos with the previous m_discard. When the first context was shorter than
 * n, that m_discard degraded to its own size and left start_pos < n, making src[i-n] read before
 * the input and write garbage into the range that must stay NaN.
 */
TEST_CASE("test_DIFF_increment_equivalence") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData k_full = stock.getKData(KQuery(0, 30));
    CHECK_EQ(k_full.size(), 30);

    Indicator expect = DIFF(CLOSE(), 10);
    expect.setContext(k_full);

    auto check_with_history = [&](size_t history) {
        Indicator got = DIFF(CLOSE(), 10);
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

    /** @arg The first context was fully discarded, so start_pos fell below n */
    check_with_history(5);

    /** @arg The first context already had valid results, the usual incremental path */
    check_with_history(20);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_DIFF_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/DIFF.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = DIFF(CLOSE(kdata));
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
        CHECK_EQ(ma1[i], doctest::Approx(ma2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
