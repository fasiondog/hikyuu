/*
 * test_FITR2.cpp
 *
 *  Created on: 2023-11-09
 *      Author: fasiondog
 */
#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/FITR2.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_FITR2 test_indicator_FITR2
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_FITR2") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData kdata;
    Indicator fitr2;

    /** @arg 空指标 */
    fitr2 = FITR2();
    CHECK_EQ(fitr2.size(), 0);
    CHECK_EQ(fitr2.name(), "FITR2");

    /** @arg n = 2 */
    kdata = stock.getKData(KQuery(-100));
    Indicator c = CLOSE(kdata);
    fitr2 = FITR2(c, 2);
    CHECK_EQ(fitr2.size(), c.size());
    CHECK_EQ(fitr2.name(), "FITR2");
    CHECK_EQ(fitr2.discard(), 1);
    CHECK_UNARY(std::isnan(fitr2[0]));

    /** @arg n = 3, 抛弃前 2 个点 */
    fitr2 = FITR2(c, 3);
    CHECK_EQ(fitr2.size(), c.size());
    CHECK_EQ(fitr2.name(), "FITR2");
    CHECK_EQ(fitr2.discard(), 2);  // n-1 = 2
    CHECK_UNARY(std::isnan(fitr2[0]));
    CHECK_UNARY(std::isnan(fitr2[1]));
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_FITR2_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/FITR2.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator fitr2_1 = FITR2(CLOSE(kdata), 10);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(fitr2_1);
    }

    Indicator fitr2_2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(fitr2_2);
    }

    CHECK_EQ(fitr2_2.name(), "FITR2");
    CHECK_EQ(fitr2_1.size(), fitr2_2.size());
    CHECK_EQ(fitr2_1.discard(), fitr2_2.discard());
    CHECK_EQ(fitr2_1.getResultNumber(), fitr2_2.getResultNumber());
    for (size_t i = fitr2_1.discard(); i < fitr2_1.size(); ++i) {
        CHECK_EQ(fitr2_1[i], doctest::Approx(fitr2_2[i]));
    }
}

#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */