/*
 * test_IKData.cpp
 *
 *  Created on: 2013-2-12
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/RECOVER.h>

using namespace hku;

/**
 * @defgroup test_indicator_RECOVER test_indicator_RECOVER
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_RECOVER") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData kdata;
    KQuery query;

    /** @arg 非法 ind */
    CHECK_THROWS_AS(RECOVER_FORWARD(MA()), std::exception);
    CHECK_THROWS_AS(RECOVER_BACKWARD(MA()), std::exception);
    CHECK_THROWS_AS(RECOVER_EQUAL_FORWARD(MA()), std::exception);
    CHECK_THROWS_AS(RECOVER_EQUAL_BACKWARD(MA()), std::exception);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_RECOVER_export") {
    // StockManager& sm = StockManager::instance();
    // string filename(sm.tmpdir());
    // filename += "/IKDATA.xml";

    // Stock stock = sm.getStock("sh000001");
    // KData kdata = stock.getKData(KQuery(-20));
    // Indicator ma1 = KDATA(kdata);
    // {
    //     std::ofstream ofs(filename);
    //     boost::archive::xml_oarchive oa(ofs);
    //     oa << BOOST_SERIALIZATION_NVP(ma1);
    // }

    // Indicator ma2;
    // {
    //     std::ifstream ifs(filename);
    //     boost::archive::xml_iarchive ia(ifs);
    //     ia >> BOOST_SERIALIZATION_NVP(ma2);
    // }

    // CHECK_EQ(ma1.size(), ma2.size());
    // CHECK_EQ(ma1.discard(), ma2.discard());
    // CHECK_EQ(ma1.getResultNumber(), ma2.getResultNumber());
    // for (size_t i = 0; i < ma1.size(); ++i) {
    //     CHECK_EQ(ma1.get(i, 0), doctest::Approx(ma2.get(i, 0)));
    //     CHECK_EQ(ma1.get(i, 1), doctest::Approx(ma2.get(i, 1)));
    //     CHECK_EQ(ma1.get(i, 2), doctest::Approx(ma2.get(i, 2)));
    //     CHECK_EQ(ma1.get(i, 3), doctest::Approx(ma2.get(i, 3)));
    //     CHECK_EQ(ma1.get(i, 4), doctest::Approx(ma2.get(i, 4)));
    //     CHECK_EQ(ma1.get(i, 5), doctest::Approx(ma2.get(i, 5)));
    // }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
