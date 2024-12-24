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
 * @defgroup test_indicator_TA_STDDEV test_indicator_TA_STDDEV
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_TA_STDDEV") {
    KData kdata = getKData("sh000001", KQuery(-10));
    Indicator c = CLOSE(kdata);

    /** @arg 非法参数 */
    CHECK_THROWS(TA_STDDEV(c, 1));
    CHECK_THROWS(TA_STDDEV(c, 100001));
    CHECK_THROWS(TA_STDDEV(c, 2, Null<double>()));

    /** @arg 正常情况 */
    Indicator result = TA_STDDEV(c, 2);
    CHECK_EQ(result.name(), "TA_STDDEV");
    CHECK_EQ(result.discard(), 1);
    CHECK_EQ(result.size(), kdata.size());
    CHECK_EQ(result.getResultNumber(), 1);
    CHECK_EQ(result[1], doctest::Approx(1.24449).epsilon(0.0001));
    CHECK_EQ(result[9], doctest::Approx(3.662).epsilon(0.0001));

    /** @arg 计算数据的 discard 不为0 */
    auto data = TA_MA(c, 3);
    CHECK_EQ(data.discard(), 2);
    result = TA_STDDEV(data, 2);
    CHECK_EQ(result.size(), kdata.size());
    CHECK_EQ(result.discard(), 3);
    CHECK_EQ(result[3], doctest::Approx(2.00516).epsilon(0.0001));
    CHECK_EQ(result[9], doctest::Approx(10.15916).epsilon(0.0001));
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_TA_STDDEV_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/TA_STDDEV.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = TA_STDDEV(CLOSE(kdata));
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
