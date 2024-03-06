/*
 * test_IKData.cpp
 *
 *  Created on: 2013-2-12
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/KDATA.h>

using namespace hku;

/**
 * @defgroup test_indicator_IKData test_indicator_IKData
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_IKData") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData kdata;

    /** @arg Indicator为空 */
    Indicator ki, open, high, low, close, amount, count;
    CHECK_EQ(open.size(), 0);
    CHECK_EQ(open.empty(), true);

    /** @arg 对应的KData为空 */
    ki = KDATA(kdata);
    CHECK_EQ(ki.size(), 0);
    CHECK_EQ(ki.empty(), true);

    open = OPEN(kdata);
    CHECK_EQ(open.size(), 0);
    CHECK_EQ(open.empty(), true);

    high = HIGH(kdata);
    CHECK_EQ(high.size(), 0);
    CHECK_EQ(high.empty(), true);

    low = LOW(kdata);
    CHECK_EQ(low.size(), 0);
    CHECK_EQ(low.empty(), true);

    close = CLOSE(kdata);
    CHECK_EQ(close.size(), 0);
    CHECK_EQ(close.empty(), true);

    amount = AMO(kdata);
    CHECK_EQ(amount.size(), 0);
    CHECK_EQ(amount.empty(), true);

    count = VOL(kdata);
    CHECK_EQ(count.size(), 0);
    CHECK_EQ(count.empty(), true);

    /** @arg 非空的KData */
    KQuery query(10);
    kdata = stock.getKData(query);
    size_t total = kdata.size();

    ki = KDATA(kdata);
    CHECK_EQ(ki.size(), kdata.size());
    for (size_t i = 0; i < total; ++i) {
        CHECK_EQ(ki.get(i, 0), doctest::Approx(kdata[i].openPrice));
        CHECK_EQ(ki.get(i, 1), doctest::Approx(kdata[i].highPrice));
        CHECK_EQ(ki.get(i, 2), doctest::Approx(kdata[i].lowPrice));
        CHECK_EQ(ki.get(i, 3), doctest::Approx(kdata[i].closePrice));
        CHECK_EQ(ki.get(i, 4), doctest::Approx(kdata[i].transAmount));
        CHECK_EQ(ki.get(i, 5), doctest::Approx(kdata[i].transCount));
    }

    open = OPEN(kdata);
    CHECK_EQ(open.size(), kdata.size());
    for (size_t i = 0; i < total; ++i) {
        CHECK_EQ(open[i], doctest::Approx(kdata[i].openPrice));
    }

    high = HIGH(kdata);
    CHECK_EQ(high.size(), kdata.size());
    for (size_t i = 0; i < total; ++i) {
        CHECK_EQ(high[i], doctest::Approx(kdata[i].highPrice));
    }

    low = LOW(kdata);
    CHECK_EQ(low.size(), kdata.size());
    for (size_t i = 0; i < total; ++i) {
        CHECK_EQ(low[i], doctest::Approx(kdata[i].lowPrice));
    }

    close = CLOSE(kdata);
    CHECK_EQ(close.size(), kdata.size());
    for (size_t i = 0; i < total; ++i) {
        CHECK_EQ(close[i], doctest::Approx(kdata[i].closePrice));
    }

    amount = AMO(kdata);
    CHECK_EQ(amount.size(), kdata.size());
    for (size_t i = 0; i < total; ++i) {
        CHECK_EQ(amount[i], doctest::Approx(kdata[i].transAmount));
    }

    count = VOL(kdata);
    CHECK_EQ(count.size(), kdata.size());
    for (size_t i = 0; i < total; ++i) {
        CHECK_EQ(count[i], doctest::Approx(kdata[i].transCount));
    }
}

/** @par 检测点 */
TEST_CASE("test_IKData_setContext") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");

    KQuery query(10);
    KData kdata = stock.getKData(query);
    size_t total = kdata.size();

    Indicator ki, open, high, low, close, amount, count;

    ki = KDATA();
    ki.setContext(stock, query);
    CHECK_EQ(ki.size(), kdata.size());
    for (size_t i = 0; i < total; ++i) {
        CHECK_EQ(ki.get(i, 0), doctest::Approx(kdata[i].openPrice));
        CHECK_EQ(ki.get(i, 1), doctest::Approx(kdata[i].highPrice));
        CHECK_EQ(ki.get(i, 2), doctest::Approx(kdata[i].lowPrice));
        CHECK_EQ(ki.get(i, 3), doctest::Approx(kdata[i].closePrice));
        CHECK_EQ(ki.get(i, 4), doctest::Approx(kdata[i].transAmount));
        CHECK_EQ(ki.get(i, 5), doctest::Approx(kdata[i].transCount));
    }

    open = OPEN();
    open.setContext(stock, query);
    CHECK_EQ(open.size(), kdata.size());
    for (size_t i = 0; i < total; ++i) {
        CHECK_EQ(open[i], doctest::Approx(kdata[i].openPrice));
    }

    high = HIGH();
    high.setContext(stock, query);
    CHECK_EQ(high.size(), kdata.size());
    for (size_t i = 0; i < total; ++i) {
        CHECK_EQ(high[i], doctest::Approx(kdata[i].highPrice));
    }

    low = LOW();
    low.setContext(stock, query);
    CHECK_EQ(low.size(), kdata.size());
    for (size_t i = 0; i < total; ++i) {
        CHECK_EQ(low[i], doctest::Approx(kdata[i].lowPrice));
    }

    close = CLOSE();
    close.setContext(stock, query);
    CHECK_EQ(close.size(), kdata.size());
    for (size_t i = 0; i < total; ++i) {
        CHECK_EQ(close[i], doctest::Approx(kdata[i].closePrice));
    }

    amount = AMO();
    amount.setContext(stock, query);
    CHECK_EQ(amount.size(), kdata.size());
    for (size_t i = 0; i < total; ++i) {
        CHECK_EQ(amount[i], doctest::Approx(kdata[i].transAmount));
    }

    count = VOL();
    count.setContext(stock, query);
    CHECK_EQ(count.size(), kdata.size());
    for (size_t i = 0; i < total; ++i) {
        CHECK_EQ(count[i], doctest::Approx(kdata[i].transCount));
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_KDATA_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/IKDATA.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = KDATA(kdata);
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
        CHECK_EQ(ma1.get(i, 3), doctest::Approx(ma2.get(i, 3)));
        CHECK_EQ(ma1.get(i, 4), doctest::Approx(ma2.get(i, 4)));
        CHECK_EQ(ma1.get(i, 5), doctest::Approx(ma2.get(i, 5)));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
