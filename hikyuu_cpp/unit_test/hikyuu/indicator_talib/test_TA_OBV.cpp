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
 * @defgroup test_indicator_TA_OBV test_indicator_TA_OBV
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_TA_OBV") {
    KData kdata = getKData("sz000001", KQuery(-30));
    Indicator result = TA_OBV(kdata);
    CHECK_EQ(result.name(), "TA_OBV");
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), kdata.size());
    CHECK_EQ(result[0], doctest::Approx(199574).epsilon(0.00001));
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_TA_OBV_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/TA_OBV.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = TA_OBV(kdata);
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
