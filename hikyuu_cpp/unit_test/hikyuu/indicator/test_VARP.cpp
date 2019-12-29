/*
 * test_VARP.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/VARP.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_VARP test_indicator_VARP
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_VARP") {
    /** @arg n > 1 的正常情况 */
    PriceList d;
    for (size_t i = 0; i < 15; ++i) {
        d.push_back(i + 1);
    }
    d[5] = 4.0;
    d[7] = 4.0;
    d[11] = 6.0;

    Indicator ind = PRICELIST(d);
    Indicator dev = VARP(ind, 10);
    CHECK_EQ(dev.name(), "VARP");
    CHECK_EQ(dev.size(), 15);
    CHECK_UNARY(std::isnan(dev[8]));
    CHECK_LT(std::fabs(dev[9] - 7.69), 0.01);
    CHECK_LT(std::fabs(dev[10] - 8.89), 0.01);
    CHECK_LT(std::fabs(dev[11] - 7.21), 0.01);
    CHECK_LT(std::fabs(dev[12] - 9.61), 0.01);
    CHECK_LT(std::fabs(dev[13] - 12.01), 0.01);
    CHECK_LT(std::fabs(dev[14] - 14.41), 0.01);

    /** @arg n = 1时 */
    dev = VARP(ind, 1);
    CHECK_EQ(dev.name(), "VARP");
    CHECK_EQ(dev.size(), 15);
    for (size_t i = 0; i < dev.size(); ++i) {
        CHECK_UNARY(std::isnan(dev[i]));
    }

    /** @arg operator() */
    Indicator expect = VARP(ind, 10);
    dev = VARP(10);
    Indicator result = dev(ind);
    CHECK_EQ(result.size(), expect.size());
    for (size_t i = result.discard(); i < expect.size(); ++i) {
        CHECK_EQ(result[i], expect[i]);
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_VARP_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/VARP.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = VARP(CLOSE(kdata), 10);
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

    CHECK_EQ(ma2.name(), "VARP");
    CHECK_EQ(ma1.size(), ma2.size());
    CHECK_EQ(ma1.discard(), ma2.discard());
    CHECK_EQ(ma1.getResultNumber(), ma2.getResultNumber());
    for (size_t i = ma1.discard(); i < ma1.size(); ++i) {
        CHECK_EQ(ma1[i], doctest::Approx(ma2[i]).epsilon(0.00001));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
