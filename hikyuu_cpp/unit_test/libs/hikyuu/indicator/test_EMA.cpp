/*
 * test_EMA.cpp
 *
 *  Created on: 2013-4-10
 *      Author: fasiondog
 */

#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_indicator_suite
    #include <boost/test/unit_test.hpp>
#endif

#include <hikyuu/indicator/crt/EMA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/MA.h>

using namespace hku;

/**
 * @defgroup test_indicator_EMA test_indicator_EMA
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_EMA ) {
    PriceList d;
    for (int i = 0; i < 30; ++i) {
        d.push_back(i);
    }

    /** @arg n = 1 */
    Indicator data = PRICELIST(d);
    Indicator ema = EMA(data, 1);
    BOOST_CHECK(ema.discard() == 0);
    BOOST_CHECK(ema.size() == 30);
    BOOST_CHECK(ema[0] == 0.0);
    BOOST_CHECK(ema[10] == 10.0);
    BOOST_CHECK(ema[29] == 29.0);

    /** @arg n = 2 */
    ema = EMA(data, 2);
    BOOST_CHECK(ema.discard() == 0);
    BOOST_CHECK(ema.size() == 30);
    BOOST_CHECK(ema[0] == 0.0);
    BOOST_CHECK(std::fabs(ema[1] - 0.66667) < 0.0001);
    BOOST_CHECK(std::fabs(ema[2] - 1.55556) < 0.0001);
    BOOST_CHECK(std::fabs(ema[3] - 2.51852) < 0.0001);
    BOOST_CHECK(std::fabs(ema[4] - 3.50617) < 0.0001);

    /** @arg n = 10 */
    ema = EMA(data, 10);
    BOOST_CHECK(ema.discard() == 0);
    BOOST_CHECK(ema.size() == 30);
    BOOST_CHECK(ema[0] == 0.0);
    BOOST_CHECK(std::fabs(ema[1] - 0.18182) < 0.0001);
    BOOST_CHECK(std::fabs(ema[2] - 0.51240) < 0.0001);

#if 0 //由于修改了SMA计算过程，MA的抛弃数量为零，下面的测试用例失效
    /** @arg 待计算的数据存在非零的discard */
    Indicator ma = MA(data, 2);
    ema = EMA(ma, 2);
    BOOST_CHECK(ma.discard() == 1);
    BOOST_CHECK(ema.discard() == 1);
    BOOST_CHECK(ma[0] == Null<price_t>());
    BOOST_CHECK(ma[1] == 0.5);
    BOOST_CHECK(ma[2] == 1.5);
    BOOST_CHECK(ema[0] == Null<price_t>());
    BOOST_CHECK(ema[1] == 0.5);
    BOOST_CHECK(std::fabs(ema[2] - 1.16667) < 0.0001 );
#endif

    /** @arg operator() */
    Indicator ma = MA(data, 2);
    Indicator expect = EMA(ma, 2);
    ema = EMA(2);
    Indicator result = ema(ma);
    BOOST_CHECK(result.size() == expect.size());
    for (size_t i = 0; i < expect.size(); ++i) {
        BOOST_CHECK(result[i] == expect[i]);
    }
}

/** @} */

