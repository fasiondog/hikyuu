/*
 * test_ROCP.cpp
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/ROCP.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_ROCP test_indicator_ROCP
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_ROCP") {
    Indicator result;

    PriceList a;
    for (int i = 0; i < 10; ++i) {
        a.push_back(i);
    }

    Indicator data = PRICELIST(a);
    result = ROCP(data, 1);
    CHECK_EQ(result.name(), "ROCP");
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 1);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_EQ(result[1], 0);
    CHECK_EQ(result[2], 1);
    CHECK_EQ(result[3], 0.5);
    CHECK_EQ(result[5], 0.25);
    CHECK_EQ(result[9], 0.125);

    result = ROCP(data, 2);
    CHECK_EQ(result.name(), "ROCP");
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 2);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_UNARY(std::isnan(result[1]));
    CHECK_EQ(result[2], 0);
    CHECK_EQ(result[3], 2);
    CHECK_EQ(result[4], 1);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_ROCP_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/ROCP.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = ROCP(CLOSE(kdata));
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

    CHECK_EQ(x2.name(), "ROCP");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = x1.discard(); i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
