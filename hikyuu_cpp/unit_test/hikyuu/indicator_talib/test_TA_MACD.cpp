/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-18
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator_talib/ta_crt.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/CVAL.h>

using namespace hku;

/**
 * @defgroup test_indicator_TA_MACD test_indicator_TA_MACD
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_TA_MACD") {
    KData kdata = getKData("sh000001", KQuery(-35));
    Indicator c = CLOSE(kdata);

    // /** @arg 非法参数 */
    CHECK_THROWS(TA_MACD(c, 1, 26, 9));
    CHECK_THROWS(TA_MACD(c, 100001, 26, 9));
    CHECK_THROWS(TA_MACD(c, 12, 1, 9));
    CHECK_THROWS(TA_MACD(c, 12, 100001, 9));
    CHECK_THROWS(TA_MACD(c, 12, 26, 0));
    CHECK_THROWS(TA_MACD(c, 12, 26, 100001));

    /** @arg 正常情况 */
    Indicator result = TA_MACD(CLOSE(kdata), 3);
    CHECK_EQ(result.name(), "TA_MACD");
    CHECK_EQ(result.size(), kdata.size());
    CHECK_EQ(result.getResultNumber(), 3);
    CHECK_EQ(result.discard(), 33);
    CHECK_EQ(result[33], doctest::Approx(-63.45293).epsilon(0.0001));
    CHECK_EQ(result.get(33, 1), doctest::Approx(-50.63308).epsilon(0.0001));
    CHECK_EQ(result.get(33, 2), doctest::Approx(-12.819849).epsilon(0.0001));
    CHECK_EQ(result[34], doctest::Approx(-69.42609).epsilon(0.0001));
    CHECK_EQ(result.get(34, 1), doctest::Approx(-54.391689).epsilon(0.0001));
    CHECK_EQ(result.get(34, 2), doctest::Approx(-15.034402).epsilon(0.0001));
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_TA_MACD_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/TA_MACD.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-35));
    Indicator x1 = TA_MACD(CLOSE(kdata));
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
    CHECK_UNARY(x1.size() == x2.size());
    CHECK_UNARY(x1.discard() == x2.discard());
    CHECK_UNARY(x1.getResultNumber() == x2.getResultNumber());
    for (size_t r = 0, total = x1.getResultNumber(); r < total; ++r) {
        for (size_t i = x1.discard(); i < x1.size(); ++i) {
            CHECK_EQ(x1.get(i, r), doctest::Approx(x2.get(i, r)).epsilon(0.00001));
        }
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
