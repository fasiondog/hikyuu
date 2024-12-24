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
 * @defgroup test_indicator_TA_STOCHRSI test_indicator_TA_STOCHRSI
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_TA_STOCHRSI") {
    KData kdata = getKData("sh000001", KQuery(-15));
    Indicator c = CLOSE(kdata);

    /** @arg 非法参数 */
    CHECK_THROWS(TA_STOCHRSI(c, 1, 5, 3, 0));
    CHECK_THROWS(TA_STOCHRSI(c, 100001, 5, 3, 0));
    CHECK_THROWS(TA_STOCHRSI(c, 14, 0, 3, 0));
    CHECK_THROWS(TA_STOCHRSI(c, 14, 100001, 3, 0));
    CHECK_THROWS(TA_STOCHRSI(c, 14, 5, 0, 0));
    CHECK_THROWS(TA_STOCHRSI(c, 14, 5, 100001, 0));
    CHECK_THROWS(TA_STOCHRSI(c, 14, 5, 3, -1));
    CHECK_THROWS(TA_STOCHRSI(c, 14, 5, 3, 9));

    /** @arg 正常情况 */
    Indicator result = TA_STOCHRSI(c, 2);
    CHECK_EQ(result.name(), "TA_STOCHRSI");
    CHECK_EQ(result.discard(), 8);
    CHECK_EQ(result.size(), kdata.size());
    CHECK_EQ(result.getResultNumber(), 2);
    CHECK_EQ(result[8], doctest::Approx(100).epsilon(0.0001));
    CHECK_EQ(result.get(8, 1), doctest::Approx(76.83963).epsilon(0.0001));
    CHECK_EQ(result[14], doctest::Approx(6.863976).epsilon(0.0001));
    CHECK_EQ(result.get(14, 1), doctest::Approx(17.41548).epsilon(0.0001));

    /** @arg 计算数据的 discard 不为0 */
    auto data = TA_MA(c, 3);
    CHECK_EQ(data.discard(), 2);
    result = TA_STOCHRSI(data, 2);
    CHECK_EQ(result.size(), kdata.size());
    CHECK_EQ(result.getResultNumber(), 2);
    CHECK_EQ(result.discard(), 10);
    CHECK_EQ(result[10], doctest::Approx(27.60069).epsilon(0.0001));
    CHECK_EQ(result.get(10, 1), doctest::Approx(42.53356).epsilon(0.0001));
    CHECK_EQ(result[14], doctest::Approx(0).epsilon(0.0001));
    CHECK_EQ(result.get(14, 1), doctest::Approx(4.32746).epsilon(0.0001));
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_TA_STOCHRSI_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/TA_STOCHRSI.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = TA_STOCHRSI(CLOSE(kdata));
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
            CHECK_EQ(x1[i], doctest::Approx(x2[i]).epsilon(0.00001));
        }
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
