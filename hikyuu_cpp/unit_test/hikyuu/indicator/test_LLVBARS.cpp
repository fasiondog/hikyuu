/*
 * test_LLVBARS.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-14
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/LLVBARS.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_LLVBARS test_indicator_LLVBARS
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_LLVBARS") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm.getStock("sh000001");
    KData k = stk.getKData(-10);
    Indicator c = CLOSE(k);

    Indicator result;

    /** @arg n = 0, 低点顺序下降 */
    PriceList a;
    for (int i = 0; i < 10; ++i) {
        a.push_back(10 - i);
    }

    Indicator data = PRICELIST(a);
    result = LLVBARS(data, 0);
    CHECK_EQ(result.name(), "LLVBARS");
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.discard(), 0);
    for (int i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], 0);
    }

    /** @arg n = 0, 低点升序 */
    a.clear();
    for (int i = 0; i < 10; ++i) {
        a.push_back(i);
    }

    data = PRICELIST(a);
    result = LLVBARS(data, 0);
    CHECK_EQ(result.name(), "LLVBARS");
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.discard(), 0);
    for (int i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], i);
    }

    /** @arg n = 0， 顺序随机 */
    result = LLVBARS(c, 0);
    CHECK_EQ(result.name(), "LLVBARS");
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 0);
    CHECK_EQ(result[1], 1);
    CHECK_EQ(result[2], 0);
    CHECK_EQ(result[3], 1);
    CHECK_EQ(result[4], 2);
    CHECK_EQ(result[5], 0);
    CHECK_EQ(result[6], 1);
    CHECK_EQ(result[9], 0);

    /** @arg n = 1 */
    result = LLVBARS(c, 1);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.discard(), 0);
    for (int i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], 0);
    }

    /** @arg n = 5 */
    result = LLVBARS(c, 5);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 0);
    CHECK_EQ(result[1], 1);
    CHECK_EQ(result[2], 0);
    CHECK_EQ(result[3], 1);
    CHECK_EQ(result[4], 2);
    CHECK_EQ(result[5], 0);
    CHECK_EQ(result[8], 0);
    CHECK_EQ(result[9], 0);

    /** @arg n = 10 */
    result = LLVBARS(c, 10);
    CHECK_EQ(result.name(), "LLVBARS");
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 0);
    CHECK_EQ(result[1], 1);
    CHECK_EQ(result[5], 0);
    CHECK_EQ(result[6], 1);
    CHECK_EQ(result[7], 2);
    CHECK_EQ(result[9], 0);

    /** @arg ind.size() == 1 */
    result = LLVBARS(CVAL(1));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 0);

    /** @arg n > ind.size() */
    result = LLVBARS(c, 20);
    CHECK_GT(20, c.size());
    CHECK_EQ(result.name(), "LLVBARS");
    CHECK_EQ(result.name(), "LLVBARS");
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 0);
    CHECK_EQ(result[1], 1);
    CHECK_EQ(result[2], 0);
    CHECK_EQ(result[3], 1);
    CHECK_EQ(result[4], 2);
    CHECK_EQ(result[5], 0);
    CHECK_EQ(result[6], 1);
    CHECK_EQ(result[9], 0);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_LLVBARS_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/LLVBARS.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = LLVBARS(CLOSE(kdata), 2);
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

    CHECK_EQ(x2.name(), "LLVBARS");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
