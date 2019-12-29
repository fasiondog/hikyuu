/*
 * test_CEILING.cpp
 *
 *  Created on: 2019-4-14
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/CEILING.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_CEILING test_indicator_CEILING
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_CEILING") {
    Indicator result;

    PriceList a;
    a.push_back(13.2);
    a.push_back(0.3);
    a.push_back(1.5);

    Indicator data = PRICELIST(a);

    result = CEILING(data);
    CHECK_EQ(result.name(), "CEILING");
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(result[0], doctest::Approx(14));
    CHECK_EQ(result[1], doctest::Approx(1));
    CHECK_EQ(result[2], doctest::Approx(2));

    result = CEILING(-11.15);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], doctest::Approx(-11));
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_CEILING_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/CEILING.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = CEILING(CLOSE(kdata));
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

    CHECK_EQ(x2.name(), "CEILING");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
