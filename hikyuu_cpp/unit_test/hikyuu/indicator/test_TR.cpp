/*
 * test_TR.cpp
 *
 *  Created on: 2019-3-29
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/TR.h>

using namespace hku;

/**
 * @defgroup test_indicator_TR test_indicator_TR
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_TR") {
    KData k;

    /** @arg k 为空 */
    auto ret = TR(k);
    CHECK_EQ(ret.size(), 0);
    CHECK_EQ(ret.discard(), 0);
    CHECK_EQ(ret.name(), "TR");

    /** @arg 正常k */
    k = getKData("sh000001", KQueryByIndex(-10));
    REQUIRE(k.size() > 0);
    ret = ret(k);
    CHECK_EQ(ret.name(), "TR");
    CHECK_EQ(ret.size(), k.size());
    CHECK_EQ(ret.discard(), 1);
    PriceList expect = {0.0,      39.02399,  31.32,  24.671,    30.29199,
                        92.95299, 90.144999, 42.014, 35.516999, 23.07400};
    for (size_t i = 1; i < ret.size(); ++i) {
        CHECK_EQ(ret[i], doctest::Approx(expect[i]).epsilon(0.0001));
    }
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_TR_benchmark") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0));
    Indicator c = kdata.close();
    int cycle = 1000;  // 测试循环次数

    {
        BENCHMARK_TIME_MSG(test_TR_benchmark, cycle, fmt::format("data len: {}", c.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator ind = TR();
            Indicator result = ind(kdata);
        }
    }
}
#endif

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_TR_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/TR.xml";

    KData k = getStock("SH600000").getKData(KQuery(-10));
    Indicator x1 = TR(k);
    x1.setContext(k);

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

    CHECK_EQ(x2.name(), "TR");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = x1.discard(); i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
