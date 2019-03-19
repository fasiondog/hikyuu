/*
 * test_MACD.cpp
 *
 *  Created on: 2013-4-11
 *      Author: fasiondog
 */


#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_indicator_suite
    #include <boost/test/unit_test.hpp>
#endif

#include <hikyuu/indicator/crt/MACD.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/EMA.h>

using namespace hku;

/**
 * @defgroup test_indicator_MACD test_indicator_MACD
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_MACD ) {
    PriceList d;
    for (size_t i = 0; i < 20; ++i) {
        d.push_back(i);
    }

    Indicator ind = PRICELIST(d);
    Indicator macd, bar, diff, dea;
    Indicator ema1, ema2, fast, slow, bmacd;

    /** @arg 源数据为空 */
    macd = MACD(Indicator(), 12, 26, 9);
    BOOST_CHECK(macd.size() == 0);
    BOOST_CHECK(macd.empty() == true);

    /** @arg n1 = n2 = n3 = 1*/
    macd = MACD(ind, 1, 1, 1);
    BOOST_CHECK(macd.getResultNumber() == 3);
    bar = macd.getResult(0);
    diff = macd.getResult(1);
    dea = macd.getResult(2);
    BOOST_CHECK(bar.size() == 20);
    BOOST_CHECK(diff.size() == 20);
    BOOST_CHECK(dea.size() == 20);

    BOOST_CHECK(diff[0] == 0);
    BOOST_CHECK(diff[1] == 0);
    BOOST_CHECK(diff[19] == 0);

    BOOST_CHECK(dea[0] == 0);
    BOOST_CHECK(dea[1] == 0);
    BOOST_CHECK(dea[19] == 0);

    BOOST_CHECK(bar[0] == 0);
    BOOST_CHECK(bar[1] == 0);
    BOOST_CHECK(bar[19] == 0);

    /** @arg n1 = 1 n2 = 2 n3 = 3*/
    macd = MACD(ind, 1, 2, 3);
    BOOST_CHECK(macd.size() == 20);
    BOOST_CHECK(macd.discard() == 0);
    bar = macd.getResult(0);
    diff = macd.getResult(1);
    dea = macd.getResult(2);
    ema1 = EMA(ind, 1);
    ema2 = EMA(ind, 2);
    fast = ema1 - ema2;
    slow = EMA(fast, 3);
    bmacd = fast - slow;
    BOOST_CHECK(bar.size() == 20);
    BOOST_CHECK(diff.size() == 20);
    BOOST_CHECK(dea.size() == 20);

    BOOST_CHECK(diff[0] == fast[0]);
    BOOST_CHECK(diff[1] == fast[1]);
    BOOST_CHECK(diff[19] == fast[19]);

    BOOST_CHECK(dea[0] == slow[0]);
    BOOST_CHECK(std::fabs(dea[1] - slow[1]) < 0.0001);
    BOOST_CHECK(dea[19] == slow[19]);

    BOOST_CHECK(bar[0] == bmacd[0]);
    BOOST_CHECK(bar[1] == bmacd[1]);
    BOOST_CHECK(bar[19] == bmacd[19]);

    /** @arg n1 = 3 n2 = 2 n3 = 1*/
    macd = MACD(ind, 3, 2, 1);
    BOOST_CHECK(macd.size() == 20);
    BOOST_CHECK(macd.discard() == 0);
    bar = macd.getResult(0);
    diff = macd.getResult(1);
    dea = macd.getResult(2);
    ema1 = EMA(ind, 3);
    ema2 = EMA(ind, 2);
    fast = ema1 - ema2;
    slow = EMA(fast, 1);
    bmacd = fast - slow;
    BOOST_CHECK(bar.size() == 20);
    BOOST_CHECK(diff.size() == 20);
    BOOST_CHECK(dea.size() == 20);

    BOOST_CHECK(diff[0] == fast[0]);
    BOOST_CHECK(diff[1] == fast[1]);
    BOOST_CHECK(diff[19] == fast[19]);

    BOOST_CHECK(dea[0] == slow[0]);
    BOOST_CHECK(dea[1] == slow[1]);
    BOOST_CHECK(dea[19] == slow[19]);

    BOOST_CHECK(bar[0] == bmacd[0]);
    BOOST_CHECK(bar[1] == bmacd[1]);
    BOOST_CHECK(bar[19] == bmacd[19]);

    /** @arg n1 = 3 n2 = 5 n3 = 2*/
    macd = MACD(ind, 3, 5, 2);
    BOOST_CHECK(macd.size() == 20);
    BOOST_CHECK(macd.discard() == 0);
    bar = macd.getResult(0);
    diff = macd.getResult(1);
    dea = macd.getResult(2);
    ema1 = EMA(ind, 3);
    ema2 = EMA(ind, 5);
    fast = ema1 - ema2;
    slow = EMA(fast, 2);
    bmacd = fast - slow;
    BOOST_CHECK(bar.size() == 20);
    BOOST_CHECK(diff.size() == 20);
    BOOST_CHECK(dea.size() == 20);

    BOOST_CHECK(diff[0] == fast[0]);
    BOOST_CHECK(diff[1] == fast[1]);
    BOOST_CHECK(diff[19] == fast[19]);

    BOOST_CHECK(dea[0] == slow[0]);
    BOOST_CHECK(dea[1] == slow[1]);
    BOOST_CHECK(dea[19] == slow[19]);

    BOOST_CHECK(bar[0] == bmacd[0]);
    BOOST_CHECK(bar[1] == bmacd[1]);
    BOOST_CHECK(bar[19] == bmacd[19]);

    /** @arg operator() */
    Indicator expect = MACD(ind, 3, 5, 2);
    Indicator tmp = MACD(3, 5, 2);
    Indicator result = tmp(ind);
    BOOST_CHECK(result.size() == expect.size());
    for (size_t i = 0; i < expect.size(); ++i) {
        BOOST_CHECK(result.get(i, 0) == expect.get(i, 0));
        BOOST_CHECK(result.get(i, 1) == expect.get(i, 1));
        BOOST_CHECK(result.get(i, 2) == expect.get(i, 2));
    }
}

/** @} */


