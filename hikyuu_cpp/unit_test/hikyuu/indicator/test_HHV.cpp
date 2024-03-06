/*
 * test_HHV.cpp
 *
 *  Created on: 2019年4月1日
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/HHV.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_HHV test_indicator_HHV
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_HHV") {
    Indicator result;

    PriceList a;
    for (int i = 0; i < 10; ++i) {
        a.push_back(i);
    }

    Indicator data = PRICELIST(a);

    /** @arg n = 0 */
    result = HHV(data, 0);
    CHECK_EQ(result.discard(), 0);
    for (int i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], data[i]);
    }

    /** @arg n = 1 */
    result = HHV(data, 1);
    CHECK_EQ(result.discard(), 0);
    for (int i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], data[i]);
    }

    /** @arg n = 9 */
    result = HHV(data, 9);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], data[0]);
    CHECK_EQ(result[1], data[1]);
    CHECK_EQ(result[7], data[7]);
    CHECK_EQ(result[8], data[8]);
    CHECK_EQ(result[9], data[9]);

    /** @arg n = 10 */
    result = HHV(data, 10);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], data[0]);
    CHECK_EQ(result[1], data[1]);
    CHECK_EQ(result[8], data[8]);
    CHECK_EQ(result[9], data[9]);

    /** ind.size() == 1 */
    result = HHV(CVAL(2));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 2);

    /** 一个实际出错过的例子 */
    a = {4508.4, 3994.6, 2815.6, 3438.6, 2525.0, 12943.1, 16302.7, 8211.5, 5704.6, 4660.4, 4868.6,
         8107.4, 3940.5, 6332.1, 3129.0, 4750.9, 4582.6,  5859.2,  3825.7, 7030.2, 9430.0};
    result = HHV(PRICELIST(a), 10);
    PriceList expect{4508.4,  4508.4,  4508.4,  4508.4,  4508.4,  12943.1, 16302.7,
                     16302.7, 16302.7, 16302.7, 16302.7, 16302.7, 16302.7, 16302.7,
                     16302.7, 16302.7, 8211.5,  8107.4,  8107.4,  8107.4,  9430.0};
    for (size_t i = 0; i < 20; i++) {
        CHECK_EQ(result[i], doctest::Approx(expect[i]));
    }
}

/** @par 检测点 */
TEST_CASE("test_HHV_dyn") {
    PriceList a;
    for (int i = 0; i < 10; ++i) {
        a.push_back(i);
    }

    Indicator data = PRICELIST(a);
    Indicator expect = HHV(data, 3);
    Indicator result = HHV(data, CVAL(data, 3));
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < expect.size(); i++) {
        CHECK_EQ(expect[i], result[i]);
    }

    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-100));
    // KData kdata = stock.getKData(KQuery(0, Null<size_t>(), KQuery::MIN));
    Indicator c = CLOSE(kdata);
    expect = HHV(c, 50);
    result = HHV(c, CVAL(c, 50));
    CHECK_EQ(expect.size(), result.size());
    CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], result[i]);
    }

    result = HHV(c, IndParam(CVAL(c, 50)));
    CHECK_EQ(expect.size(), result.size());
    CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], result[i]);
    }

    expect = HHV(c, 0);
    result = HHV(c, CVAL(c, 0));
    CHECK_EQ(expect.size(), result.size());
    CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], result[i]);
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_HHV_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/HHV.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = HHV(CLOSE(kdata), 2);
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

    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
