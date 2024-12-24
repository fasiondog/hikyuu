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
 * @defgroup test_indicator_TA_MAMA test_indicator_TA_MAMA
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_TA_MAMA") {
    KData kdata = getKData("sh000001", KQuery(-40));

    Indicator c = CLOSE(kdata);

    /** @arg 非法参数 */
    CHECK_THROWS(TA_MAMA(c, 0.009));
    CHECK_THROWS(TA_MAMA(c, 0.991));

    /** @arg 正常情况 */
    Indicator result = TA_MAMA(CLOSE(kdata));
    CHECK_EQ(result.name(), "TA_MAMA");
    CHECK_EQ(result.discard(), 32);
    CHECK_EQ(result.size(), kdata.size());
    CHECK_EQ(result.getResultNumber(), 2);
    CHECK_EQ(result[32], doctest::Approx(2414.92181).epsilon(0.0001));
    CHECK_EQ(result.get(32, 1), doctest::Approx(2320.10591).epsilon(0.0001));
    CHECK_EQ(result[39], doctest::Approx(2363.65113).epsilon(0.0001));
    CHECK_EQ(result.get(39, 1), doctest::Approx(2346.14345).epsilon(0.0001));

    /** @arg 计算数据的 discard 不为0 */
    auto data = TA_MA(c, 3);
    CHECK_EQ(data.discard(), 2);
    result = TA_MAMA(data);
    CHECK_EQ(result.size(), kdata.size());
    CHECK_EQ(result.getResultNumber(), 2);
    CHECK_EQ(result.discard(), 34);
    CHECK_EQ(result[34], doctest::Approx(2432.51862).epsilon(0.0001));
    CHECK_EQ(result.get(34, 1), doctest::Approx(2274.052997).epsilon(0.0001));
    CHECK_EQ(result[39], doctest::Approx(2365.43706).epsilon(0.0001));
    CHECK_EQ(result.get(39, 1), doctest::Approx(2326.13832).epsilon(0.0001));
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_TA_MAMA_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/TA_MAMA.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-40));
    Indicator x1 = TA_MAMA(CLOSE(kdata));
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
    for (size_t r = 0, total = x1.getResultNumber(); r < total; r++) {
        for (size_t i = x1.discard(); i < x1.size(); ++i) {
            CHECK_EQ(x1[i], doctest::Approx(x2[i]).epsilon(0.00001));
        }
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
