/*
 * test_BACKSET.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-13
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/BACKSET.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_BACKSET test_indicator_BACKSET
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_BACKSET") {
    Indicator result;

    /** @arg 未指定输入ind */
    result = BACKSET();
    CHECK_EQ(result.name(), "BACKSET");
    CHECK_EQ(result.size(), 0);
    CHECK_EQ(result.discard(), 0);

    /** @arg n = 1, total = 0*/
    PriceList a;
    Indicator data = PRICELIST(a);
    result = BACKSET(data, 1);
    CHECK_EQ(result.size(), 0);
    CHECK_EQ(result.discard(), 0);

    /** @arg n = 1, total = 1*/
    a.push_back(0);
    data = PRICELIST(a);
    result = BACKSET(data, 1);
    CHECK_EQ(result.name(), "BACKSET");
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 0);

    /** @arg n = 1, total > 1*/
    a.push_back(1);
    a.push_back(0);
    a.push_back(1);
    a.push_back(1);

    data = PRICELIST(a);
    result = BACKSET(data, 1);
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 0);
    for (size_t i = 0; i < data.size(); i++) {
        CHECK_EQ(result[i], data[i]);
    }

    /** @arg n = 1, total = 0*/
    a.clear();
    data = PRICELIST(a);
    result = BACKSET(data, 1);
    CHECK_EQ(result.size(), 0);
    CHECK_EQ(result.discard(), 0);

    /** @arg n = 2, total = 2*/
    a.clear();
    a.push_back(1);
    a.push_back(0);
    data = PRICELIST(a);
    result = BACKSET(data, 2);
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 1);
    CHECK_EQ(result[1], 0);

    a.clear();
    a.push_back(0);
    a.push_back(1);
    data = PRICELIST(a);
    result = BACKSET(data, 2);
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 1);
    CHECK_EQ(result[1], 1);

    /** @arg n = 3 */
    a.clear();
    a.push_back(0);
    a.push_back(1);
    a.push_back(0);
    a.push_back(0);
    a.push_back(0);
    a.push_back(0);
    a.push_back(1);
    a.push_back(0);
    a.push_back(1);
    a.push_back(0);
    a.push_back(1);
    data = PRICELIST(a);
    result = BACKSET(data, 3);
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 1);
    CHECK_EQ(result[1], 1);
    CHECK_EQ(result[2], 0);
    CHECK_EQ(result[3], 0);
    CHECK_EQ(result[4], 1);
    CHECK_EQ(result[5], 1);
    CHECK_EQ(result[6], 1);
    CHECK_EQ(result[7], 1);
    CHECK_EQ(result[8], 1);
    CHECK_EQ(result[9], 1);
    CHECK_EQ(result[10], 1);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_BACKSET_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/BACKSET.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = BACKSET(CLOSE(kdata));
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

    CHECK_EQ(x2.name(), "BACKSET");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
