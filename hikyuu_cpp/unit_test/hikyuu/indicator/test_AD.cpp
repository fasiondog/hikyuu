/*
 * test_AD.cpp
 *
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/AD.h>

using namespace hku;

/**
 * @defgroup test_indicator_AD test_indicator_AD
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_AD") {
    StockManager& sm = StockManager::instance();

    Stock stk = getStock("SH600004");
    KQuery query = KQueryByIndex(-10);
    KData k = stk.getKData(query);

    Indicator ad = AD(k);
    CHECK_EQ(ad.name(), "AD");
    CHECK_EQ(ad.size(), k.size());
    CHECK_EQ(ad.discard(), 0);
    CHECK_EQ(ad[0], 0);
    CHECK_EQ(ad[1], doctest::Approx(458.65).epsilon(0.1));
    CHECK_EQ(ad[2], doctest::Approx(63.99).epsilon(0.1));
    CHECK_EQ(ad[5], doctest::Approx(30.77).epsilon(0.1));
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_AD_benchmark") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0));
    int cycle = 1000;  // 测试循环次数

    {
        BENCHMARK_TIME_MSG(test_AD_benchmark, cycle, fmt::format("data len: {}", kdata.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator ind = AD();
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
TEST_CASE("test_AD_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/AD.xml";

    KData k = getStock("SH600000").getKData(KQuery(-10));
    Indicator x1 = AD(k);
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

    CHECK_EQ(x2.name(), "AD");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
