/*
 * test_LAST.cpp
 *
 *  Created on: 2019-4-28
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/LAST.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_LAST test_indicator_LAST
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_LAST") {
    Indicator result;

    PriceList a;
    a.push_back(0);   // 0
    a.push_back(1);   // 1
    a.push_back(2);   // 2
    a.push_back(3);   // 3
    a.push_back(4);   // 4
    a.push_back(0);   // 5
    a.push_back(0);   // 6
    a.push_back(7);   // 7
    a.push_back(8);   // 8
    a.push_back(9);   // 9
    a.push_back(10);  // 10

    Indicator data = PRICELIST(a);

    /** @arg m=0, n=0 */
    result = LAST(data, 0, 0);
    CHECK_EQ(result.name(), "LAST");
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 0);
    CHECK_EQ(result[1], 1);
    CHECK_EQ(result[2], 1);
    CHECK_EQ(result[3], 1);
    CHECK_EQ(result[4], 1);
    CHECK_EQ(result[5], 0);
    CHECK_EQ(result[6], 0);
    CHECK_EQ(result[7], 1);
    CHECK_EQ(result[8], 1);
    CHECK_EQ(result[9], 1);
    CHECK_EQ(result[10], 1);

    /** @arg m=0, n=5 */
    result = LAST(data, 0, 5);
    CHECK_EQ(result.name(), "LAST");
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 5);
    CHECK_EQ(result[5], 0);
    CHECK_EQ(result[6], 0);
    CHECK_EQ(result[7], 0);
    CHECK_EQ(result[8], 0);
    CHECK_EQ(result[9], 0);
    CHECK_EQ(result[10], 0);

    /** @arg m=6,n=3*/
    result = LAST(data, 6, 3);
    CHECK_EQ(result.name(), "LAST");
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 6);
    CHECK_EQ(result[6], 0);
    CHECK_EQ(result[7], 1);
    CHECK_EQ(result[8], 0);
    CHECK_EQ(result[9], 0);
    CHECK_EQ(result[10], 0);

    /** @arg m=3,n=6*/
    result = LAST(data, 6, 3);
    CHECK_EQ(result.name(), "LAST");
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 6);
    CHECK_EQ(result[6], 0);
    CHECK_EQ(result[7], 1);
    CHECK_EQ(result[8], 0);
    CHECK_EQ(result[9], 0);
    CHECK_EQ(result[10], 0);

    /** @arg m=3,n=2*/
    result = LAST(data, 3, 2);
    CHECK_EQ(result.name(), "LAST");
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 3);
    CHECK_EQ(result[3], 0);
    CHECK_EQ(result[4], 1);
    CHECK_EQ(result[5], 1);
    CHECK_EQ(result[6], 1);
    CHECK_EQ(result[7], 0);
    CHECK_EQ(result[8], 0);
    CHECK_EQ(result[9], 0);
    CHECK_EQ(result[10], 1);

    /** @arg m=3,n=3*/
    result = LAST(data, 3, 3);
    CHECK_EQ(result.name(), "LAST");
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 3);
    CHECK_EQ(result[3], 0);
    CHECK_EQ(result[4], 1);
    CHECK_EQ(result[5], 1);
    CHECK_EQ(result[6], 1);
    CHECK_EQ(result[7], 1);
    CHECK_EQ(result[8], 0);
    CHECK_EQ(result[9], 0);
    CHECK_EQ(result[10], 1);
}

/** @par 检测点 */
TEST_CASE("test_LAST_dyn") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-30));
    // KData kdata = stock.getKData(KQuery(0, Null<size_t>(), KQuery::MIN));
    Indicator c = CLOSE(kdata);
    Indicator o = OPEN(kdata);
    Indicator expect = LAST(c, 6, 3);
    Indicator result = LAST(c, CVAL(c, 6), CVAL(c, 3));
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = result.discard(); i < result.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    result = LAST(c, IndParam(CVAL(c, 6)), IndParam(CVAL(c, 3)));
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = result.discard(); i < result.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_LAST_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/LAST.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = LAST(CLOSE(kdata) > OPEN(kdata));
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

    CHECK_EQ(x2.name(), "LAST");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = x1.discard(); i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
