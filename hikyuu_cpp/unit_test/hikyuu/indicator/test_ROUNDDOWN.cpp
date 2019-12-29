/*
 * test_ROUNDDOWN.cpp
 *
 *  Created on: 2019-4-14
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/ROUNDDOWN.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_ROUNDDOWN test_indicator_ROUNDDOWN
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_ROUNDDOWN") {
    Indicator result;

    PriceList a;
    a.push_back(1.323);
    a.push_back(0.301);
    a.push_back(0.305);

    Indicator data = PRICELIST(a);

    result = ROUNDDOWN(data, 2);
    CHECK_EQ(result.name(), "ROUNDDOWN");
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(result[0], doctest::Approx(1.32));
    CHECK_EQ(result[1], doctest::Approx(0.30));
    CHECK_EQ(result[2], doctest::Approx(0.30));

    result = ROUNDDOWN(-11.15, 1);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], doctest::Approx(-11.1));
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_ROUNDDOWN_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/ROUNDDOWN.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = ROUNDDOWN(CLOSE(kdata));
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

    CHECK_EQ(x2.name(), "ROUNDDOWN");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
