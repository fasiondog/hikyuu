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
 * @defgroup test_indicator_TA_SAR test_indicator_TA_SAR
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_TA_SAR") {
    KData kdata = getKData("sz000001", KQuery(-10));

    /** @arg 非法参数 */
    CHECK_THROWS(TA_SAR(kdata, -1.0));
    CHECK_THROWS(TA_SAR(kdata, 0.02, -1.0));

    /** @arg KData 为空 */
    Indicator result = TA_SAR(KData());
    CHECK_EQ(result.name(), "TA_SAR");
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), 0);

    /** @arg KData 长度小于默认参数抛弃数量 */
    result = TA_SAR(getKData("sh000001", KQuery(-1)));
    CHECK_EQ(result.discard(), 1);
    CHECK_EQ(result.size(), 1);

    /** @arg 正常情况 */
    result = TA_SAR(kdata);
    CHECK_EQ(result.name(), "TA_SAR");
    CHECK_EQ(result.discard(), 1);
    CHECK_EQ(result.size(), kdata.size());
    CHECK_EQ(result.getResultNumber(), 1);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_EQ(result[1], doctest::Approx(15.72).epsilon(0.00001));
    CHECK_EQ(result[9], doctest::Approx(15.4958).epsilon(0.00001));
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_TA_SAR_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/TA_SAR.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = TA_SAR(kdata);
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
