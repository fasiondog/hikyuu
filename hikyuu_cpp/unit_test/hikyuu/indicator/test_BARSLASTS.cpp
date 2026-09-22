/*
 * test_BARSLASTS.cpp
 *
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-17
 *      Author: hikyuu
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/BARSLASTS.h>
#include <hikyuu/indicator/crt/BARSLAST.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/REF.h>
#include <hikyuu/indicator/crt/SLICE.h>
#include <hikyuu/indicator/Indicator.h>

using namespace hku;

/**
 * @defgroup test_indicator_BARSLASTS test_BARSLASTS
 * @ingroup test_hikyuu_indicator
 * @{
 */

/** @par Test points */
TEST_CASE("test_BARSLASTS") {
    /** @arg With n=1 the result should match BARSLAST */
    PriceList a;
    for (int i = 0; i < 8; ++i) {
        a.push_back(i % 4 == 0 ? 1.0 : 0.0);
    }
    Indicator data = PRICELIST(a);
    Indicator result = BARSLASTS(data, 1);
    Indicator expected = BARSLAST(data);
    CHECK_EQ(result.size(), expected.size());
    CHECK_EQ(result.discard(), expected.discard());
    check_indicator(result, expected);

    /** @arg The basic test with n=2 */
    result = BARSLASTS(data, 2);
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 4);

    // Position 0: the condition holds for the 1st time, fewer than 2, so NaN
    CHECK_UNARY(std::isnan(result[0]));
    // Positions 1-3: the condition holds only once, fewer than 2, so NaN
    CHECK_UNARY(std::isnan(result[1]));
    CHECK_UNARY(std::isnan(result[2]));
    CHECK_UNARY(std::isnan(result[3]));
    // Position 4: the condition holds for the 2nd time, the distance from the 1st (position 0) is 4
    CHECK_EQ(result[4], 4);
    // Positions 5-6: the condition has held twice, the distances from the 1st (position 0) are 5
    // and 6
    CHECK_EQ(result[5], 5);
    CHECK_EQ(result[6], 6);
    // Position 7: the condition has held twice, the distance from the 1st (position 0) is 7
    CHECK_EQ(result[7], 7);

    /** @arg The test with n=3 */
    result = BARSLASTS(data, 3);
    CHECK_EQ(result.size(), data.size());
    // The first 7 positions: the condition holds only twice, fewer than 3, so NaN
    for (size_t i = 0; i < 7; ++i) {
        CHECK_UNARY(std::isnan(result[i]));
    }

    /** @arg With n <= 0 an all-NaN sequence should be returned */
    result = BARSLASTS(data, 0);
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), data.size());
    for (size_t i = 0; i < result.size(); ++i) {
        CHECK_UNARY(std::isnan(result[i]));
    }

    result = BARSLASTS(data, -1);
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), data.size());
    for (size_t i = 0; i < result.size(); ++i) {
        CHECK_UNARY(std::isnan(result[i]));
    }

    result = BARSLASTS(data, -100);
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), data.size());
    for (size_t i = 0; i < result.size(); ++i) {
        CHECK_UNARY(std::isnan(result[i]));
    }

    /** @arg The empty data test */
    PriceList empty;
    data = PRICELIST(empty);
    result = BARSLASTS(data, 1);
    CHECK_EQ(result.size(), 0);

    /** @arg The all-zero data test */
    PriceList zeros;
    for (int i = 0; i < 5; ++i) {
        zeros.push_back(0.0);
    }
    data = PRICELIST(zeros);
    result = BARSLASTS(data, 1);
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), data.size());
}

/** @par Test point - the dynamic parameter test */
TEST_CASE("test_BARSLASTS_dyn") {
    /** @arg The comparison between the dynamic parameter n and the static parameter n=2 */
    PriceList a;
    for (int i = 0; i < 10; ++i) {
        a.push_back(i % 3 == 0 ? 1.0 : 0.0);
    }
    Indicator data = PRICELIST(a);

    // The static parameter version
    Indicator expect_static = BARSLASTS(data, 2);

    // The dynamic parameter version (a constant indicator is created with CVAL)
    Indicator result_dyn = BARSLASTS(data, CVAL(data, 2));

    CHECK_EQ(expect_static.size(), result_dyn.size());
    CHECK_EQ(expect_static.discard(), result_dyn.discard());
    for (size_t i = 0; i < result_dyn.size(); ++i) {
        if (std::isnan(expect_static[i])) {
            CHECK_UNARY(std::isnan(result_dyn[i]));
        } else {
            CHECK_EQ(expect_static[i], doctest::Approx(result_dyn[i]));
        }
    }

    /** @arg The comparison between the dynamic parameter n and the IndParam version */
    Indicator result_indparam = BARSLASTS(data, IndParam(CVAL(data, 2)));
    CHECK_EQ(expect_static.size(), result_indparam.size());
    CHECK_EQ(expect_static.discard(), result_indparam.discard());
    for (size_t i = 0; i < result_indparam.size(); ++i) {
        if (std::isnan(expect_static[i])) {
            CHECK_UNARY(std::isnan(result_indparam[i]));
        } else {
            CHECK_EQ(expect_static[i], doctest::Approx(result_indparam[i]));
        }
    }

    /** @arg The case where the dynamic parameter changes */
    // Build a varying n indicator: n=1 in the first half and n=2 in the second half
    PriceList n_values;
    for (int i = 0; i < 10; ++i) {
        n_values.push_back(i < 5 ? 1.0 : 2.0);
    }
    Indicator n_param = PRICELIST(n_values);

    result_dyn = BARSLASTS(data, n_param);
    CHECK_EQ(result_dyn.size(), data.size());

    // Verify that the first 5 positions use the n=1 logic (they should match BARSLAST)
    Indicator expect_first_half = BARSLAST(SLICE(data, 0, 5));
    for (size_t i = 0; i < 5; ++i) {
        if (std::isnan(expect_first_half[i])) {
            CHECK_UNARY(std::isnan(result_dyn[i]));
        } else {
            CHECK_EQ(expect_first_half[i], doctest::Approx(result_dyn[i]));
        }
    }

    /** @arg The case of the dynamic parameter n<=0 */
    PriceList zero_n;
    for (int i = 0; i < 10; ++i) {
        zero_n.push_back(0.0);
    }
    Indicator zero_param = PRICELIST(zero_n);
    result_dyn = BARSLASTS(data, zero_param);
    CHECK_EQ(result_dyn.size(), data.size());
    CHECK_EQ(result_dyn.discard(), data.size());

    /** @arg The real stock data test */
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-30));
    Indicator c = CLOSE(kdata);
    Indicator cond = c > REF(c, 1);  // The rising condition

    // The static parameter
    expect_static = BARSLASTS(cond, 2);

    // The dynamic parameter (a constant)
    result_dyn = BARSLASTS(cond, CVAL(cond, 2));
    CHECK_EQ(expect_static.size(), result_dyn.size());
    for (size_t i = 0; i < result_dyn.size(); ++i) {
        if (std::isnan(expect_static[i])) {
            CHECK_UNARY(std::isnan(result_dyn[i]));
        } else {
            CHECK_EQ(expect_static[i], doctest::Approx(result_dyn[i]));
        }
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_BARSLASTS_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/BARSLASTS.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = BARSLASTS(CLOSE(kdata), 2);
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

    CHECK_EQ(x2.name(), "BARSLASTS");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    check_indicator(x1, x2);
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
