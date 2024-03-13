/*
 * test_ABS.cpp
 *
 *  Created on: 2019年4月2日
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/factor/MultiFactorBase.h>

using namespace hku;

/**
 * @defgroup test_indicator_ABS test_indicator_ABS
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_MultiFactor") {}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_MultiFactor_benchmark") {
    // Stock stock = getStock("sh000001");
    // KData kdata = stock.getKData(KQuery(0));
    // Indicator c = kdata.close();
    // int cycle = 1000;  // 测试循环次数

    // {
    //     BENCHMARK_TIME_MSG(test_ABS_benchmark, cycle, fmt::format("data len: {}", c.size()));
    //     SPEND_TIME_CONTROL(false);
    //     for (int i = 0; i < cycle; i++) {
    //         Indicator ind = ABS();
    //         Indicator result = ind(c);
    //     }
    // }
}
#endif

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_MultiFactor_export") {
    // StockManager& sm = StockManager::instance();
    // string filename(sm.tmpdir());
    // filename += "/ABS.xml";

    // Stock stock = sm.getStock("sh000001");
    // KData kdata = stock.getKData(KQuery(-20));
    // Indicator x1 = ABS(CLOSE(kdata));
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

    // CHECK_UNARY(x1.size() == x2.size());
    // CHECK_UNARY(x1.discard() == x2.discard());
    // CHECK_UNARY(x1.getResultNumber() == x2.getResultNumber());
    // for (size_t i = 0; i < x1.size(); ++i) {
    //     CHECK_EQ(x1[i], doctest::Approx(x2[i]).epsilon(0.00001));
    // }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
