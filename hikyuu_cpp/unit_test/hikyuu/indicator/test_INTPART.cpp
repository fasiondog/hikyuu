/*
 * test_INTPART.cpp
 *
 *  Created on: 2019-4-18
 *      Author: fasiondog
 */
#include "doctest/doctest.h"
#include <cmath>
#include <fstream>
#include <limits>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/INTPART.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_INTPART test_indicator_INTPART
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_INTPART") {
    Indicator result;

    PriceList a;
    a.push_back(13.2);
    a.push_back(0.3);
    a.push_back(1.5);

    Indicator data = PRICELIST(a);

    result = INTPART(data);
    CHECK_EQ(result.name(), "INTPART");
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(result[0], doctest::Approx(13));
    CHECK_EQ(result[1], doctest::Approx(0));
    CHECK_EQ(result[2], doctest::Approx(1));

    result = INTPART(-11.15);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], doctest::Approx(-11));
}

/** @par Test points */
TEST_CASE("test_INTPART_boundary") {
    const double nan = Null<double>();
    const double inf = std::numeric_limits<double>::infinity();
    const double big = 3000000000.7;  // 3e9, beyond int32 range but a finite value

    PriceList a;
    for (double v : {0.9, -0.9, 2.5, -2.5, nan, inf, -inf, big}) {
        a.push_back(v);
    }
    Indicator result = INTPART(PRICELIST(a));
    CHECK_EQ(result.size(), 8);
    CHECK_EQ(result.discard(), 0);

    /** @arg trunc toward zero (both signs) */
    CHECK_EQ(result[0], 0.0);
    CHECK_EQ(result[1], 0.0);
    CHECK_EQ(result[2], 2.0);
    CHECK_EQ(result[3], -2.0);
    /** @arg NaN input -> null (no longer swallowed as an int conversion) */
    CHECK_UNARY(std::isnan(result[4]));
    /** @arg +-inf input -> null instead of UB double->int */
    CHECK_UNARY(std::isnan(result[5]));
    CHECK_UNARY(std::isnan(result[6]));
    /** @arg large finite value beyond int32 range is kept (no artificial int32 bound) */
    CHECK_EQ(result[7], 3000000000.0);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_INTPART_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/INTPART.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = INTPART(CLOSE(kdata));
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

    CHECK_EQ(x2.name(), "INTPART");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
