/*
 * test_SAFTYLOSS.cpp
 *
 *  Created on: 2013-4-12
 *      Author: fasiondog
 */

#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_indicator_suite
    #include <boost/test/unit_test.hpp>
#endif

#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/SAFTYLOSS.h>
#include <hikyuu/indicator/crt/KDATA.h>

using namespace hku;

/**
 * @defgroup test_indicator_SAFTYLOSS test_indicator_SAFTYLOSS
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_SAFTYLOSS ) {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KQuery query;
    KData kdata;
    Indicator close, result;

    /** @arg 源数据为空 */
    result = SAFTYLOSS(CLOSE(kdata), 2, 1);
    BOOST_CHECK(result.empty() == true);
    BOOST_CHECK(result.size() == 0);
    BOOST_CHECK(result.discard() == 1);

    /** @arg 参数n1、n2非法 */
    query = KQuery(0, 20);
    kdata = stock.getKData(query);
    close = CLOSE(kdata);
    result = SAFTYLOSS(close, 1, 1);
    BOOST_CHECK(result.empty() == false);
    BOOST_CHECK(result.size() == close.size());
    BOOST_CHECK(result.discard() == 0);
    for (size_t i = 0; i < result.size(); ++i) {
        BOOST_CHECK(result[i] == Null<price_t>());
    }

    result = SAFTYLOSS(close, 0, 0);
    BOOST_CHECK(result.empty() == false);
    BOOST_CHECK(result.size() == close.size());
    BOOST_CHECK(result.discard() == 0);

    result = SAFTYLOSS(close, 2, 0);
    BOOST_CHECK(result.empty() == false);
    BOOST_CHECK(result.size() == close.size());
    BOOST_CHECK(result.discard() == 0);

    /** @arg 正常参数 */
    result = SAFTYLOSS(close, 2, 1, 1.0);
    BOOST_CHECK(result.empty() == false);
    BOOST_CHECK(result.size() == close.size());
    BOOST_CHECK(result.discard() == 1);
    BOOST_CHECK(result[0] == Null<price_t>());
    BOOST_CHECK(result[1] == 27.67);
    BOOST_CHECK(result[2] == 28.05);
    BOOST_CHECK(result[3] == 27.45);
    BOOST_CHECK(std::fabs(result[19] - 25.54) < 0.0001);

    result = SAFTYLOSS(close, 2, 2, 1.0);
    BOOST_CHECK(result.empty() == false);
    BOOST_CHECK(result.size() == close.size());
    BOOST_CHECK(result.discard() == 2);
    BOOST_CHECK(result[0] == Null<price_t>());
    BOOST_CHECK(result[1] == Null<price_t>());
    BOOST_CHECK(result[2] == 28.05);
    BOOST_CHECK(result[3] == 28.05);
    BOOST_CHECK(result[4] == 27.45);
    BOOST_CHECK(std::fabs(result[19] - 25.54) < 0.0001);

    result = SAFTYLOSS(close, 3, 2, 2.0);
    BOOST_CHECK(result.empty() == false);
    BOOST_CHECK(result.size() == close.size());
    BOOST_CHECK(result.discard() == 3);
    BOOST_CHECK(result[0] == Null<price_t>());
    BOOST_CHECK(result[1] == Null<price_t>());
    BOOST_CHECK(result[2] == Null<price_t>());
    BOOST_CHECK(std::fabs(result[3] - 27.97) < 0.0001);
    BOOST_CHECK(std::fabs(result[4] - 27.15) < 0.0001);
    BOOST_CHECK(std::fabs(result[5] - 25.05) < 0.0001);
    BOOST_CHECK(std::fabs(result[19] - 24.84) < 0.0001);

    result = SAFTYLOSS(close, 3, 3, 2.0);
    BOOST_CHECK(result.empty() == false);
    BOOST_CHECK(result.size() == close.size());
    BOOST_CHECK(result.discard() == 4);
    BOOST_CHECK(result[0] == Null<price_t>());
    BOOST_CHECK(result[1] == Null<price_t>());
    BOOST_CHECK(result[2] == Null<price_t>());
    BOOST_CHECK(result[3] == Null<price_t>());
    BOOST_CHECK(std::fabs(result[4] - 27.97) < 0.0001);
    BOOST_CHECK(std::fabs(result[5] - 27.15) < 0.0001);
    BOOST_CHECK(std::fabs(result[6] - 26.7) < 0.0001);
    BOOST_CHECK(std::fabs(result[19] - 25.68) < 0.0001);

    result = SAFTYLOSS(close, 10, 3, 2.0);
    BOOST_CHECK(result.empty() == false);
    BOOST_CHECK(result.size() == close.size());
    BOOST_CHECK(result.discard() == 11);
    BOOST_CHECK(result[0] == Null<price_t>());
    BOOST_CHECK(result[1] == Null<price_t>());
    BOOST_CHECK(result[2] == Null<price_t>());
    BOOST_CHECK(result[10] == Null<price_t>());
    BOOST_CHECK(std::fabs(result[11] - 25.7486) < 0.0001);
    BOOST_CHECK(std::fabs(result[12] - 25.79) < 0.0001);
    BOOST_CHECK(std::fabs(result[13] - 26.03) < 0.0001);
    BOOST_CHECK(std::fabs(result[19] - 26.105) < 0.0001);

    /** @arg operator() */
    Indicator ind = SAFTYLOSS(10, 3, 2.0);
    BOOST_CHECK(ind.size() == 0);
    result = ind(close);
    Indicator expect = SAFTYLOSS(close, 10, 3, 2.0);
    BOOST_CHECK(result.size() == expect.size());
    BOOST_CHECK(result.size() != 0);
    BOOST_CHECK(result.discard() == expect.discard());
    for (size_t i = 0; i < expect.size(); ++i) {
        BOOST_CHECK(result[i] == expect[i]);
    }

}

/** @} */


