/*
 * test_Indicator.cpp
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

#include <hikyuu/indicator/Indicator.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/StockManager.h>

using namespace hku;

/**
 * @defgroup test_indicator_Indicator test_indicator_Indicator
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_operator_add ) {
    /** @arg 正常相加*/
    PriceList d1, d2;
    for (size_t i = 0; i < 10; ++i) {
        d1.push_back(i);
        d2.push_back(i+1);
    }

    Indicator data1 = PRICELIST(d1);
    Indicator data2 = PRICELIST(d2);
    Indicator result = data1 + data2;
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.getResultNumber() == 1);
    BOOST_CHECK(result.discard() == 0);
    for (size_t i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == i + i + 1);
    }

    /** @arg 两个待加的ind的size不同 */
    Indicator data3;
    result = data1 + data3;
    BOOST_CHECK(result.empty());
    BOOST_CHECK(result.size() == 0);

    /** @arg 两个待加的ind的size相同，但result_number不同 */
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KQuery query(0, 10);
    KData kdata = stock.getKData(query);
    Indicator k = KDATA(kdata);
    BOOST_CHECK(k.size() == data1.size());
    result = k + data1;
    BOOST_CHECK(result.size() == k.size());
    BOOST_CHECK(result.getResultNumber() == 1);
    for (size_t i = 0; i < result.size(); ++i) {
        BOOST_CHECK(result[i] == (k[i] + data1[i]));
    }
}

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_operator_reduce ) {
    /** @arg 正常相减*/
    PriceList d1, d2;
    for (size_t i = 0; i < 10; ++i) {
        d1.push_back(i);
        d2.push_back(i+1);
    }

    Indicator data1 = PRICELIST(d1);
    Indicator data2 = PRICELIST(d2);
    Indicator result = data1 - data2;
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.getResultNumber() == 1);
    BOOST_CHECK(result.discard() == 0);
    for (size_t i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == data1[i] - data2[i]);
    }

    /** @arg 两个待减的ind的size不同 */
    Indicator data3;
    result = data1 - data3;
    BOOST_CHECK(result.empty());
    BOOST_CHECK(result.size() == 0);

    /** @arg 两个待减的ind的size相同，但result_number不同 */
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KQuery query(0, 10);
    KData kdata = stock.getKData(query);
    Indicator k = KDATA(kdata);
    BOOST_CHECK(k.size() == data1.size());
    result = k - data1;
    BOOST_CHECK(result.size() == k.size());
    for (size_t i = 0; i < result.size(); ++i) {
        BOOST_CHECK(result[i] == (k[i] - data1[i]));
    }
}


/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_operator_multi ) {
    /** @arg 正常相乘*/
    PriceList d1, d2;
    for (size_t i = 0; i < 10; ++i) {
        d1.push_back(i);
        d2.push_back(i+1);
    }

    Indicator data1 = PRICELIST(d1);
    Indicator data2 = PRICELIST(d2);
    Indicator result = data1 * data2;
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.getResultNumber() == 1);
    BOOST_CHECK(result.discard() == 0);
    for (size_t i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == data1[i] * data2[i]);
    }

    /** @arg 两个待乘的ind的size不同 */
    Indicator data3;
    result = data1 * data3;
    BOOST_CHECK(result.empty());
    BOOST_CHECK(result.size() == 0);

    /** @arg 两个待乘的ind的size相同，但result_number不同 */
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KQuery query(0, 10);
    KData kdata = stock.getKData(query);
    Indicator k = KDATA(kdata);
    BOOST_CHECK(k.size() == data1.size());
    result = k * data1;
    BOOST_CHECK(result.size() == k.size());
    for (size_t i = 0; i < result.size(); ++i) {
        BOOST_CHECK(result[i] == (k[i] * data1[i]));
    }
}


