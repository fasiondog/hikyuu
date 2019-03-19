/*
 * test_AMA.cpp
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

#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/AMA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_AMA test_indicator_AMA
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_AMA ) {
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
    BOOST_CHECK(ind.size() == 20);
    result = AMA(ind, 10, 2, 30);
    BOOST_CHECK(result.getParam<int>("n") == 10);
    BOOST_CHECK(result.getParam<int>("fast_n") == 2);
    BOOST_CHECK(result.getParam<int>("slow_n") == 30);
    BOOST_CHECK(result.size() == 20);
    BOOST_CHECK(result.empty() == false);
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result.getResultNumber() == 2);
    BOOST_CHECK(result[0] == 6063);
    BOOST_CHECK(std::fabs(result[9] - 6103.6781) < 0.000001);
    BOOST_CHECK(std::fabs(result[10] - 6120.760197) < 0.000001);
    BOOST_CHECK(std::fabs(result[18] - 6216.376893) < 0.000001);
    BOOST_CHECK(std::fabs(result[19] - 6239.100742) < 0.000001);

    BOOST_CHECK(result.get(0,1) == 1.0);
    BOOST_CHECK(std::fabs(result.get(9, 1) - 0.557895) < 0.000001);
    BOOST_CHECK(std::fabs(result.get(10, 1) - 0.611111) < 0.000001);
    BOOST_CHECK(std::fabs(result.get(11, 1) - 0.826484) < 0.000001);
    BOOST_CHECK(std::fabs(result.get(18, 1) - 0.517799) < 0.000001);
    BOOST_CHECK(std::fabs(result.get(19, 1) - 0.585526) < 0.000001);

    /** @arg operator() */
    Indicator ama = AMA(10, 2, 30);
    BOOST_CHECK(ama.size() == 0);
    Indicator expect = AMA(ind, 10, 2, 30);
    result = ama(ind);
    BOOST_CHECK(result.size() == expect.size());
    BOOST_CHECK(result.getResultNumber() == expect.getResultNumber());
    BOOST_CHECK(result.discard() == expect.discard());
    for (size_t i = 0; i < expect.size(); ++i) {
        BOOST_CHECK(result[i] == expect[i]);
        BOOST_CHECK(result.get(i, 1) == expect.get(i, 1));
    }
}

/** @} */

