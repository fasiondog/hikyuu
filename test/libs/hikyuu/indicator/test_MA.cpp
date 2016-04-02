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
 * @defgroup test_indicator_MA test_indicator_MA
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
    BOOST_CHECK(ma.discard() == 0);
    BOOST_CHECK(std::fabs(ma[0] - 2415.197) < 0.00001);
    BOOST_CHECK(std::fabs(ma[1] - 2397.1715) < 0.00001);
    BOOST_CHECK(std::fabs(ma[2] - 2395.890) < 0.00001);
    BOOST_CHECK(std::fabs(ma[3] - 2392.89075) < 0.00001);
    BOOST_CHECK(std::fabs(ma[4] - 2394.1114) < 0.00001);
    BOOST_CHECK(std::fabs(ma[5] - 2396.14767) < 0.00001);
    BOOST_CHECK(std::fabs(ma[6] - 2395.62443) < 0.00001);
    BOOST_CHECK(std::fabs(ma[7] - 2393.03375) < 0.00001);
    BOOST_CHECK(std::fabs(ma[8] - 2389.709) < 0.00001);
    BOOST_CHECK(std::fabs(ma[9] - 2383.4041) < 0.00001);

    /** @arg n = 10 且数据大小刚好为9 时, 正常关联数据 */
    kdata = stock.getKData(KQuery(-9));
    open = OPEN(kdata);
    ma = MA(open, 10);
    BOOST_CHECK(ma.empty() == false);
    BOOST_CHECK(ma.size() == kdata.size());
    BOOST_CHECK(ma.discard() == 0);
    BOOST_CHECK(std::fabs(ma[0] - 2379.146) < 0.00001);
    BOOST_CHECK(std::fabs(ma[1] - 2386.2365) < 0.00001);
    BOOST_CHECK(std::fabs(ma[2] - 2385.45533) < 0.00001);
    BOOST_CHECK(std::fabs(ma[3] - 2388.84) < 0.00001);
    BOOST_CHECK(std::fabs(ma[7] - 2386.523) < 0.00001);
    BOOST_CHECK(std::fabs(ma[8] - 2379.87156) < 0.00001);

    /** @arg n = 10 且数据大小为11 时, 正常关联数据 */
    kdata = stock.getKData(KQuery(-11));
    open = OPEN(kdata);
    ma = MA(open, 10);
    BOOST_CHECK(ma.empty() == false);
    BOOST_CHECK(ma.size() == kdata.size());
    BOOST_CHECK(ma.discard() == 0);
    BOOST_CHECK(std::fabs(ma[0] - 2400.984) < 0.00001);
    BOOST_CHECK(std::fabs(ma[8] - 2393.91711) < 0.00001);
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
    BOOST_CHECK(ma1.discard() == 0);
    BOOST_CHECK(ma2.discard() == 0);
    BOOST_CHECK(ma1[0] == data[0]);
    BOOST_CHECK(ma1[1] == (data[0]+data[1])/2);
    BOOST_CHECK(ma1[2] == 1.5);
    BOOST_CHECK(ma1[3] == 2.5);
    BOOST_CHECK(ma2[0] == ma1[0]);
    BOOST_CHECK(ma2[1] == (ma1[0]+ma1[1])/2);
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