/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_operator_division ) {
    /** @arg 正常相除*/
    PriceList d1, d2;
    for (size_t i = 0; i < 10; ++i) {
        d1.push_back(i);
        d2.push_back(i+1);
    }

    Indicator data1 = PRICELIST(d1);
    Indicator data2 = PRICELIST(d2);
    Indicator result = data2 / data1;
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.getResultNumber() == 1);
    BOOST_CHECK(result.discard() == 0);
    for (size_t i = 0; i < 10; ++i) {
        if (data1[i] == 0.0) {
            BOOST_CHECK(result[i] == Null<price_t>());
        } else {
            BOOST_CHECK(result[i] == data2[i] / data1[i]);
        }
    }

    /** @arg 两个待除的ind的size不同 */
    Indicator data3;
    result = data1 / data3;
    BOOST_CHECK(result.empty());
    BOOST_CHECK(result.size() == 0);

    /** @arg 两个待除的ind的size相同，但result_number不同 */
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KQuery query(0, 10);
    KData kdata = stock.getKData(query);
    Indicator k = KDATA(kdata);
    BOOST_CHECK(k.size() == data1.size());
    result = k / data1;
    BOOST_CHECK(result.size() == k.size());
    for (size_t i = 0; i < result.size(); ++i) {
        if (data1[i] == 0.0) {
            BOOST_CHECK(result[i] == Null<price_t>());
        } else {
            BOOST_CHECK(result[i] == (k[i] / data1[i]));
        }
    }
}

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_operator_eq ) {
    /** @arg 正常相等*/
    PriceList d1, d2;
    for (size_t i = 0; i < 10; ++i) {
        d1.push_back(i);
        d2.push_back(i);
    }

    Indicator data1 = PRICELIST(d1);
    Indicator data2 = PRICELIST(d2);
    Indicator result = (data2 == data1);
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.getResultNumber() == 1);
    BOOST_CHECK(result.discard() == 0);
    for (size_t i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == true);
    }

    /** @arg 两个ind的size不同 */
    Indicator data3;
    result = (data1 == data3);
    BOOST_CHECK(result.empty());
    BOOST_CHECK(result.size() == 0);

    /** @arg 两个ind的size相同，但result_number不同 */
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KQuery query(0, 10);
    KData kdata = stock.getKData(query);
    Indicator k = KDATA(kdata);
    BOOST_CHECK(k.size() == data1.size());
    result = (k == data1);
    BOOST_CHECK(result.size() == k.size());
    for (size_t i = 0; i < result.size(); ++i) {
        BOOST_CHECK(result[i] == false);
    }
}

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_operator_ne ) {
    /** @arg 正常不相等 */
    PriceList d1, d2;
    for (size_t i = 0; i < 10; ++i) {
        d1.push_back(i);
        d2.push_back(i);
    }

    Indicator data1 = PRICELIST(d1);
    Indicator data2 = PRICELIST(d2);
    Indicator result = (data2 != data1);
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.getResultNumber() == 1);
    BOOST_CHECK(result.discard() == 0);
    for (size_t i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == false);
    }

    /** @arg 两个ind的size不同  */
    Indicator data3;
    result = (data1 != data3);
    BOOST_CHECK(result.empty());
    BOOST_CHECK(result.size() == 0);

    /** @arg 两个ind的size相同，但result_number不同 */
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KQuery query(0, 10);
    KData kdata = stock.getKData(query);
    Indicator k = KDATA(kdata);
    BOOST_CHECK(k.size() == data1.size());
    result = (k != data1);
    BOOST_CHECK(result.size() == k.size());
    for (size_t i = 0; i < result.size(); ++i) {
        BOOST_CHECK(result[i] == true);
    }
}

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_operator_gt ) {
    PriceList d1, d2, d3;
    for (size_t i = 0; i < 10; ++i) {
        d1.push_back(i);
        d2.push_back(i);
        d3.push_back(i+1);
    }

    Indicator data1 = PRICELIST(d1);
    Indicator data2 = PRICELIST(d2);
    Indicator data3 = PRICELIST(d3);

    /** @arg ind1 > ind2*/
    Indicator result = (data3 > data1);
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.getResultNumber() == 1);
    BOOST_CHECK(result.discard() == 0);
    for (size_t i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == 1.0);
    }

    /** @arg ind1 < ind2 */
    result = (data1 > data3);
    for (size_t i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == 0.0);
    }

    /** @arg ind1 == ind2 */
    result = (data1 > data2);
    for (size_t i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == 0.0);
    }

    /** @arg 两个ind的size不同 */
    Indicator data4;
    result = data1 > data4;
    BOOST_CHECK(result.empty());
    BOOST_CHECK(result.size() == 0);

    /** @arg 两个ind的size相同，但result_number不同 */
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KQuery query(0, 10);
    KData kdata = stock.getKData(query);
    Indicator k = KDATA(kdata);
    BOOST_CHECK(k.size() == data1.size());
    result = (k > data1);
    BOOST_CHECK(result.size() == k.size());
    for (size_t i = 0; i < result.size(); ++i) {
        BOOST_CHECK(result[i] == 1.0);
    }
}

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_operator_ge ) {
    PriceList d1, d2, d3;
    for (size_t i = 0; i < 10; ++i) {
        d1.push_back(i);
        d2.push_back(i);
        d3.push_back(i+1);
    }

    Indicator data1 = PRICELIST(d1);
    Indicator data2 = PRICELIST(d2);
    Indicator data3 = PRICELIST(d3);

    /** @arg ind1 > ind2*/
    Indicator result = (data3 >= data1);
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.getResultNumber() == 1);
    BOOST_CHECK(result.discard() == 0);
    for (size_t i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == 1.0);
    }

    /** @arg ind1 < ind2 */
    result = (data1 >= data3);
    for (size_t i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == 0.0);
    }

    /** @arg ind1 == ind2 */
    result = (data1 >= data2);
    for (size_t i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == 1.0);
    }

    /** @arg 两个ind的size不同 */
    Indicator data4;
    result = data1 >= data4;
    BOOST_CHECK(result.empty());
    BOOST_CHECK(result.size() == 0);

    /** @arg 两个ind的size相同，但result_number不同 */
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KQuery query(0, 10);
    KData kdata = stock.getKData(query);
    Indicator k = KDATA(kdata);
    BOOST_CHECK(k.size() == data1.size());
    result = (k >= data1);
    BOOST_CHECK(result.size() == k.size());
    for (size_t i = 0; i < result.size(); ++i) {
        BOOST_CHECK(result[i] == 1.0);
    }
}

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_operator_lt ) {
    PriceList d1, d2, d3;
    for (size_t i = 0; i < 10; ++i) {
        d1.push_back(i);
        d2.push_back(i);
        d3.push_back(i+1);
    }

    Indicator data1 = PRICELIST(d1);
    Indicator data2 = PRICELIST(d2);
    Indicator data3 = PRICELIST(d3);

    /** @arg ind1 > ind2*/
    Indicator result = (data3 < data1);
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.getResultNumber() == 1);
    BOOST_CHECK(result.discard() == 0);
    for (size_t i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == 0.0);
    }

    /** @arg ind1 < ind2 */
    result = (data1 < data3);
    for (size_t i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == 1.0);
    }

    /** @arg ind1 == ind2 */
    result = (data1 < data2);
    for (size_t i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == 0.0);
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
BOOST_AUTO_TEST_CASE( test_operator_le ) {
    PriceList d1, d2, d3;
    for (size_t i = 0; i < 10; ++i) {
        d1.push_back(i);
        d2.push_back(i);
        d3.push_back(i+1);
    }

    Indicator data1 = PRICELIST(d1);
    Indicator data2 = PRICELIST(d2);
    Indicator data3 = PRICELIST(d3);

    /** @arg ind1 > ind2*/
    Indicator result = (data3 <= data1);
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.getResultNumber() == 1);
    BOOST_CHECK(result.discard() == 0);
    for (size_t i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == 0.0);
    }

    /** @arg ind1 < ind2 */
    result = (data1 <= data3);
    for (size_t i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == 1.0);
    }

    /** @arg ind1 == ind2 */
    result = (data1 <= data2);
    for (size_t i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == 1.0);
    }

    /** @arg 两个ind的size不同 */
    Indicator data4;
    result = data1 <= data4;
    BOOST_CHECK(result.empty());
    BOOST_CHECK(result.size() == 0);

    /** @arg 两个ind的size相同，但result_number不同 */
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KQuery query(0, 10);
    KData kdata = stock.getKData(query);
    Indicator k = KDATA(kdata);
    BOOST_CHECK(k.size() == data1.size());
    result = (k <= data1);
    BOOST_CHECK(result.size() == k.size());
    for (size_t i = 0; i < result.size(); ++i) {
        BOOST_CHECK(result[i] == 0.0);
    }
}


