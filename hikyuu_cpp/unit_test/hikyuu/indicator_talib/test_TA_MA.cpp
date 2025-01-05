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
 * @defgroup test_indicator_TA_MA test_indicator_TA_MA
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_TA_MA") {
    KData kdata = getKData("sh000001", KQuery(-10));
    Indicator c = CLOSE(kdata);

    // /** @arg 非法 n < 1 || n > 100000 */
    CHECK_THROWS(TA_MA(c, 0));
    CHECK_THROWS(TA_MA(c, 2, -1));
    CHECK_THROWS(TA_MA(c, 2, 9));

    /** @arg 正常情况 */
    Indicator result = TA_MA(CLOSE(kdata), 3);
    CHECK_EQ(result.name(), "TA_MA");
    CHECK_EQ(result.discard(), 2);
    CHECK_EQ(result.size(), kdata.size());
    CHECK_EQ(result[2], doctest::Approx(2390.94766).epsilon(0.0001));
    CHECK_EQ(result[9], doctest::Approx(2339.93266).epsilon(0.0001));
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_TA_MA_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/TA_MA.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = TA_MA(CLOSE(kdata));
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
    for (size_t i = x1.discard(); i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]).epsilon(0.00001));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
