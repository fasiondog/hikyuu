/*
 * test_ABS.cpp
 *
 *  Created on: 2019年4月2日
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/IC.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/MA.h>

using namespace hku;

/**
 * @defgroup test_indicator_IC test_indicator_IC
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_IC") {
    StockManager& sm = StockManager::instance();
    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];

    KQuery query = KQuery(-200);
    Indicator result = IC(stks, query, 1, ref_stk)(MA(CLOSE()));
    // HKU_INFO("{}", result);
    // for (size_t i = 0, total = result.size(); i < total; i++) {
    //     HKU_INFO("{}: {}", i, result[i]);
    // }

    // PriceList a;
    // for (int i = 0; i < 10; ++i) {
    //     a.push_back(-i);
    // }

    // Indicator data = PRICELIST(a);

    // result = ABS(data);
    // CHECK_UNARY(result.name() == "ABS");
    // CHECK_UNARY(result.discard() == 0);
    // for (int i = 0; i < 10; ++i) {
    //     CHECK_UNARY(result[i] == -data[i]);
    // }

    // result = ABS(-11);
    // CHECK_UNARY(result.size() == 1);
    // CHECK_UNARY(result.discard() == 0);
    // CHECK_UNARY(result[0] == 11);
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_IC_benchmark") {
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
TEST_CASE("test_IC_export") {
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
