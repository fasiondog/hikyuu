/*
 * test_MA.cpp
 *
 *  Created on: 2013-2-12
 *      Author: fasiondog
 */
#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_MA test_indicator_MA
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_MA") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData kdata;
    Indicator open, ma;

    /** @arg n = 3, 但关联数据为空 */
    open = OPEN(kdata);
    ma = MA(open, 3);
    CHECK_EQ(ma.name(), "MA");
    CHECK_EQ(ma.size(), 0);
    CHECK_EQ(ma.empty(), true);

    /** @arg n = 0 时，正常关联数据 */
    kdata = stock.getKData(KQuery(-10));
    open = OPEN(kdata);
    ma = MA(open, 0);
    CHECK_EQ(ma.empty(), false);
    CHECK_EQ(ma.size(), kdata.size());
    CHECK_EQ(ma.discard(), 0);
    std::vector<price_t> expects{
      2415.197,  2397.1715, 2395.89,   2392.8908, 2394.1114,
      2396.1477, 2395.6244, 2393.0338, 2389.7090, 2383.4041,
    };
    for (size_t i = 0; i < kdata.size(); ++i) {
        CHECK_EQ(ma[i], doctest::Approx(expects[i]).epsilon(0.0001));
    }

    /** @arg n = 10 且数据大小刚好为10 时, 正常关联数据 */
    kdata = stock.getKData(KQuery(-10));
    open = OPEN(kdata);
    ma = MA(open, 10);
    CHECK_EQ(ma.empty(), false);
    CHECK_EQ(ma.size(), kdata.size());
    CHECK_EQ(ma.discard(), 0);
    CHECK_LT(std::fabs(ma[0] - 2415.197), 0.00001);
    CHECK_LT(std::fabs(ma[1] - 2397.1715), 0.00001);
    CHECK_LT(std::fabs(ma[2] - 2395.890), 0.00001);
    CHECK_LT(std::fabs(ma[3] - 2392.89075), 0.00001);
    CHECK_LT(std::fabs(ma[4] - 2394.1114), 0.00001);
    CHECK_LT(std::fabs(ma[5] - 2396.14767), 0.00001);
    CHECK_LT(std::fabs(ma[6] - 2395.62443), 0.00001);
    CHECK_LT(std::fabs(ma[7] - 2393.03375), 0.00001);
    CHECK_LT(std::fabs(ma[8] - 2389.709), 0.00001);
    CHECK_LT(std::fabs(ma[9] - 2383.4041), 0.00001);

    /** @arg n = 10 且数据大小刚好为9 时, 正常关联数据 */
    kdata = stock.getKData(KQuery(-9));
    open = OPEN(kdata);
    ma = MA(open, 10);
    CHECK_EQ(ma.empty(), false);
    CHECK_EQ(ma.size(), kdata.size());
    CHECK_EQ(ma.discard(), 0);
    CHECK_LT(std::fabs(ma[0] - 2379.146), 0.00001);
    CHECK_LT(std::fabs(ma[1] - 2386.2365), 0.00001);
    CHECK_LT(std::fabs(ma[2] - 2385.45533), 0.00001);
    CHECK_LT(std::fabs(ma[3] - 2388.84), 0.00001);
    CHECK_LT(std::fabs(ma[7] - 2386.523), 0.00001);
    CHECK_LT(std::fabs(ma[8] - 2379.87156), 0.00001);

    /** @arg n = 10 且数据大小为11 时, 正常关联数据 */
    kdata = stock.getKData(KQuery(-11));
    open = OPEN(kdata);
    ma = MA(open, 10);
    CHECK_EQ(ma.empty(), false);
    CHECK_EQ(ma.size(), kdata.size());
    CHECK_EQ(ma.discard(), 0);
    CHECK_LT(std::fabs(ma[0] - 2400.984), 0.00001);
    CHECK_LT(std::fabs(ma[8] - 2393.91711), 0.00001);
    CHECK_LT(std::fabs(ma[9] - 2390.8365), 0.00001);
    CHECK_LT(std::fabs(ma[10] - 2383.4041), 0.00001);

    /** @arg n = 1 */
    kdata = stock.getKData(KQuery(-11));
    open = OPEN(kdata);
    ma = MA(open, 1);
    CHECK_EQ(ma.empty(), false);
    CHECK_EQ(ma.size(), kdata.size());
    CHECK_EQ(ma.discard(), 0);
    for (size_t i = 0; i < open.size(); ++i) {
        CHECK_LT(std::fabs(ma[i] - open[i]), 0.0001);
    }

    /** @arg 源数据本身带有discard不等于0 */
    PriceList data;
    for (int i = 0; i < 10; ++i) {
        data.push_back(i);
    }

    Indicator d = PRICELIST(data);
    Indicator ma1 = MA(d, 2);
    Indicator ma2 = MA(ma1, 2);
    CHECK_EQ(ma1.discard(), 0);
    CHECK_EQ(ma2.discard(), 0);
    CHECK_EQ(ma1[0], data[0]);
    CHECK_EQ(ma1[1], (data[0] + data[1]) / 2);
    CHECK_EQ(ma1[2], 1.5);
    CHECK_EQ(ma1[3], 2.5);
    CHECK_EQ(ma2[0], ma1[0]);
    CHECK_EQ(ma2[1], (ma1[0] + ma1[1]) / 2);
    CHECK_EQ(ma2[2], 1.0);
    CHECK_EQ(ma2[3], 2.0);

    /** @arg operator() */
    ma = MA(2);
    ma1 = MA(d, 2);
    ma2 = ma(d);
    CHECK_EQ(ma.size(), 0);
    CHECK_EQ(ma1.size(), 10);
    CHECK_EQ(ma1.size(), ma2.size());
    for (size_t i = 0; i < ma1.size(); ++i) {
        CHECK_EQ(ma1[i], ma2[i]);
    }
}

/** @par 检测点 */
TEST_CASE("test_MA_dyn") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-30));
    // KData kdata = stock.getKData(KQuery(0, Null<size_t>(), KQuery::MIN));
    Indicator c = CLOSE(kdata);
    Indicator expect = MA(c, 10);
    Indicator result = MA(c, CVAL(c, 10));
    CHECK_EQ(expect.size(), result.size());
    CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    result = MA(c, IndParam(CVAL(c, 10)));
    CHECK_EQ(expect.size(), result.size());
    CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    expect = MA(c, 0);
    result = MA(c, CVAL(c, 0));
    CHECK_EQ(expect.size(), result.size());
    CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_MA_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/MA.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = MA(CLOSE(kdata), 10);
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

    CHECK_EQ(ma2.name(), "MA");
    CHECK_EQ(ma1.size(), ma2.size());
    CHECK_EQ(ma1.discard(), ma2.discard());
    CHECK_EQ(ma1.getResultNumber(), ma2.getResultNumber());
    for (size_t i = 0; i < ma1.size(); ++i) {
        CHECK_EQ(ma1[i], doctest::Approx(ma2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
