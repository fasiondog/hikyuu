/*
 * test_BETWEEN.cpp
 *
 *  Created on: 2019年4月2日
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/BETWEEN.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_BETWEEN test_indicator_BETWEEN
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_BETWEEN") {
    Indicator result;

    /** @arg 所有参数均为 ind */
    PriceList aval;
    for (int i = 0; i < 10; i++) {
        aval.push_back(i);
    }

    PriceList bval;
    for (int i = 0; i < 10; i++) {
        bval.push_back(5);
    }

    PriceList cval;
    for (int i = 0; i < 10; i++) {
        cval.push_back(3);
    }

    Indicator a = PRICELIST(aval);
    Indicator b = PRICELIST(bval);
    Indicator c = PRICELIST(cval);

    result = BETWEEN(a, b, c);
    CHECK_EQ(result.name(), "BETWEEN");
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result[4], 1);
    for (int i = 0; i < 10; i++) {
        if (i != 4) {
            CHECK_EQ(result[i], 0);
        }
    }

    /** @arg a 为ind，b、c均为数字 */
    result = BETWEEN(a, 5, 3);
    CHECK_EQ(result.name(), "BETWEEN");
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result[4], 1);
    for (int i = 0; i < 10; i++) {
        if (i != 4) {
            CHECK_EQ(result[i], 0);
        }
    }

    /** @arg a 为数字，b、c中一个为 ind，一个为数字 */
    result = BETWEEN(2, b, 2);
    CHECK_EQ(result.name(), "BETWEEN");
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), 10);
    for (int i = 0; i < 10; i++) {
        CHECK_EQ(result[i], 0);
    }

    result = BETWEEN(3, b, 2);
    CHECK_EQ(result.name(), "BETWEEN");
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), 10);
    for (int i = 0; i < 10; i++) {
        CHECK_EQ(result[i], 1);
    }

    /** @arg a、b、c均为数字 */
    result = BETWEEN(1, 2, 3);
    CHECK_EQ(result.name(), "BETWEEN");
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 0);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_BETWEEN_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/BETWEEN.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));

    PriceList aval;
    for (int i = 0; i < 10; i++) {
        aval.push_back(i);
    }

    PriceList bval;
    for (int i = 0; i < 10; i++) {
        bval.push_back(5);
    }

    PriceList cval;
    for (int i = 0; i < 10; i++) {
        cval.push_back(3);
    }

    Indicator a = PRICELIST(aval);
    Indicator b = PRICELIST(bval);
    Indicator c = PRICELIST(cval);

    Indicator x1 = BETWEEN(a, b, c);
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
