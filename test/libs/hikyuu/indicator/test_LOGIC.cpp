/*
 * test_LOGIC.cpp
 *
 *  Created on: 2017年7月13日
 *      Author: fasiondog
 */

#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_indicator_suite
    #include <boost/test/unit_test.hpp>
#endif

#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/IND_LOGIC.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/KDATA.h>

using namespace hku;

/**
 * @defgroup test_indicator_LOGIC test_indicator_LOGIC
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_LOGIC_AND ) {
    PriceList d1, d2, d3;
    for (size_t i = 0; i < 10; ++i) {
        d1.push_back(0);
        d2.push_back(1);
        d3.push_back(i);
    }

    Indicator data1 = PRICELIST(d1);
    Indicator data2 = PRICELIST(d2);
    Indicator data3 = PRICELIST(d3);

    /** @arg ind1为全0， ind2为全1 */
    Indicator result = IND_AND(data1, data2);
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.getResultNumber() == 1);
    BOOST_CHECK(result.discard() == 0);
    for (size_t i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == 0.0);
    }

    /** @arg ind为全0， val为1 */
    result = IND_AND(data1, 1.0);
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.getResultNumber() == 1);
    BOOST_CHECK(result.discard() == 0);
    for (size_t i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == 0.0);
    }

    /** @arg ind1为全0， ind2为从0开始的整数 */
    result = IND_AND(data1, data3);
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.getResultNumber() == 1);
    BOOST_CHECK(result.discard() == 0);
    for (size_t i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == 0.0);
    }

    /** @arg ind1为全1， ind2为从0开始的整数 */
    result = IND_AND(data2, data3);
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.getResultNumber() == 1);
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result[0] == 0.0);
    for (size_t i = 1; i < 10; ++i) {
        BOOST_CHECK(result[i] == 1.0);
    }

    /** @arg 两个ind的size不同 */
    Indicator data4;
    result = data1 < data4;
    BOOST_CHECK(result.empty());
    BOOST_CHECK(result.size() == 0);

    /** @arg 两个ind的size相同，但result_number不同 */
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KQuery query(0, 10);
    KData kdata = stock.getKData(query);
    Indicator k = KDATA(kdata);
    BOOST_CHECK(k.size() == data1.size());
    result = (k < data1);
    BOOST_CHECK(result.size() == k.size());
    for (size_t i = 0; i < result.size(); ++i) {
        BOOST_CHECK(result[i] == 0.0);
    }
}

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_LOGIC_OR ) {
    PriceList d1, d2, d3;
    for (size_t i = 0; i < 10; ++i) {
        d1.push_back(0);
        d2.push_back(1);
        d3.push_back(i);
    }

    Indicator data1 = PRICELIST(d1);
    Indicator data2 = PRICELIST(d2);
    Indicator data3 = PRICELIST(d3);

    /** @arg ind1为全0， ind2为全1 */
    Indicator result = IND_OR(data1, data2);
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.getResultNumber() == 1);
    BOOST_CHECK(result.discard() == 0);
    for (size_t i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == 1.0);
    }

    /** @arg ind为全0， val为1 */
    result = IND_OR(data1, 1.0);
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.getResultNumber() == 1);
    BOOST_CHECK(result.discard() == 0);
    for (size_t i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == 1.0);
    }

    /** @arg ind1为全0， ind2为从0开始的整数 */
    result = IND_OR(data1, data3);
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.getResultNumber() == 1);
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result[0] == 0.0);
    for (size_t i = 1; i < 10; ++i) {
        BOOST_CHECK(result[i] == 1.0);
    }

    /** @arg ind1为全1， ind2为从0开始的整数 */
    result = IND_OR(data2, data3);
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.getResultNumber() == 1);
    BOOST_CHECK(result.discard() == 0);
    for (size_t i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == 1.0);
    }

    /** @arg 两个ind的size不同 */
    Indicator data4;
    result = data1 < data4;
    BOOST_CHECK(result.empty());
    BOOST_CHECK(result.size() == 0);

    /** @arg 两个ind的size相同，但result_number不同 */
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KQuery query(0, 10);
    KData kdata = stock.getKData(query);
    Indicator k = KDATA(kdata);
    BOOST_CHECK(k.size() == data1.size());
    result = (k < data1);
    BOOST_CHECK(result.size() == k.size());
    for (size_t i = 0; i < result.size(); ++i) {
        BOOST_CHECK(result[i] == 0.0);
    }
}

/** @} */


