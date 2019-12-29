/*
 * test_SUMBARS.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-7
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include < fstream >
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/SUMBARS.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_SUMBARS test_indicator_SUMBARS
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_SUMBARS") {
    Indicator result;

    PriceList a;

    for (int i = 0; i < 10; i++) {
        a.push_back(i * 10);
    }
    Indicator data = PRICELIST(a);

    result = SUMBARS(data, 10);
    CHECK_EQ(result.name(), "SUMBARS");
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 1);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_EQ(result[1], 0);
    CHECK_EQ(result[2], 0);
    CHECK_EQ(result[3], 0);
    CHECK_EQ(result[4], 0);
    CHECK_EQ(result[5], 0);
    CHECK_EQ(result[6], 0);
    CHECK_EQ(result[7], 0);
    CHECK_EQ(result[8], 0);
    CHECK_EQ(result[9], 0);

    result = SUMBARS(data, 15);
    CHECK_EQ(result.name(), "SUMBARS");
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 2);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_UNARY(std::isnan(result[1]));
    CHECK_EQ(result[2], 0);
    CHECK_EQ(result[3], 0);
    CHECK_EQ(result[4], 0);
    CHECK_EQ(result[5], 0);
    CHECK_EQ(result[6], 0);
    CHECK_EQ(result[7], 0);
    CHECK_EQ(result[8], 0);
    CHECK_EQ(result[9], 0);

    result = SUMBARS(data, 90);
    CHECK_EQ(result.name(), "SUMBARS");
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 4);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_UNARY(std::isnan(result[1]));
    CHECK_UNARY(std::isnan(result[2]));
    CHECK_UNARY(std::isnan(result[3]));
    CHECK_EQ(result[4], 2);
    CHECK_EQ(result[5], 1);
    CHECK_EQ(result[6], 1);
    CHECK_EQ(result[7], 1);
    CHECK_EQ(result[8], 1);
    CHECK_EQ(result[9], 0);

    result = SUMBARS(data, 0);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result[1], 0);
    CHECK_EQ(result[2], 0);
    CHECK_EQ(result[3], 0);
    CHECK_EQ(result[4], 0);
    CHECK_EQ(result[5], 0);
    CHECK_EQ(result[6], 0);
    CHECK_EQ(result[7], 0);
    CHECK_EQ(result[8], 0);
    CHECK_EQ(result[9], 0);

    result = SUMBARS(data, 170);
    CHECK_EQ(result.discard(), 6);
    CHECK_EQ(result.size(), 10);
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    CHECK_EQ(result[6], 3);
    CHECK_EQ(result[7], 2);
    CHECK_EQ(result[8], 2);
    CHECK_EQ(result[9], 1);

    result = SUMBARS(data, 450);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.discard(), 9);
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    CHECK_EQ(result[9], 8);

    result = SUMBARS(data, 451);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.discard(), 10);
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }

    result = SUMBARS(CVAL(10), 451);
    CHECK_EQ(result.discard(), 1);
    CHECK_EQ(result.size(), 1);
    CHECK_UNARY(std::isnan(result[0]));

    result = SUMBARS(CVAL(10), 10);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 0);

    a.clear();
    a.push_back(10);
    a.push_back(20);
    data = PRICELIST(a);
    result = SUMBARS(data, 20);
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result.discard(), 1);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_EQ(result[1], 0);

    result = SUMBARS(data, 0);
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 0);
    CHECK_EQ(result[1], 0);

    result = SUMBARS(data, 30);
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result.discard(), 1);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_EQ(result[1], 1);

    result = SUMBARS(data, 40);
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result.discard(), 2);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_UNARY(std::isnan(result[1]));
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_SUMBARS_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/SUMBARS.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = SUMBARS(CLOSE(kdata), 10000);
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

    CHECK_EQ(x2.name(), "SUMBARS");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = x1.discard(); i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]).epsilon(0.00001));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
