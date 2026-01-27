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
    KQuery query = KQuery(-100);
    KData ref_k = ref_stk.getKData(query);
    Indicator result;

    /** @arg 传入非法 n */
    CHECK_THROWS_AS(IC(MA(CLOSE()), stks, ref_stk, -1), std::exception);

    /** @arg 传入的 ref_stk 为 null, 计算绝对收益 */
    result = IC(stks, Stock(), 1)(MA(CLOSE()))(ref_k);
    CHECK_EQ(result.name(), "IC");
    CHECK_EQ(result.size(), ref_k.size());
    CHECK_EQ(result.discard(), 21);
    CHECK_EQ(result[21], -1.);
    CHECK_EQ(result[22], 0.8);
    CHECK_EQ(result[99], 0.5);

    /** @arg 传入空的 stks */
    result = IC(MA(CLOSE()), StockList(), ref_stk, 1)(ref_k);
    CHECK_EQ(result.name(), "IC");
    CHECK_UNARY(!result.empty());
    CHECK_EQ(result.size(), ref_k.size());
    CHECK_EQ(result.discard(), result.size());

    /** @arg 传入的 stks 数量不足，需要大于等于2 */
    result = IC(MA(CLOSE()), {sm["sh600004"]}, ref_stk, 1)(ref_k);
    CHECK_EQ(result.name(), "IC");
    CHECK_UNARY(!result.empty());
    CHECK_EQ(result.size(), ref_k.size());
    CHECK_EQ(result.discard(), result.size());

    /** @arg ref_stk 数据长度不足 */
    result = IC(MA(CLOSE()), stks, ref_stk, 1)(ref_stk.getKData(KQuery(-1)));
    CHECK_EQ(result.name(), "IC");
    CHECK_UNARY(!result.empty());
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.discard(), result.size());

    /** @arg 传入的 stks 中夹杂有 null stock，实际的 stks 长度小于2 */
    result = IC(MA(CLOSE()), {sm["sh600004"], Stock()}, ref_stk, 1)(ref_stk.getKData(KQuery(-2)));
    CHECK_EQ(result.name(), "IC");
    CHECK_UNARY(!result.empty());
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result.discard(), 2);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_UNARY(std::isnan(result[1]));

    /** @arg 传入的 stks 长度为2，query 的长度为2*/
    result =
      IC(CLOSE(), {sm["sh600004"], sm["sh600005"]}, Stock(), 1)(ref_stk.getKData(KQuery(-2)));
    CHECK_EQ(result.name(), "IC");
    CHECK_UNARY(!result.empty());
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result.discard(), 1);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_EQ(result[1], doctest::Approx(-1.0));

    // 严格模式
    result = IC(CLOSE(), {sm["sh600004"], sm["sh600005"]}, Stock(), 1, true,
                true)(ref_stk.getKData(KQuery(-2)));
    CHECK_EQ(result.name(), "IC");
    CHECK_UNARY(!result.empty());
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], doctest::Approx(-1.0));
    CHECK_UNARY(std::isnan(result[1]));

    /** @arg 正常执行 */
    result = IC(CLOSE(), stks, Stock(), 1)(ref_k);
    CHECK_EQ(result.name(), "IC");
    CHECK_UNARY(!result.empty());
    CHECK_EQ(result.size(), ref_k.size());
    CHECK_EQ(result.discard(), 1);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_EQ(result[3], doctest::Approx(0.4));
    CHECK_EQ(result[99], doctest::Approx(0.5));

    // 严格模式
    result = IC(CLOSE(), stks, Stock(), 1, true, true)(ref_k);
    CHECK_EQ(result.name(), "IC");
    CHECK_UNARY(!result.empty());
    CHECK_EQ(result.size(), ref_k.size());
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], doctest::Approx(0.8));
    CHECK_EQ(result[2], doctest::Approx(0.4));
    CHECK_EQ(result[98], doctest::Approx(0.5));
    CHECK_EQ(result[99], Null<Indicator::value_t>());

    /** @arg ref_stock 为空时，和默认为sh000300 时差别 */
    result = IC(CLOSE(), stks, ref_stk, 1)(ref_k);
    CHECK_EQ(result.name(), "IC");
    CHECK_UNARY(!result.empty());
    CHECK_EQ(result.size(), ref_k.size());
    CHECK_EQ(result.discard(), 1);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_EQ(result[3], doctest::Approx(0.35));
    CHECK_EQ(result[99], doctest::Approx(-0.625));
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_IC_benchmark") {
    StockManager& sm = StockManager::instance();
    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQuery(-1000);
    KData ref_k = ref_stk.getKData(query);

    int cycle = 100;  // 测试循环次数

    {
        BENCHMARK_TIME_MSG(test_IC_benchmark, cycle, fmt::format("data len: {}", ref_k.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator ind = IC(MA(CLOSE()), stks, ref_stk, 1)(ref_k);
        }
    }
}
#endif

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_IC_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/IC.xml";
    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];

    KQuery query = KQuery(-200);
    Indicator x1 = IC(stks, ref_stk, 1)(MA(CLOSE()))(ref_stk.getKData(query));

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
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        if (std::isnan(x1[i])) {
            CHECK_UNARY(std::isnan(x2[i]));
        } else {
            CHECK_EQ(x1[i], doctest::Approx(x2[i]).epsilon(0.00001));
        }
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