/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_getResult_getResultAsPriceList ) {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KQuery query;
    KData kdata;
    Indicator ikdata, result1;
    PriceList result2;

    /** @arg 源数据为空 */
    ikdata = KDATA(kdata);
    result1 = ikdata.getResult(0);
    result2 = ikdata.getResultAsPriceList(0);
    BOOST_CHECK(result1.size() == 0);
    BOOST_CHECK(result2.size() == 0);

    /** @arg result_num参数非法 */
    query = KQuery(0, 10);
    kdata = stock.getKData(query);
    ikdata = KDATA(kdata);
    BOOST_CHECK(ikdata.size() == 10);
    result1 = ikdata.getResult(6);
    result2 = ikdata.getResultAsPriceList(6);
    BOOST_CHECK(result1.size() == 0);
    BOOST_CHECK(result2.size() == 0);

    /** @arg 正常获取 */
    result1 = ikdata.getResult(0);
    result2 = ikdata.getResultAsPriceList(1);
    BOOST_CHECK(result1.size() == 10);
    BOOST_CHECK(result2.size() == 10);
    BOOST_CHECK(result1[0] == 29.5);
    BOOST_CHECK(std::fabs(result1[1] - 27.58) < 0.0001);
    BOOST_CHECK(result1[9] == 26.45);

    BOOST_CHECK(result2[0] == 29.8);
    BOOST_CHECK(result2[1] == 28.38);
    BOOST_CHECK(result2[9] == 26.55);
}

/** @} */
