/*
 * test_LASTVALUE.cpp
 *
 *  Created on: 2019年4月2日
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/LASTVALUE.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_LASTVALUE test_indicator_LASTVALUE
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_LASTVALUE") {
    Indicator result;

    /** @arg ignore_discard=false */
    Indicator::value_t nan = Null<Indicator::value_t>();
    Indicator data = PRICELIST(PriceList{nan, 2, 3, 4, 5}, 1);
    Indicator expect = PRICELIST(PriceList{nan, 5, 5, 5, 5}, 1);

    result = LASTVALUE(data, false);
    CHECK_EQ(result.name(), "LASTVALUE");
    CHECK_EQ(result.discard(), expect.discard());
    CHECK_EQ(result.size(), expect.size());
    for (size_t i = 0; i < result.discard(); ++i) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = result.discard(); i < data.size(); ++i) {
        CHECK_EQ(result[i], expect[i]);
    }

    /** @arg ignore_discard=true */
    expect = PRICELIST(PriceList{5, 5, 5, 5, 5});

    result = LASTVALUE(data, true);
    CHECK_EQ(result.name(), "LASTVALUE");
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), expect.size());
    for (size_t i = result.discard(); i < data.size(); ++i) {
        CHECK_EQ(result[i], expect[i]);
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_LASTVALUE_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/LASTVALUE.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = LASTVALUE(CLOSE(kdata));
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
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]).epsilon(0.00001));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
