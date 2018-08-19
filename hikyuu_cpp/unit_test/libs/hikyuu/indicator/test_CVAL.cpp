/*
 * test_CVAL.cpp
 *
 *  Created on: 2017年6月25日
 *      Author: Administrator
 */

#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_indicator_suite
    #include <boost/test/unit_test.hpp>
#endif

#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_CVAL test_indicator_CVAL
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_CVAL ) {
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

    /** @arg operator(ind) */
    Indicator ind = PRICELIST(d);
    BOOST_CHECK(ind.size() == 20);
    result = CVAL(ind, 100);
    BOOST_CHECK(result.getParam<double>("value") == 100);
    BOOST_CHECK(result.size() == 20);
    BOOST_CHECK(result.empty() == false);
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result.getResultNumber() == 1);
    for (size_t i = 0; i < ind.size(); ++i) {
        BOOST_CHECK(result[i] == 100);
    }

    /** @arg operator() */
    result = CVAL(100);
    BOOST_CHECK(result.getParam<double>("value") == 100);
    BOOST_CHECK(result.size() == 0);
    BOOST_CHECK(result.empty() == true);
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result.getResultNumber() == 1);

    /** @arg 生成指定长度的常量Indicator, discard=0 */
    result = CVAL(100, 5);
    BOOST_CHECK(result.getParam<double>("value") == 100);
    BOOST_CHECK(result.size() == 5);
    BOOST_CHECK(result.empty() == false);
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result.getResultNumber() == 1);
    for (int i = 0; i < 5; i++) {
        BOOST_CHECK(result[i] == 100);
    }

    /** @arg 生成指定长度的常量Indicator, discard=2 */
    result = CVAL(100, 5, 2);
    BOOST_CHECK(result.getParam<double>("value") == 100);
    BOOST_CHECK(result.size() == 5);
    BOOST_CHECK(result.empty() == false);
    BOOST_CHECK(result.discard() == 2);
    BOOST_CHECK(result.getResultNumber() == 1);
    BOOST_CHECK(result[0] == Null<price_t>());
    BOOST_CHECK(result[1] == Null<price_t>());
    for (int i = 2; i < 5; i++) {
        BOOST_CHECK(result[i] == 100);
    }

}

/** @} */


