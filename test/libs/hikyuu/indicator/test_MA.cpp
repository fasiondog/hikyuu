/*
 * test_MA.cpp
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
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_MA
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_MA ) {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData kdata;
    Indicator open, ma;

    /** @arg n = 3, 但关联数据为空 */
    open = OPEN(kdata);
    ma = MA(open, 3);
    BOOST_CHECK(ma.size() == 0);
    BOOST_CHECK(ma.empty() == true);

    /** @arg n = 0 时，正常关联数据 */
    kdata = stock.getKData(KQuery(-10));
    open = OPEN(kdata);
    ma = MA(open, 0);
    BOOST_CHECK(ma.empty() == false);
    BOOST_CHECK(ma.size() == kdata.size());
    BOOST_CHECK(ma.discard() == 0);
    for (size_t i = 0; i < kdata.size(); ++i) {
        BOOST_CHECK(ma[i] == Null<price_t>());
    }

    /** @arg n = 10 且数据大小刚好为10 时, 正常关联数据 */
    kdata = stock.getKData(KQuery(-10));
    open = OPEN(kdata);
    ma = MA(open, 10);
    BOOST_CHECK(ma.empty() == false);
    BOOST_CHECK(ma.size() == kdata.size());
    BOOST_CHECK(ma.discard() == 9);
    for (size_t i = 0; i < kdata.size() - 1; ++i) {
        BOOST_CHECK(ma[i] == Null<price_t>());
    }
    BOOST_CHECK(std::fabs(ma[9] - 2383.4041) < 0.00001);

    /** @arg n = 10 且数据大小刚好为9 时, 正常关联数据 */
    kdata = stock.getKData(KQuery(-9));
    open = OPEN(kdata);
    ma = MA(open, 10);
    BOOST_CHECK(ma.empty() == false);
    BOOST_CHECK(ma.size() == kdata.size());
    BOOST_CHECK(ma.discard() == 9);
    for (size_t i = 0; i < kdata.size(); ++i) {
        BOOST_CHECK(ma[i] == Null<price_t>());
    }

    /** @arg n = 10 且数据大小为11 时, 正常关联数据 */
    kdata = stock.getKData(KQuery(-11));
    open = OPEN(kdata);
    ma = MA(open, 10);
    BOOST_CHECK(ma.empty() == false);
    BOOST_CHECK(ma.size() == kdata.size());
    BOOST_CHECK(ma.discard() == 9);
    for (size_t i = 0; i < 9; ++i) {
        BOOST_CHECK(ma[i] == Null<price_t>());
    }
    BOOST_CHECK(std::fabs(ma[9] - 2390.8365) < 0.00001);
    BOOST_CHECK(std::fabs(ma[10] - 2383.4041) < 0.00001);

    /** @arg n = 1 */
    kdata = stock.getKData(KQuery(-11));
    open = OPEN(kdata);
    ma = MA(open, 1);
    BOOST_CHECK(ma.empty() == false);
    BOOST_CHECK(ma.size() == kdata.size());
    BOOST_CHECK(ma.discard() == 0);
    for (size_t i = 0; i < open.size(); ++i) {
        BOOST_CHECK(std::fabs(ma[i] - open[i]) < 0.0001);
    }

    /** @arg 源数据本身带有discard不等于0 */
    PriceList data;
    for (int i = 0; i < 10; ++i) {
        data.push_back(i);
    }

    Indicator d = PRICELIST(data);
    Indicator ma1 = MA(d, 2);
    Indicator ma2 = MA(ma1, 2);
    BOOST_CHECK(ma1.discard() == 1);
    BOOST_CHECK(ma2.discard() == 2);
    BOOST_CHECK(ma1[0] == Null<price_t>());
    BOOST_CHECK(ma1[1] == 0.5);
    BOOST_CHECK(ma1[2] == 1.5);
    BOOST_CHECK(ma1[3] == 2.5);
    BOOST_CHECK(ma2[0] == Null<price_t>());
    BOOST_CHECK(ma2[1] == Null<price_t>());
    BOOST_CHECK(ma2[2] == 1.0);
    BOOST_CHECK(ma2[3] == 2.0);

    /** @arg operator() */
    ma = MA(2);
    ma1 = MA(d, 2);
    ma2 = ma(d);
    BOOST_CHECK(ma.size() == 0);
    BOOST_CHECK(ma1.size() == 10);
    BOOST_CHECK(ma1.size() == ma2.size());
    for (size_t i = 0; i < ma1.size(); ++i) {
        BOOST_CHECK(ma1[i] == ma2[i]);
    }
}

/** @} */
