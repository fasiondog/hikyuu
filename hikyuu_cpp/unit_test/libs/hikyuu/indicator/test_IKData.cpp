/*
 * test_IKData.cpp
 *
 *  Created on: 2013-2-12
 *      Author: fasiondog
 */

#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_indicator_suite
    #include <boost/test/unit_test.hpp>
#endif

#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/KDATA.h>

using namespace hku;

/**
 * @defgroup test_indicator_IKData test_indicator_IKData
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_IKData ) {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData kdata;

    /** @arg Indicator为空 */
    Indicator ki, open, high, low, close, amount, count;
    BOOST_CHECK(open.size() == 0);
    BOOST_CHECK(open.empty() == true);

    /** @arg 对应的KData为空 */
    ki = KDATA(kdata);
    BOOST_CHECK(ki.size() == 0);
    BOOST_CHECK(ki.empty() == true);

    open = OPEN(kdata);
    BOOST_CHECK(open.size() == 0);
    BOOST_CHECK(open.empty() == true);

    high = HIGH(kdata);
    BOOST_CHECK(high.size() == 0);
    BOOST_CHECK(high.empty() == true);

    low = LOW(kdata);
    BOOST_CHECK(low.size() == 0);
    BOOST_CHECK(low.empty() == true);

    close = CLOSE(kdata);
    BOOST_CHECK(close.size() == 0);
    BOOST_CHECK(close.empty() == true);

    amount = AMO(kdata);
    BOOST_CHECK(amount.size() == 0);
    BOOST_CHECK(amount.empty() == true);

    count = VOL(kdata);
    BOOST_CHECK(count.size() == 0);
    BOOST_CHECK(count.empty() == true);


    /** @arg 非空的KData */
    KQuery query(10);
    kdata = stock.getKData(query);
    size_t total = kdata.size();

    ki = KDATA(kdata);
    BOOST_CHECK(ki.size() == kdata.size());
    for (size_t i = 0; i < total; ++i) {
        BOOST_CHECK(ki.get(i, 0) == kdata[i].openPrice);
        BOOST_CHECK(ki.get(i, 1) == kdata[i].highPrice);
        BOOST_CHECK(ki.get(i, 2) == kdata[i].lowPrice);
        BOOST_CHECK(ki.get(i, 3) == kdata[i].closePrice);
        BOOST_CHECK(ki.get(i, 4) == kdata[i].transAmount);
        BOOST_CHECK(ki.get(i, 5) == kdata[i].transCount);
    }

    open = OPEN(kdata);
    BOOST_CHECK(open.size() == kdata.size());
    for (size_t i = 0; i < total; ++i) {
        BOOST_CHECK(open[i] == kdata[i].openPrice);
    }

    high = HIGH(kdata);
    BOOST_CHECK(high.size() == kdata.size());
    for (size_t i = 0; i < total; ++i) {
        BOOST_CHECK(high[i] == kdata[i].highPrice);
    }

    low = LOW(kdata);
    BOOST_CHECK(low.size() == kdata.size());
    for (size_t i = 0; i < total; ++i) {
        BOOST_CHECK(low[i] == kdata[i].lowPrice);
    }

    close = CLOSE(kdata);
    BOOST_CHECK(close.size() == kdata.size());
    for (size_t i = 0; i < total; ++i) {
        BOOST_CHECK(close[i] == kdata[i].closePrice);
    }

    amount = AMO(kdata);
    BOOST_CHECK(amount.size() == kdata.size());
    for (size_t i = 0; i < total; ++i) {
        BOOST_CHECK(amount[i] == kdata[i].transAmount);
    }

    count = VOL(kdata);
    BOOST_CHECK(count.size() == kdata.size());
    for (size_t i = 0; i < total; ++i) {
        BOOST_CHECK(count[i] == kdata[i].transCount);
    }
}

/** @} */
