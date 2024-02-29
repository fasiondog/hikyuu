/*
 * test_MACD.cpp
 *
 *  Created on: 2013-4-11
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/MACD.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/EMA.h>

using namespace hku;

/**
 * @defgroup test_indicator_MACD test_indicator_MACD
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_MACD") {
    PriceList d;
    for (size_t i = 0; i < 20; ++i) {
        d.push_back(i);
    }

    Indicator ind = PRICELIST(d);
    Indicator macd, bar, diff, dea;
    Indicator ema1, ema2, fast, slow, bmacd;

    /** @arg 源数据为空 */
    macd = MACD(Indicator(), 12, 26, 9);
    CHECK_EQ(macd.size(), 0);
    CHECK_EQ(macd.empty(), true);

    /** @arg n1 = n2 = n3 = 1*/
    macd = MACD(ind, 1, 1, 1);
    CHECK_EQ(macd.getResultNumber(), 3);
    bar = macd.getResult(0);
    diff = macd.getResult(1);
    dea = macd.getResult(2);
    CHECK_EQ(bar.size(), 20);
    CHECK_EQ(diff.size(), 20);
    CHECK_EQ(dea.size(), 20);

    CHECK_EQ(diff[0], 0);
    CHECK_EQ(diff[1], 0);
    CHECK_EQ(diff[19], 0);

    CHECK_EQ(dea[0], 0);
    CHECK_EQ(dea[1], 0);
    CHECK_EQ(dea[19], 0);

    CHECK_EQ(bar[0], 0);
    CHECK_EQ(bar[1], 0);
    CHECK_EQ(bar[19], 0);

    /** @arg n1 = 1 n2 = 2 n3 = 3*/
    macd = MACD(ind, 1, 2, 3);
    CHECK_EQ(macd.size(), 20);
    CHECK_EQ(macd.discard(), 0);
    bar = macd.getResult(0);
    diff = macd.getResult(1);
    dea = macd.getResult(2);
    ema1 = EMA(ind, 1);
    ema2 = EMA(ind, 2);
    fast = ema1 - ema2;
    slow = EMA(fast, 3);
    bmacd = fast - slow;
    CHECK_EQ(bar.size(), 20);
    CHECK_EQ(diff.size(), 20);
    CHECK_EQ(dea.size(), 20);

    CHECK_EQ(diff[0], doctest::Approx(fast[0]));
    CHECK_EQ(diff[1], doctest::Approx(fast[1]));
    CHECK_EQ(diff[19], doctest::Approx(fast[19]));

    CHECK_EQ(dea[0], slow[0]);
    CHECK_LT(std::fabs(dea[1] - slow[1]), 0.0001);
    CHECK_EQ(dea[19], slow[19]);

    CHECK_EQ(bar[0], doctest::Approx(bmacd[0]));
    CHECK_EQ(bar[1], doctest::Approx(bmacd[1]));
    CHECK_EQ(bar[19], doctest::Approx(bmacd[19]));

    /** @arg n1 = 3 n2 = 2 n3 = 1*/
    macd = MACD(ind, 3, 2, 1);
    CHECK_EQ(macd.size(), 20);
    CHECK_EQ(macd.discard(), 0);
    bar = macd.getResult(0);
    diff = macd.getResult(1);
    dea = macd.getResult(2);
    ema1 = EMA(ind, 3);
    ema2 = EMA(ind, 2);
    fast = ema1 - ema2;
    slow = EMA(fast, 1);
    bmacd = fast - slow;
    CHECK_EQ(bar.size(), 20);
    CHECK_EQ(diff.size(), 20);
    CHECK_EQ(dea.size(), 20);

    CHECK_EQ(diff[0], doctest::Approx(fast[0]));
    CHECK_EQ(diff[1], doctest::Approx(fast[1]));
    CHECK_EQ(diff[19], doctest::Approx(fast[19]));

    CHECK_EQ(dea[0], doctest::Approx(slow[0]));
    CHECK_EQ(dea[1], doctest::Approx(slow[1]));
    CHECK_EQ(dea[19], doctest::Approx(slow[19]));

    CHECK_EQ(bar[0], doctest::Approx(bmacd[0]));
    CHECK_EQ(bar[1], doctest::Approx(bmacd[1]));
    CHECK_EQ(bar[19], doctest::Approx(bmacd[19]));

    /** @arg n1 = 3 n2 = 5 n3 = 2*/
    macd = MACD(ind, 3, 5, 2);
    CHECK_EQ(macd.size(), 20);
    CHECK_EQ(macd.discard(), 0);
    bar = macd.getResult(0);
    diff = macd.getResult(1);
    dea = macd.getResult(2);
    ema1 = EMA(ind, 3);
    ema2 = EMA(ind, 5);
    fast = ema1 - ema2;
    slow = EMA(fast, 2);
    bmacd = fast - slow;
    CHECK_EQ(bar.size(), 20);
    CHECK_EQ(diff.size(), 20);
    CHECK_EQ(dea.size(), 20);

    CHECK_EQ(diff[0], doctest::Approx(fast[0]));
    CHECK_EQ(diff[1], doctest::Approx(fast[1]));
    CHECK_EQ(diff[19], doctest::Approx(fast[19]));

    CHECK_EQ(dea[0], doctest::Approx(slow[0]));
    CHECK_EQ(dea[1], doctest::Approx(slow[1]));
    CHECK_EQ(dea[19], doctest::Approx(slow[19]));

    CHECK_EQ(bar[0], doctest::Approx(bmacd[0]));
    CHECK_EQ(bar[1], doctest::Approx(bmacd[1]));
    CHECK_EQ(bar[19], doctest::Approx(bmacd[19]));

    /** @arg operator() */
    Indicator expect = MACD(ind, 3, 5, 2);
    Indicator tmp = MACD(3, 5, 2);
    Indicator result = tmp(ind);
    CHECK_EQ(result.size(), expect.size());
    for (size_t i = 0; i < expect.size(); ++i) {
        CHECK_EQ(result.get(i, 0), expect.get(i, 0));
        CHECK_EQ(result.get(i, 1), expect.get(i, 1));
        CHECK_EQ(result.get(i, 2), expect.get(i, 2));
    }
}

/** @par 检测点 */
TEST_CASE("test_MACD_dyn") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-50));
    // KData kdata = stock.getKData(KQuery(0, Null<size_t>(), KQuery::MIN));
    Indicator c = CLOSE(kdata);
    Indicator expect = MACD(c, 12, 26, 9);
    Indicator result = MACD(c, CVAL(c, 12), CVAL(c, 26), CVAL(c, 9));
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = result.discard(); i < result.size(); i++) {
        CHECK_EQ(expect.get(i, 0), doctest::Approx(result.get(i, 0)));
        CHECK_EQ(expect.get(i, 1), doctest::Approx(result.get(i, 1)));
        CHECK_EQ(expect.get(i, 2), doctest::Approx(result.get(i, 2)));
    }

    result = MACD(c, IndParam(CVAL(c, 12)), IndParam(CVAL(c, 26)), IndParam(CVAL(c, 9)));
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = result.discard(); i < result.size(); i++) {
        CHECK_EQ(expect.get(i, 0), doctest::Approx(result.get(i, 0)));
        CHECK_EQ(expect.get(i, 1), doctest::Approx(result.get(i, 1)));
        CHECK_EQ(expect.get(i, 2), doctest::Approx(result.get(i, 2)));
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_MACD_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/MACD.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = MACD(CLOSE(kdata));
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
        CHECK_EQ(ma1.get(i, 2), doctest::Approx(ma2.get(i, 2)));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
