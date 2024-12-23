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
 * @defgroup test_indicator_TA_ADOSC test_indicator_TA_ADOSC
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_TA_ADOSC") {
    KData kdata = getKData("sh000001", KQuery(-20));

    /** @arg 非法参数 */
    CHECK_THROWS(TA_ADOSC(kdata, -1));
    CHECK_THROWS(TA_ADOSC(kdata, 2, -1));
    CHECK_THROWS(TA_ADOSC(kdata, 100001));
    CHECK_THROWS(TA_ADOSC(kdata, 2, 100001));

    /** @arg KData 为空 */
    Indicator result = TA_ADOSC(KData());
    CHECK_EQ(result.name(), "TA_ADOSC");
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), 0);

    /** @arg KData 长度小于默认参数抛弃数量 */
    result = TA_ADOSC(getKData("sh000001", KQuery(-8)));
    CHECK_EQ(result.discard(), 8);
    CHECK_EQ(result.size(), 8);

    /** @arg 正常情况 */
    result = TA_ADOSC(kdata);
    CHECK_EQ(result.name(), "TA_ADOSC");
    CHECK_EQ(result.discard(), 9);
    CHECK_EQ(result.size(), kdata.size());
    CHECK_EQ(result.getResultNumber(), 1);
    CHECK_UNARY(std::isnan(result[8]));
    CHECK_EQ(result[9], doctest::Approx(-22528066.68939).epsilon(0.0001));
    CHECK_EQ(result[19], doctest::Approx(-42392914.26395).epsilon(0.0001));
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_TA_ADOSC_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/TA_ADOSC.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = TA_ADOSC(kdata);
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
