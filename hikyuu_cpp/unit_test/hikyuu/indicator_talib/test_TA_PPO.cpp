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
 * @defgroup test_indicator_TA_PPO test_indicator_TA_PPO
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_TA_PPO") {
    KData kdata = getKData("sh000001", KQuery(-30));
    Indicator c = CLOSE(kdata);

    /** @arg 非法参数 */
    CHECK_THROWS(TA_PPO(c, 1, 26, 0));
    CHECK_THROWS(TA_PPO(c, 100001, 26, 0));
    CHECK_THROWS(TA_PPO(c, 12, 1, 0));
    CHECK_THROWS(TA_PPO(c, 12, 100001, 0));
    CHECK_THROWS(TA_PPO(c, 12, 26, -1));
    CHECK_THROWS(TA_PPO(c, 12, 26, 9));

    // /** @arg 正常情况 */
    Indicator result = TA_PPO(CLOSE(kdata));
    CHECK_EQ(result.name(), "TA_PPO");
    CHECK_EQ(result.discard(), 25);
    CHECK_EQ(result.size(), kdata.size());
    CHECK_EQ(result.getResultNumber(), 1);
    CHECK_EQ(result[25], doctest::Approx(-1.571039).epsilon(0.0001));
    CHECK_EQ(result[29], doctest::Approx(-2.557277).epsilon(0.0001));

    // /** @arg 计算数据的 discard 不为0 */
    auto data = TA_MA(c, 3);
    CHECK_EQ(data.discard(), 2);
    result = TA_PPO(data);
    CHECK_EQ(result.size(), kdata.size());
    CHECK_EQ(result.discard(), 27);
    CHECK_EQ(result.getResultNumber(), 1);
    CHECK_EQ(result[27], doctest::Approx(-1.93459).epsilon(0.0001));
    CHECK_EQ(result[29], doctest::Approx(-2.42093).epsilon(0.0001));
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_TA_PPO_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/TA_PPO.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-30));
    Indicator x1 = TA_PPO(CLOSE(kdata));
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
