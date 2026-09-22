/*
 * test_SIGNED_POWER.cpp
 *
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-6-9
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/SIGNED_POWER.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_SIGNED_POWER test_indicator_SIGNED_POWER
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test point: test the basic functionality and the sign preservation */
TEST_CASE("test_SIGNED_POWER") {
    Indicator result;

    // Test the combination of the positive numbers, the negative numbers and 0
    PriceList a;
    a.push_back(-2);
    a.push_back(-1);
    a.push_back(0);
    a.push_back(1);
    a.push_back(2);

    Indicator data = PRICELIST(a);

    // Test the cube - verify the sign preservation
    result = SIGNED_POWER(data, 3);
    CHECK_EQ(result.name(), "SIGNED_POWER");
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), 5);
    // (-2)^3 = -8, the negative sign is kept
    CHECK_EQ(result[0], doctest::Approx(-8.0));
    // (-1)^3 = -1, the negative sign is kept
    CHECK_EQ(result[1], doctest::Approx(-1.0));
    // 0^3 = 0
    CHECK_EQ(result[2], doctest::Approx(0.0));
    // 1^3 = 1
    CHECK_EQ(result[3], doctest::Approx(1.0));
    // 2^3 = 8
    CHECK_EQ(result[4], doctest::Approx(8.0));

    // Test the square - verify the sign preservation (the square of a negative number keeps the
    // sign)
    result = SIGNED_POWER(data, 2);
    CHECK_EQ(result.name(), "SIGNED_POWER");
    CHECK_EQ(result.discard(), 0);
    // (-2)^2 = -4 (the negative sign is kept)
    CHECK_EQ(result[0], doctest::Approx(-4.0));
    // (-1)^2 = -1 (the negative sign is kept)
    CHECK_EQ(result[1], doctest::Approx(-1.0));
    // 0^2 = 0
    CHECK_EQ(result[2], doctest::Approx(0.0));
    // 1^2 = 1
    CHECK_EQ(result[3], doctest::Approx(1.0));
    // 2^2 = 4
    CHECK_EQ(result[4], doctest::Approx(4.0));

    // Test a single value
    result = SIGNED_POWER(-11, 3);
    CHECK_EQ(result.name(), "SIGNED_POWER");
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.discard(), 0);
    // -11^3 = -1331, the negative sign is kept
    CHECK_EQ(result[0], doctest::Approx(-1331.0));

    result = SIGNED_POWER(5, 3);
    CHECK_EQ(result[0], doctest::Approx(125.0));
}

/** @par Test point: test the dynamic parameter version */
TEST_CASE("test_SIGNED_POWER_dyn") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-30));
    Indicator c = CLOSE(kdata);

    // Use an integer parameter as the reference
    Indicator expect = SIGNED_POWER(c, 3);

    // Test using CVAL as the dynamic parameter
    Indicator result = SIGNED_POWER(c, CVAL(c, 3));
    CHECK_EQ(expect.size(), result.size());
    CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    // Test the dynamic parameter wrapped with IndParam
    result = SIGNED_POWER(c, IndParam(CVAL(c, 3)));
    CHECK_EQ(expect.size(), result.size());
    CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_SIGNED_POWER_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/SIGNED_POWER.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = SIGNED_POWER(CLOSE(kdata), 3);
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

    CHECK_EQ(x1.name(), x2.name());
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */