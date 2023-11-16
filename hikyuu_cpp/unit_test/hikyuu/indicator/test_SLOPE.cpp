/*
 * test_MA.cpp
 *
 *  Created on: 2013-2-12
 *      Author: fasiondog
 */
#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/SLOPE.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_SLOPE test_indicator_SLOPE
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_SLOPE") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData kdata;
    Indicator slope;

    /** @arg 空指标 */
    slope = SLOPE();
    CHECK_EQ(slope.size(), 0);
    CHECK_EQ(slope.name(), "SLOPE");

    /** @arg n = 0, 1 */
    kdata = stock.getKData(KQuery(-100));
    Indicator c = CLOSE(kdata);
    slope = SLOPE(c, 0);
    CHECK_EQ(slope.size(), c.size());
    CHECK_EQ(slope.name(), "SLOPE");
    CHECK_EQ(slope.discard(), 1);
    CHECK_UNARY(std::isnan(slope[0]));
    for (size_t i = slope.discard(), len = slope.size(); i < len; i++) {
        CHECK_EQ(slope[i], 0.);
    }

    slope = SLOPE(c, 1);
    CHECK_EQ(slope.size(), c.size());
    CHECK_EQ(slope.name(), "SLOPE");
    CHECK_EQ(slope.discard(), 1);
    CHECK_UNARY(std::isnan(slope[0]));
    for (size_t i = slope.discard(), len = slope.size(); i < len; i++) {
        CHECK_EQ(slope[i], 0.0);
    }

    /** @arg n = 2 */
    slope = SLOPE(c, 2);
    CHECK_EQ(slope.discard(), 1);
    CHECK_EQ(slope.size(), c.size());
    CHECK_UNARY(std::isnan(slope[0]));
    CHECK_EQ(slope[1], doctest::Approx(40.894).epsilon(0.0001));
    CHECK_EQ(slope[2], doctest::Approx(14.968).epsilon(0.0001));
    CHECK_EQ(slope[3], doctest::Approx(9.725).epsilon(0.0001));

    /** @arg n = 3 */
    slope = SLOPE(c, 3);
    CHECK_EQ(slope.discard(), 1);
    CHECK_EQ(slope.size(), c.size());
    CHECK_UNARY(std::isnan(slope[0]));
    CHECK_EQ(slope[1], doctest::Approx(40.894).epsilon(0.0001));
    CHECK_EQ(slope[2], doctest::Approx(27.931).epsilon(0.0001));
    CHECK_EQ(slope[3], doctest::Approx(12.347).epsilon(0.0001));
}

/** @par 检测点 */
TEST_CASE("test_SLOPE_dyn") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-30));
    Indicator c = CLOSE(kdata);
    Indicator expect = SLOPE(c, 10);
    Indicator result = SLOPE(c, CVAL(c, 10));
    CHECK_EQ(expect.size(), result.size());
    CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    result = SLOPE(c, IndParam(CVAL(c, 10)));
    CHECK_EQ(expect.size(), result.size());
    CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    expect = SLOPE(c, 0);
    result = SLOPE(c, CVAL(c, 0));
    CHECK_EQ(expect.size(), result.size());
    CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_SLOPE_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/SLOPE.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = SLOPE(CLOSE(kdata), 10);
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

    CHECK_EQ(ma2.name(), "SLOPE");
    CHECK_EQ(ma1.size(), ma2.size());
    CHECK_EQ(ma1.discard(), ma2.discard());
    CHECK_EQ(ma1.getResultNumber(), ma2.getResultNumber());
    for (size_t i = ma1.discard(); i < ma1.size(); ++i) {
        CHECK_EQ(ma1[i], doctest::Approx(ma2[i]));
    }
}

#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
