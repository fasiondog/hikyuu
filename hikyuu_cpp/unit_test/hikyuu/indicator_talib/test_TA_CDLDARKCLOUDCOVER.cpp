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
 * @defgroup test_indicator_TA_CDLDARKCLOUDCOVER test_indicator_TA_CDLDARKCLOUDCOVER
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_TA_CDLDARKCLOUDCOVER") {
    KData kdata = getKData("sh000001", KQuery(-30));

    /** @arg 非法 n < 0 */
    CHECK_THROWS(TA_CDLDARKCLOUDCOVER(kdata, -0.1));

    /** @arg 正常情况 */
    Indicator result = TA_CDLDARKCLOUDCOVER(kdata);
    CHECK_EQ(result.name(), "TA_CDLDARKCLOUDCOVER");
    CHECK_EQ(result.discard(), 11);
    CHECK_EQ(result.size(), kdata.size());
    for (size_t i = 0, len = result.discard(); i < len; ++i) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    CHECK_EQ(result[11], 0.);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_TA_CDLDARKCLOUDCOVER_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/TA_CDLDARKCLOUDCOVER.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = TA_CDLDARKCLOUDCOVER(kdata);
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
