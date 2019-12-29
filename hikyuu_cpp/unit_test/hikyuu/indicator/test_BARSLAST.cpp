/*
 * test_BARSLAST.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-4
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/BARSLAST.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_BARSLAST test_indicator_BARSLAST
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_BARSLAST") {
    Indicator result;

    PriceList a;
    a.push_back(0);
    a.push_back(0);
    a.push_back(0);
    a.push_back(1);
    a.push_back(0);
    a.push_back(0);
    a.push_back(1);

    Indicator data = PRICELIST(a);

    result = BARSLAST(data);
    CHECK_EQ(result.name(), "BARSLAST");
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 3);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_UNARY(std::isnan(result[1]));
    CHECK_UNARY(std::isnan(result[2]));
    CHECK_EQ(result[3], 0);
    CHECK_EQ(result[4], 1);
    CHECK_EQ(result[5], 2);
    CHECK_EQ(result[6], 0);

    a.insert(a.begin(), 1);
    data = PRICELIST(a);
    result = BARSLAST(data);
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 0);
    CHECK_EQ(result[1], 1);
    CHECK_EQ(result[2], 2);
    CHECK_EQ(result[3], 3);
    CHECK_EQ(result[4], 0);
    CHECK_EQ(result[5], 1);
    CHECK_EQ(result[6], 2);
    CHECK_EQ(result[7], 0);

    a.push_back(0.0);
    data = PRICELIST(a);
    result = BARSLAST(data);
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 0);
    CHECK_EQ(result[1], 1);
    CHECK_EQ(result[2], 2);
    CHECK_EQ(result[3], 3);
    CHECK_EQ(result[4], 0);
    CHECK_EQ(result[5], 1);
    CHECK_EQ(result[6], 2);
    CHECK_EQ(result[7], 0);
    CHECK_EQ(result[8], 1);

    result = BARSLAST(-11);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 0);

    result = BARSLAST(0);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.discard(), 1);
    CHECK_UNARY(std::isnan(result[0]));
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_BARSLAST_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/BARSLAST.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = BARSLAST(CLOSE(kdata));
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

    CHECK_EQ(x2.name(), "BARSLAST");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
