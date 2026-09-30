/*
 * test_EXIST.cpp
 *
 *  Created on: 2019-4-19
 *      Author: fasiondog
 */
#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/EXIST.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_EXIST test_indicator_EXIST
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_EXIST") {
    Indicator result;

    PriceList a;
    a.push_back(0);
    a.push_back(1);
    a.push_back(0);
    a.push_back(1);
    a.push_back(1);
    a.push_back(0);

    Indicator data = PRICELIST(a);

    /** @arg n=0 */
    result = EXIST(data, 0);
    CHECK_EQ(result.discard(), data.discard());
    CHECK_EQ(result.size(), data.size());
    for (int i = 0; i < data.discard(); ++i) {
        CHECK_EQ(result[i], data[i]);
    }

    /** @arg n=1, total>1 */
    result = EXIST(data, 1);
    CHECK_EQ(result.name(), "EXIST");
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), data.size());
    for (int i = 0; i < data.size(); ++i) {
        CHECK_EQ(result[i], data[i]);
    }

    /** @arg n=1, total=1 */
    result = EXIST(CVAL(1), 1);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 1);

    result = EXIST(CVAL(0), 1);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 0);

    /** @arg n > 1, total = n */
    result = EXIST(data, 6);
    CHECK_EQ(result.name(), "EXIST");
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 5);
    CHECK_EQ(result[5], 1);

    /** @arg n > 1, total < n */
    result = EXIST(data, 7);
    CHECK_EQ(result.name(), "EXIST");
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 6);

    /** @arg n > 1, total > n */
    result = EXIST(data, 3);
    CHECK_EQ(result.name(), "EXIST");
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 2);
    for (int i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    CHECK_EQ(result[2], 1);
    CHECK_EQ(result[3], 1);
    CHECK_EQ(result[4], 1);
    CHECK_EQ(result[5], 1);

    a.push_back(0);
    a.push_back(0);
    a.push_back(0);
    a.push_back(0);

    data = PRICELIST(a);
    result = EXIST(data, 3);
    CHECK_EQ(result.name(), "EXIST");
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 2);
    for (int i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    CHECK_EQ(result[2], 1);
    CHECK_EQ(result[3], 1);
    CHECK_EQ(result[4], 1);
    CHECK_EQ(result[5], 1);
    CHECK_EQ(result[6], 1);
    CHECK_EQ(result[7], 0);
    CHECK_EQ(result[8], 0);
    CHECK_EQ(result[9], 0);
}

/** @par Test points */
TEST_CASE("test_EXIST_dyn") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-30));
    // KData kdata = stock.getKData(KQuery(0, Null<size_t>(), KQuery::MIN));
    Indicator c = CLOSE(kdata);
    Indicator o = OPEN(kdata);
    Indicator expect = EXIST(c > o, 3);
    Indicator result = EXIST(c > o, CVAL(c, 3));
    // CHECK_EQ(expect.discard(), result.discard());
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < expect.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    expect = EXIST(c > o, 0);
    result = EXIST(c > o, CVAL(c, 0));
    // CHECK_EQ(expect.discard(), result.discard());
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < expect.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }
}

/**
 * @par Test points
 * The core regression: full calculation == incremental calculation (the same Indicator instance is
 * reused with consecutive setContext calls, so the tail-extended second call enters
 * _increment_calculate).
 *
 * Background: IExist::_increment_calculate used `i <= m_discard` as the seed-scan bound. The
 * framework zeroes m_discard right before calling _increment_calculate (IndicatorImp
 * increment_execute_leaf_or_op), so the seed scan never ran, dst[start_pos] was written 0 and the
 * following up-to-n values lost the window state. The full path set start_pos == m_discard so it
 * was coincidentally correct, hiding the bug. The fix uses `i <= start_pos`.
 */
TEST_CASE("test_EXIST_increment_equivalence") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    // The partial window is a prefix of the full one, extended at the tail -> incremental admission
    KData k_full = stock.getKData(KQuery(-20));
    KData k_partial = stock.getKData(KQuery(-20, -10));

    // (1) The full baseline: an independent instance
    Indicator ind_full = EXIST(CLOSE() > OPEN(), 5);
    ind_full.setContext(k_full);

    // (2) The incremental test: reuse the same instance with consecutive setContext
    Indicator ind_inc = EXIST(CLOSE() > OPEN(), 5);
    ind_inc.setContext(k_partial);  // cache m_old_context
    ind_inc.setContext(k_full);     // extended at the tail -> enter _increment_calculate

    CHECK_EQ(ind_full.size(), ind_inc.size());
    CHECK_EQ(ind_full.discard(), ind_inc.discard());
    for (size_t i = 0; i < ind_full.size(); ++i) {
        if (std::isnan(ind_full[i]) && std::isnan(ind_inc[i])) {
            continue;
        }
        CHECK_EQ(ind_inc[i], doctest::Approx(ind_full[i]).epsilon(0.0001));
    }
}

/**
 * @par Test points
 * A NaN in the condition series means no data: it neither satisfies nor breaks the condition,
 * consistent with IBarsSince and with every consumer (SG_Bool, IF, conditions all treat NaN as
 * not holding).
 */
TEST_CASE("test_EXIST_nan_is_not_a_condition") {
    PriceList a;
    a.push_back(0);
    a.push_back(0);
    a.push_back(Null<price_t>());
    a.push_back(0);
    a.push_back(0);

    Indicator x = PRICELIST(a);
    Indicator result = EXIST(x, 3);
    CHECK_EQ(result.size(), 5);
    // the first two bars have no full window
    CHECK_EQ(result.discard(), 2);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_UNARY(std::isnan(result[1]));
    for (size_t i = result.discard(); i < result.size(); ++i) {
        CHECK_EQ(result[i], 0);
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_EXIST_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/EXIST.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = EXIST(CLOSE(kdata) > OPEN(kdata));
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

    CHECK_EQ(x2.name(), "EXIST");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = x1.discard(); i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
