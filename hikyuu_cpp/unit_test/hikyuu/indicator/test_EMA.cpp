/*
 * test_EMA.cpp
 *
 *  Created on: 2013-4-10
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/EMA.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/MA.h>

using namespace hku;

/**
 * @defgroup test_indicator_EMA test_indicator_EMA
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_EMA") {
    PriceList d;
    for (int i = 0; i < 30; ++i) {
        d.push_back(i);
    }

    /** @arg n = 1 */
    Indicator data = PRICELIST(d);
    Indicator ema = EMA(data, 1);
    CHECK_EQ(ema.discard(), 0);
    CHECK_EQ(ema.size(), 30);
    CHECK_EQ(ema[0], 0.0);
    CHECK_EQ(ema[10], 10.0);
    CHECK_EQ(ema[29], 29.0);

    /** @arg n = 2 */
    ema = EMA(data, 2);
    CHECK_EQ(ema.discard(), 0);
    CHECK_EQ(ema.size(), 30);
    CHECK_EQ(ema[0], 0.0);
    CHECK_LT(std::fabs(ema[1] - 0.66667), 0.0001);
    CHECK_LT(std::fabs(ema[2] - 1.55556), 0.0001);
    CHECK_LT(std::fabs(ema[3] - 2.51852), 0.0001);
    CHECK_LT(std::fabs(ema[4] - 3.50617), 0.0001);

    /** @arg n = 10 */
    ema = EMA(data, 10);
    CHECK_EQ(ema.discard(), 0);
    CHECK_EQ(ema.size(), 30);
    CHECK_EQ(ema[0], 0.0);
    CHECK_LT(std::fabs(ema[1] - 0.18182), 0.0001);
    CHECK_LT(std::fabs(ema[2] - 0.51240), 0.0001);

    /** @arg operator() */
    Indicator ma = MA(data, 2);
    Indicator expect = EMA(ma, 2);
    ema = EMA(2);
    Indicator result = ema(ma);
    CHECK_EQ(result.size(), expect.size());
    for (size_t i = 0; i < expect.size(); ++i) {
        CHECK_EQ(result[i], expect[i]);
    }
}

/** @par 检测点 */
TEST_CASE("test_EMA_dyn") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-30));
    // KData kdata = stock.getKData(KQuery(0, Null<size_t>(), KQuery::MIN));
    Indicator c = CLOSE(kdata);
    Indicator expect = EMA(c, 10);
    Indicator result = EMA(c, CVAL(c, 10));
    CHECK_EQ(expect.size(), result.size());
    // CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    result = EMA(c, IndParam(CVAL(c, 10)));
    CHECK_EQ(expect.size(), result.size());
    // CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    expect = EMA(c, 2);
    result = EMA(c, CVAL(c, 2));
    CHECK_EQ(expect.size(), result.size());
    // CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_EMA_benchmark") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0));
    Indicator c = kdata.close();
    int cycle = 1000;  // 测试循环次数

    {
        BENCHMARK_TIME_MSG(test_EMA_benchmark, cycle, fmt::format("data len: {}", c.size()));
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
TEST_CASE("test_EMA_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/EMA.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = EMA(CLOSE(kdata), 10);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(ma1);
    }

    Indicator ma2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(ma2);
    }

    CHECK_EQ(ma1.size(), ma2.size());
    CHECK_EQ(ma1.discard(), ma2.discard());
    CHECK_EQ(ma1.getResultNumber(), ma2.getResultNumber());
    for (size_t i = 0; i < ma1.size(); ++i) {
        CHECK_EQ(ma1[i], doctest::Approx(ma2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
