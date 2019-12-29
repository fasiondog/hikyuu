/*
 * test_SMA.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-16
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/SMA.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_SMA test_indicator_SMA
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_SMA") {
    Indicator result;
    PriceList a;
    Indicator data = PRICELIST(a);

    /** @arg 输入指标长度为0 */
    result = SMA(data);
    CHECK_EQ(result.name(), "SMA");
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), 0);

    /** @arg 输入指标长度为1，n = 2, m=2 */
    a.push_back(10);
    data = PRICELIST(a);
    result = SMA(data, 2, 2);
    CHECK_EQ(result.name(), "SMA");
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], 10);

    /** @arg 输入指标长度为1，n = 1, m=2 */
    a.clear();
    a.push_back(10);
    data = PRICELIST(a);
    result = SMA(data, 1, 2);
    CHECK_EQ(result.name(), "SMA");
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], 10);

    /** @arg 输入指标长度为10，n = 1, m=1 */
    a.clear();
    for (int i = 0; i < 10; ++i) {
        a.push_back(i / 10.0);
    }
    data = PRICELIST(a);
    result = SMA(data, 1, 1);
    CHECK_EQ(result.name(), "SMA");
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), data.size());
    for (size_t i = result.discard(); i < data.size(); i++) {
        CHECK_EQ(result[i], data[i]);
    }

    /** @arg 输入指标长度为1，n = 4, m=2 */
    result = SMA(data, 4, 2);
    CHECK_EQ(result.name(), "SMA");
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result[0], 0);
    CHECK_EQ(result[1], 0.05);
    CHECK_EQ(result[5], doctest::Approx(0.4031).epsilon(0.01));
    CHECK_EQ(result[9], doctest::Approx(0.8002).epsilon(0.01));
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_SMA_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/SMA.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = SMA(CLOSE(kdata));
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

    CHECK_EQ(x2.name(), "SMA");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]).epsilon(0.00001));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
