/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-12
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/REPLACE.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_REPLACE test_indicator_REPLACE
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_REPLACE") {
    double nan = Null<double>();

    /** @arg 替换非 nan 值, 不忽略 discard */
    Indicator input = PRICELIST(PriceList{nan, nan, 3, 4.3, 5, 4.3, 7, 8, 9, 10}, 2);
    Indicator expect = PRICELIST(PriceList{nan, nan, 3, 4, 5, 4, 7, 8, 9, 10}, 2);
    Indicator result = REPLACE(input, 4.3, 4);
    CHECK_UNARY(result.name() == "REPLACE");
    CHECK_EQ(result.size(), expect.size());
    CHECK_EQ(result.discard(), expect.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (int i = result.discard(); i < result.size(); ++i) {
        CHECK_EQ(result[i], expect[i]);
    }

    /** @arg 替换非 Nan值为 nan 值，忽略 discard */
    input = PRICELIST(PriceList{nan, nan, 3, 4.3, 5, 4.3, 7, 8, 9, 10}, 2);
    expect = PRICELIST(PriceList{nan, nan, nan, 4.3, 5, 4.3, 7, 8, 9, 10}, 3);
    result = REPLACE(input, 3, nan, true);
    CHECK_EQ(result.size(), expect.size());
    CHECK_EQ(result.discard(), expect.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (int i = result.discard(); i < result.size(); ++i) {
        CHECK_EQ(result[i], expect[i]);
    }

    /** @arg 替换 Nan值为 0，不忽略 discard */
    input = PRICELIST(PriceList{nan, nan, 3, nan, 5, 4.3, 7, 8, 9, 10}, 2);
    expect = PRICELIST(PriceList{nan, nan, 3, 0.0, 5, 4.3, 7, 8, 9, 10}, 2);
    result = REPLACE(input, nan, 0.0);
    CHECK_EQ(result.size(), expect.size());
    CHECK_EQ(result.discard(), expect.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (int i = result.discard(); i < result.size(); ++i) {
        CHECK_EQ(result[i], expect[i]);
    }

    /** @arg 替换 Nan值为 0，忽略 discard */
    input = PRICELIST(PriceList{nan, nan, 3, nan, 5, 4.3, 7, 8, 9, 10}, 2);
    expect = PRICELIST(PriceList{1., 1., 3, 1.0, 5, 4.3, 7, 8, 9, 10});
    result = REPLACE(input, nan, 1.0, true);
    CHECK_EQ(result.size(), expect.size());
    CHECK_EQ(result.discard(), expect.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (int i = result.discard(); i < result.size(); ++i) {
        CHECK_EQ(result[i], expect[i]);
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_REPLACE_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/REPLACE.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = REPLACE(CLOSE(kdata));
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

    CHECK_UNARY(x1.size() == x2.size());
    CHECK_UNARY(x1.discard() == x2.discard());
    CHECK_UNARY(x1.getResultNumber() == x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]).epsilon(0.00001));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
