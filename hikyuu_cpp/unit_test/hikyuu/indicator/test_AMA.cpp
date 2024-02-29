/*
 * test_AMA.cpp
 *
 *  Created on: 2013-4-10
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/AMA.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_AMA test_indicator_AMA
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_AMA") {
    Indicator result;

    PriceList d;
    d.push_back(6063);
    d.push_back(6041);
    d.push_back(6065);
    d.push_back(6078);
    d.push_back(6114);
    d.push_back(6121);
    d.push_back(6106);
    d.push_back(6101);
    d.push_back(6166);
    d.push_back(6169);
    d.push_back(6195);
    d.push_back(6222);
    d.push_back(6186);
    d.push_back(6214);
    d.push_back(6185);
    d.push_back(6209);
    d.push_back(6221);
    d.push_back(6278);
    d.push_back(6326);
    d.push_back(6347);

    Indicator ind = PRICELIST(d);
    CHECK_EQ(ind.size(), 20);
    result = AMA(ind, 10, 2, 30);
    CHECK_EQ(result.getParam<int>("n"), 10);
    CHECK_EQ(result.getParam<int>("fast_n"), 2);
    CHECK_EQ(result.getParam<int>("slow_n"), 30);
    CHECK_EQ(result.size(), 20);
    CHECK_EQ(result.empty(), false);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.getResultNumber(), 2);
    CHECK_EQ(result[0], 6063);
    CHECK_EQ(result[9], doctest::Approx(6103.6781));
    CHECK_EQ(result[10], doctest::Approx(6120.760197));
    CHECK_EQ(result[18], doctest::Approx(6216.376893));
    CHECK_EQ(result[19], doctest::Approx(6239.100742));

    CHECK_EQ(result.get(0, 1), 1.0);
    CHECK_EQ(result.get(9, 1), doctest::Approx(0.557895));
    CHECK_EQ(result.get(10, 1), doctest::Approx(0.611111));
    CHECK_EQ(result.get(11, 1), doctest::Approx(0.826484));
    CHECK_EQ(result.get(18, 1), doctest::Approx(0.517799));
    CHECK_EQ(result.get(19, 1), doctest::Approx(0.585526));

    /** @arg operator() */
    Indicator ama = AMA(10, 2, 30);
    CHECK_EQ(ama.size(), 0);
    Indicator expect = AMA(ind, 10, 2, 30);
    result = ama(ind);
    CHECK_EQ(result.size(), expect.size());
    CHECK_EQ(result.getResultNumber(), expect.getResultNumber());
    CHECK_EQ(result.discard(), expect.discard());
    for (size_t i = 0; i < expect.size(); ++i) {
        CHECK_EQ(result[i], expect[i]);
        CHECK_EQ(result.get(i, 1), expect.get(i, 1));
    }
}

/** @par 检测点 */
TEST_CASE("test_AMA_dyn") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-50));
    // KData kdata = stock.getKData(KQuery(0, Null<size_t>(), KQuery::MIN));
    Indicator c = CLOSE(kdata);
    Indicator expect = AMA(c, 10, 2, 30);
    Indicator result = AMA(c, CVAL(c, 10), CVAL(c, 2), CVAL(c, 30));
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = result.discard(); i < result.size(); i++) {
        CHECK_EQ(expect.get(i, 0), doctest::Approx(result.get(i, 0)));
        CHECK_EQ(expect.get(i, 1), doctest::Approx(result.get(i, 1)));
    }

    result = AMA(c, IndParam(CVAL(c, 10)), IndParam(CVAL(c, 2)), IndParam(CVAL(c, 30)));
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = result.discard(); i < result.size(); i++) {
        CHECK_EQ(expect.get(i, 0), doctest::Approx(result.get(i, 0)));
        CHECK_EQ(expect.get(i, 1), doctest::Approx(result.get(i, 1)));
    }
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_AMA_benchmark") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0));
    Indicator c = kdata.close();
    int cycle = 1000;  // 测试循环次数

    {
        BENCHMARK_TIME_MSG(test_AMA_benchmark, cycle, fmt::format("data len: {}", c.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator ind = AMA();
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
TEST_CASE("test_AMA_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/AMA.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = AMA(CLOSE(kdata), 10);
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
        CHECK_EQ(ma1.get(i, 0), doctest::Approx(ma2.get(i, 0)));
        CHECK_EQ(ma1.get(i, 1), doctest::Approx(ma2.get(i, 1)));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
