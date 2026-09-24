/*
 * test_ADJ_FACTOR.cpp
 *
 *  Created on: 2026-05-17
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/ADJ_FACTOR.h>

using namespace hku;

/**
 * @defgroup test_indicator_ADJ_FACTOR test_indicator_ADJ_FACTOR
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_ADJ_FACTOR") {
    KData k;
    Indicator adj_factor;

    /** @arg Test the version with a KData parameter - a stock with ex-rights/ex-dividend data */
    Stock stk = getStock("SZ000001");
    REQUIRE(!stk.isNull());
    k = stk.getKData(KQuery(-100));
    REQUIRE(k.size() > 0);

    adj_factor = ADJ_FACTOR(k);
    CHECK_EQ(adj_factor.name(), "ADJ_FACTOR");
    CHECK_EQ(adj_factor.size(), k.size());
    CHECK_EQ(adj_factor.discard(), 0);
    CHECK_EQ(adj_factor.getResultNumber(), 1);

    // All the adjustment factors should be >= 1.0 (a backward adjustment factor only grows or
    // stays)
    for (size_t i = 0; i < adj_factor.size(); ++i) {
        CHECK_GE(adj_factor[i], doctest::Approx(1.0));
    }

    /** @arg Test the no-argument version + setContext */
    stk = getStock("SZ000001");
    REQUIRE(!stk.isNull());
    k = stk.getKData(KQuery(-100));
    REQUIRE(k.size() > 0);

    adj_factor = ADJ_FACTOR();
    adj_factor.setContext(k);
    CHECK_EQ(adj_factor.size(), k.size());
    CHECK_EQ(adj_factor.discard(), 0);

    // Verify that it matches the result of the version with a parameter
    Indicator adj_factor2 = ADJ_FACTOR(k);
    CHECK_EQ(adj_factor.size(), adj_factor2.size());
    for (size_t i = 0; i < adj_factor.size(); ++i) {
        CHECK_EQ(adj_factor[i], doctest::Approx(adj_factor2[i]).epsilon(0.0001));
    }

    /** @arg Test a stock without ex-rights/ex-dividend data (such as an index) */
    stk = getStock("sh000001");
    REQUIRE(!stk.isNull());
    k = stk.getKData(KQuery(-50));
    REQUIRE(k.size() > 0);

    adj_factor = ADJ_FACTOR(k);
    CHECK_EQ(adj_factor.size(), k.size());
    CHECK_EQ(adj_factor.discard(), 0);

    // An index has no ex-rights/ex-dividend data, so all the factors should be 1.0
    for (size_t i = 0; i < adj_factor.size(); ++i) {
        CHECK_EQ(adj_factor[i], doctest::Approx(1.0).epsilon(0.0001));
    }

    /** @arg Empty K-line data */
    KData empty_k;
    adj_factor = ADJ_FACTOR(empty_k);
    CHECK_EQ(adj_factor.size(), 0);
    CHECK_EQ(adj_factor.discard(), 0);

    /** @arg Test the incremental calculation support */
    stk = getStock("SZ000001");
    REQUIRE(!stk.isNull());

    // First get the data of a shorter time range
    KData k_short = stk.getKData(KQuery(-20));
    REQUIRE(k_short.size() > 0);

    adj_factor = ADJ_FACTOR(k_short);
    CHECK_EQ(adj_factor.size(), k_short.size());

    // Then get the data of a longer time range
    KData k_long = stk.getKData(KQuery(-50));
    REQUIRE(k_long.size() > k_short.size());

    Indicator adj_factor_long = ADJ_FACTOR(k_long);
    CHECK_EQ(adj_factor_long.size(), k_long.size());

    // Verify the consistency of the overlapping part (the correctness of the incremental
    // calculation)
    size_t overlap_start = k_long.size() - k_short.size();
    for (size_t i = 0; i < k_short.size(); ++i) {
        CHECK_EQ(adj_factor_long[overlap_start + i],
                 doctest::Approx(adj_factor[i]).epsilon(0.0001));
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_ADJ_FACTOR_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/ADJ_FACTOR.xml";

    Stock stk = getStock("SH600000");
    REQUIRE(!stk.isNull());
    KData k = stk.getKData(KQuery(-20));
    REQUIRE(k.size() > 0);

    Indicator x1 = ADJ_FACTOR(k);
    x1.setContext(k);

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

    CHECK_EQ(x2.name(), "ADJ_FACTOR");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]).epsilon(0.0001));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
