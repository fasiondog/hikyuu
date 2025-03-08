/*
 * test_BARSSINCE.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-4
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/BARSSINCE.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_BARSSINCE test_indicator_BARSSINCE
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_BARSSINCE") {
    Indicator result;

    PriceList a;
    for (int i = 0; i < 5; ++i) {
        a.push_back(i);
    }

    auto nan = Null<Indicator::value_t>();
    Indicator data = PRICELIST(a);

    result = BARSSINCE(data);
    CHECK_EQ(result.name(), "BARSSINCE");
    CHECK_EQ(result.size(), 5);
    CHECK_EQ(result.discard(), 1);
    check_indicator(result, PRICELIST({nan, 0, 1, 2, 3}, 1));

    result = BARSSINCE(-11);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 0);

    /** @arg 输入数据的dicard位置为 Nan */
    a[1] = Null<price_t>();
    data = PRICELIST(a, 1);
    result = BARSSINCE(data);
    check_indicator(result, PRICELIST({0, nan, 0, 1, 2}, 2));

    /** @arg 输入数据中间含有 Nan */
    a[3] = Null<price_t>();
    data = PRICELIST(a);
    result = BARSSINCE(data);
    check_indicator(result, PRICELIST({0, nan, 0, 1, 2}, 2));
}

/** @par 检测点 */
TEST_CASE("test_BARSSINCEN") {
    Indicator result;

    PriceList a;
    for (int i = 0; i < 5; ++i) {
        a.push_back(i);
    }

    auto nan = Null<Indicator::value_t>();
    Indicator data = PRICELIST(a);

    /** @arg 周期为1, */
    result = BARSSINCEN(data, 1);
    CHECK_EQ(result.name(), "BARSSINCE");
    CHECK_EQ(result.size(), 5);
    CHECK_EQ(result.discard(), 1);
    check_indicator(result, PRICELIST({nan, 0., 0., 0., 0.}, 1));

    /** @arg 周期为3, */
    result = BARSSINCEN(data, 3);
    CHECK_EQ(result.name(), "BARSSINCE");
    CHECK_EQ(result.size(), 5);
    CHECK_EQ(result.discard(), 2);
    check_indicator(result, PRICELIST({nan, nan, 1., 2., 2.}, 2));

    /** @arg 周期为4, */
    result = BARSSINCEN(data, 4);
    CHECK_EQ(result.name(), "BARSSINCE");
    CHECK_EQ(result.size(), 5);
    CHECK_EQ(result.discard(), 3);
    check_indicator(result, PRICELIST({nan, nan, nan, 2., 3.}, 3));

    /** @arg 周期为3, 输入中含有 nan */
    a[2] = nan;
    data = PRICELIST(a);
    result = BARSSINCEN(data, 3);
    CHECK_EQ(result.name(), "BARSSINCE");
    CHECK_EQ(result.size(), 5);
    CHECK_EQ(result.discard(), 2);
    check_indicator(result, PRICELIST({nan, nan, 1., 2., 1.}, 2));
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_BARSSINCE_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/BARSSINCE.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = BARSSINCE(CLOSE(kdata));
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

    CHECK_EQ(x2.name(), "BARSSINCE");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
