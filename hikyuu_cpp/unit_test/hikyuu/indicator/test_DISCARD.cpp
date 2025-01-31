/*
 * test_DISCARD.cpp
 *
 *  Created on: 2019年4月2日
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/DISCARD.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_DISCARD test_indicator_DISCARD
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_DISCARD") {
    auto k = getKData("SH000001", KQuery(-20));
    auto c = k.close();
    REQUIRE(c.size() == 20);
    REQUIRE(c.discard() == 0);
    auto result = DISCARD(c, 5);
    CHECK_EQ(result.size(), 20);
    CHECK_EQ(result.discard(), 5);
    CHECK_EQ(result.name(), "DISCARD");
    for (size_t i = 0; i < result.discard(); ++i) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = result.discard(); i < result.size(); ++i) {
        CHECK_EQ(result[i], c[i]);
    }

    result = result(k);
    CHECK_EQ(result.size(), 20);
    CHECK_EQ(result.discard(), 5);
    CHECK_EQ(result.name(), "DISCARD");
    for (size_t i = 0; i < result.discard(); ++i) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = result.discard(); i < result.size(); ++i) {
        CHECK_EQ(result[i], c[i]);
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_DISCARD_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/DISCARD.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = DISCARD(CLOSE(kdata), 5);
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

    CHECK_EQ(x1.name(), x2.name());
    CHECK_EQ(x2.name(), "DISCARD");
    CHECK_EQ(x2.discard(), 5);
    CHECK_UNARY(x1.size() == x2.size());
    CHECK_UNARY(x1.discard() == x2.discard());
    CHECK_UNARY(x1.getResultNumber() == x2.getResultNumber());
    for (size_t i = 0; i < x2.discard(); ++i) {
        CHECK_UNARY(std::isnan(x2[i]));
    }
    for (size_t i = x2.discard(); i < x2.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]).epsilon(0.00001));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
