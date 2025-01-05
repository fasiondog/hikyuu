/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-23
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator_talib/ta_crt.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_TA_COS test_indicator_TA_COS
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_TA_COS") {
    Indicator result;

    PriceList a;
    for (int i = 0; i < 10; ++i) {
        a.push_back(i / 10);
    }

    Indicator data = PRICELIST(a);

    result = TA_COS(data);
    CHECK_EQ(result.name(), "TA_COS");
    CHECK_EQ(result.discard(), 0);
    for (int i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], std::cos(data[i]));
    }

    result = TA_COS(-0.1);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], doctest::Approx(std::cos(-0.1)));

    /** @arg 计算数据的 discard 不为0 */
    data = TA_MA(getKData("sz000001", KQuery(-50)).close());
    CHECK_EQ(data.discard(), 29);
    result = TA_COS(data);
    CHECK_EQ(result.name(), "TA_COS");
    CHECK_EQ(result.discard(), data.discard());
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result[29], doctest::Approx(-0.74426).epsilon(0.00001));
    CHECK_EQ(result[49], doctest::Approx(-0.78709).epsilon(0.00001));
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_TA_COS_benchmark") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0));
    Indicator c = kdata.close();
    int cycle = 1000;  // 测试循环次数

    {
        BENCHMARK_TIME_MSG(test_TA_COS_benchmark, cycle, fmt::format("data len: {}", c.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator ind = TA_COS();
            Indicator result = ind(c);
        }
    }
}
#endif

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_TA_COS_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/TA_COS.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = TA_COS(CLOSE(kdata));
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

    CHECK_EQ(x2.name(), "TA_COS");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = x1.discard(); i < x1.size(); ++i) {
        if (std::isnan(x1[i])) {
            CHECK_UNARY(std::isnan(x2[i]));
        } else {
            CHECK_EQ(x1[i], doctest::Approx(x2[i]));
        }
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
