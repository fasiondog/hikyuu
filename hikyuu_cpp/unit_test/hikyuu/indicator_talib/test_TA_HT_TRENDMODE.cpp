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
 * @defgroup test_indicator_TA_HT_TRENDMODE test_indicator_TA_HT_TRENDMODE
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_TA_HT_TRENDMODE") {
    KData kdata = getKData("sh000001", KQuery(-10));
    Indicator c = CLOSE(kdata);

    /** @arg KData 为空 */
    Indicator result = TA_HT_TRENDMODE(KData().close());
    CHECK_EQ(result.name(), "TA_HT_TRENDMODE");
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), 0);
    CHECK_EQ(result.getResultNumber(), 1);

    /** @arg KData 长度小于默认参数抛弃数量 */
    result = TA_HT_TRENDMODE(c);
    CHECK_EQ(result.name(), "TA_HT_TRENDMODE");
    CHECK_EQ(result.discard(), 10);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.getResultNumber(), 1);

    /** @arg 正常情况 */
    kdata = getKData("sh000001", KQuery(-70));
    result = TA_HT_TRENDMODE(CLOSE(kdata));
    CHECK_EQ(result.name(), "TA_HT_TRENDMODE");
    CHECK_EQ(result.discard(), 63);
    CHECK_EQ(result.size(), kdata.size());
    CHECK_EQ(result.getResultNumber(), 1);
    CHECK_EQ(result[63], 1.0);
    CHECK_EQ(result[69], 1.0);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_TA_HT_TRENDMODE_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/TA_HT_TRENDMODE.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-70));
    Indicator x1 = TA_HT_TRENDMODE(CLOSE(kdata));
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
