/*
 * test_ABS.cpp
 *
 *  Created on: 2019年4月2日
 *      Author: fasiondog
 */
#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/ZSCORE.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_ZSCORE test_indicator_ZSCORE
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_ZSCORE") {
    // Indicator result;

    // PriceList a;
    // for (int i = 0; i < 10; ++i) {
    //     a.push_back(5 - i);
    // }

    // vector<price_t> expect = {0., 0., 0., 0., 0., 1., 1., 1., 1., 1.};

    // Indicator data = PRICELIST(a);

    // result = NOT(data);
    // CHECK_EQ(result.name(), "NOT");
    // CHECK_EQ(result.discard(), 0);
    // CHECK_EQ(result[0], 0.0);
    // for (int i = 1; i < 10; ++i) {
    //     CHECK_EQ(result[i], expect[i]);
    // }
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_ZSCORE_benchmark") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0));
    Indicator c = kdata.close();
    int cycle = 1000;  // 测试循环次数

    {
        BENCHMARK_TIME_MSG(test_ZSCORE_benchmark, cycle, fmt::format("data len: {}", c.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator ind = EMA();
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
TEST_CASE("test_ZSCORE_export") {
    // StockManager& sm = StockManager::instance();
    // string filename(sm.tmpdir());
    // filename += "/NOT.xml";

    // Stock stock = sm.getStock("sh000001");
    // KData kdata = stock.getKData(KQuery(-20));
    // Indicator x1 = NOT(CLOSE(kdata));
    // {
    //     std::ofstream ofs(filename);
    //     boost::archive::xml_oarchive oa(ofs);
    //     oa << BOOST_SERIALIZATION_NVP(x1);
    // }

    // Indicator x2;
    // {
    //     std::ifstream ifs(filename);
    //     boost::archive::xml_iarchive ia(ifs);
    //     ia >> BOOST_SERIALIZATION_NVP(x2);
    // }

    // CHECK_EQ(x1.name(), x2.name());
    // CHECK_EQ(x1.size(), x2.size());
    // CHECK_EQ(x1.discard(), x2.discard());
    // CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    // for (size_t i = 0; i < x1.size(); ++i) {
    //     CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    // }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
