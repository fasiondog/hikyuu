/*
 * test_ABS.cpp
 *
 *  Created on: 2019-4-2
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

/** @par Test points */
TEST_CASE("test_IC") {
    StockManager& sm = StockManager::instance();
    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQuery(-100);
    KData ref_k = ref_stk.getKData(query);
    Indicator result;

    /** @arg An invalid n is passed */
    CHECK_THROWS_AS(IC(MA(CLOSE()), stks, -1), std::exception);

    /** @arg n = 1 */
    result = IC(stks, 1)(MA(CLOSE()))(ref_k);
    CHECK_EQ(result.name(), "IC");
    CHECK_EQ(result.size(), ref_k.size());
    CHECK_EQ(result.discard(), 21);
    CHECK_EQ(result[21], -1.);
    CHECK_EQ(result[22], doctest::Approx(0.8));
    CHECK_EQ(result[99], 0.5);

    /** @arg An empty stks is passed */
    result = IC(MA(CLOSE()), StockList(), 1)(ref_k);
    CHECK_EQ(result.name(), "IC");
    CHECK_UNARY(!result.empty());
    CHECK_EQ(result.size(), ref_k.size());
    CHECK_EQ(result.discard(), result.size());

    /** @arg The passed stks is insufficient, 2 or more are needed */
    result = IC(MA(CLOSE()), {sm["sh600004"]}, 1)(ref_k);
    CHECK_EQ(result.name(), "IC");
    CHECK_UNARY(!result.empty());
    CHECK_EQ(result.size(), ref_k.size());
    CHECK_EQ(result.discard(), result.size());

    /** @arg The data length of ref_stk is insufficient */
    result = IC(MA(CLOSE()), stks, 1)(ref_stk.getKData(KQuery(-1)));
    CHECK_EQ(result.name(), "IC");
    CHECK_UNARY(!result.empty());
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.discard(), result.size());

    /** @arg The passed stks contains a null stock, so its actual length is less than 2 */
    result = IC(MA(CLOSE()), {sm["sh600004"], Stock()}, 1)(ref_stk.getKData(KQuery(-2)));
    CHECK_EQ(result.name(), "IC");
    CHECK_UNARY(!result.empty());
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result.discard(), 2);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_UNARY(std::isnan(result[1]));

    /** @arg The passed stks length is 2 and the query length is 2 */
    result = IC(CLOSE(), {sm["sh600004"], sm["sh600005"]}, 1)(ref_stk.getKData(KQuery(-2)));
    CHECK_EQ(result.name(), "IC");
    CHECK_UNARY(!result.empty());
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result.discard(), 1);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_EQ(result[1], doctest::Approx(-1.0));

    // The strict mode
    result =
      IC(CLOSE(), {sm["sh600004"], sm["sh600005"]}, 1, true, true)(ref_stk.getKData(KQuery(-2)));
    CHECK_EQ(result.name(), "IC");
    CHECK_UNARY(!result.empty());
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], doctest::Approx(-1.0));
    CHECK_UNARY(std::isnan(result[1]));

    /** @arg The normal execution */
    result = IC(CLOSE(), stks, 1)(ref_k);
    CHECK_EQ(result.name(), "IC");
    CHECK_UNARY(!result.empty());
    CHECK_EQ(result.size(), ref_k.size());
    CHECK_EQ(result.discard(), 1);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_EQ(result[3], doctest::Approx(0.4));
    CHECK_EQ(result[99], doctest::Approx(0.5));

    // The strict mode
    result = IC(CLOSE(), stks, 1, true, true)(ref_k);
    CHECK_EQ(result.name(), "IC");
    CHECK_UNARY(!result.empty());
    CHECK_EQ(result.size(), ref_k.size());
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], doctest::Approx(0.8));
    CHECK_EQ(result[2], doctest::Approx(0.4));
    CHECK_EQ(result[98], doctest::Approx(0.5));
    CHECK_EQ(result[99], Null<Indicator::value_t>());
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

    int cycle = 100;  // Test loop count

    {
        BENCHMARK_TIME_MSG(test_IC_benchmark, cycle, fmt::format("data len: {}", ref_k.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator ind = IC(MA(CLOSE()), stks, 1)(ref_k);
        }
    }
}
#endif

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_IC_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/IC.xml";
    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];

    KQuery query = KQuery(-200);
    Indicator x1 = IC(stks, 1)(MA(CLOSE()))(ref_stk.getKData(query));

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
