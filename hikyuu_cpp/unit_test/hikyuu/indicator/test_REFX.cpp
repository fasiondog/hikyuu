/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-22
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/REFX.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_REFX test_indicator_REFX
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_REFX") {
    Indicator result;

    PriceList a;
    for (int i = 0; i < 10; ++i) {
        a.push_back(i);
    }

    Indicator data = PRICELIST(a);

    /** @arg n = 0 */
    result = REFX(data, 0);
    CHECK_EQ(result.discard(), 0);
    for (int i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], data[i]);
    }

    /** @arg n = 1 */
    result = REFX(data, 1);
    CHECK_EQ(result.discard(), 1);
    for (int i = 1; i < 10; ++i) {
        CHECK_EQ(result[i], data[i - 1]);
    }

    /** @arg n = 9 */
    result = REFX(data, 9);
    CHECK_EQ(result.discard(), 9);
    CHECK_EQ(result[9], data[0]);

    /** @arg n = 10 */
    result = REFX(data, 10);
    CHECK_EQ(result.discard(), 10);

    /** @arg n = -1 */
    data = PRICELIST(a, 2);
    Indicator::value_t null_value = Null<Indicator::value_t>();
    auto expect = PRICELIST({null_value, 2, 3, 4, 5, 6, 7, 8, 9, null_value}, 1);
    result = REFX(data, -1);
    check_indicator(result, expect);

    data = PRICELIST(a);
    expect = PRICELIST({1, 2, 3, 4, 5, 6, 7, 8, 9, null_value});
    result = REFX(data, -1);
    check_indicator(result, expect);

    /** @arg n = -2 */
    data = PRICELIST(a);
    expect = PRICELIST({2, 3, 4, 5, 6, 7, 8, 9, null_value, null_value});
    result = REFX(data, -2);
    check_indicator(result, expect);

    data = PRICELIST(a, 2);
    expect = PRICELIST({2, 3, 4, 5, 6, 7, 8, 9, null_value, null_value});
    result = REFX(data, -2);
    check_indicator(result, expect);

    data = PRICELIST(a, 3);
    expect = PRICELIST({null_value, 3, 4, 5, 6, 7, 8, 9, null_value, null_value}, 1);
    result = REFX(data, -2);
    check_indicator(result, expect);

    data = PRICELIST(a, 9);
    expect = PRICELIST({null_value, null_value, null_value, null_value, null_value, null_value,
                        null_value, 9, null_value, null_value},
                       7);
    result = REFX(data, -2);
    check_indicator(result, expect);

    /** @arg n = -9 */
    data = PRICELIST(a);
    expect = PRICELIST({9, null_value, null_value, null_value, null_value, null_value, null_value,
                        null_value, null_value, null_value});
    result = REFX(data, -9);
    check_indicator(result, expect);

    data = PRICELIST(a, 3);
    expect = PRICELIST({9, null_value, null_value, null_value, null_value, null_value, null_value,
                        null_value, null_value, null_value});
    result = REFX(data, -9);
    check_indicator(result, expect);

    data = PRICELIST(a, 9);
    expect = PRICELIST({9, null_value, null_value, null_value, null_value, null_value, null_value,
                        null_value, null_value, null_value});
    result = REFX(data, -9);
    check_indicator(result, expect);

    /** @arg n = -10 */
    data = PRICELIST(a);
    result = REFX(data, -10);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.discard(), 10);

    data = PRICELIST(a, 2);
    result = REFX(data, -10);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.discard(), 10);

    /** @arg n = -11 */
    data = PRICELIST(a);
    result = REFX(data, -11);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.discard(), 10);

    data = PRICELIST(a, 2);
    result = REFX(data, -11);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.discard(), 10);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_REFX_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/REFX.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = REFX(CLOSE(kdata), -1);
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

    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    check_indicator(x1, x2);
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
