/*
 * test_PRICELIST.cpp
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_indicator_suite
    #include <boost/test/unit_test.hpp>
#endif

#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/MA.h>

using namespace hku;

/**
 * @defgroup test_indicator_PRICELIST
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_PRICELIST ) {
    PriceList tmp_list;
    Indicator result;

    /** @arg PriceList 为空 */
    result = PRICELIST(tmp_list);
    BOOST_CHECK(result.size() == tmp_list.size());
    BOOST_CHECK(result.empty() == true);

    /** @arg PriceList 非空 */
    for (size_t i = 0; i < 10; ++i) {
        tmp_list.push_back(i);
    }
    result = PRICELIST(tmp_list);
    BOOST_CHECK(result.size() == tmp_list.size());
    BOOST_CHECK(result.empty() == false);
    for (size_t i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == tmp_list[i]);
    }

    /** @arg 数组指针为空 */
    price_t *p_tmp = NULL;
    result = PRICELIST(p_tmp, 10);
    BOOST_CHECK(result.size() == 0);
    BOOST_CHECK(result.empty() == true);

    /** @arg 数组指针非空 */
    price_t tmp[10];
    for (size_t i = 0; i < 10; ++i) {
        tmp[i] = i;
    }
    result = PRICELIST(tmp, 10);
    for (size_t i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == tmp[i]);
    }

    /** @arg 截取PriceList全部 */
    result = PRICELIST(tmp_list, 0, 10);
    BOOST_CHECK(result.size() == 10);
    for (size_t i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == tmp_list[i]);
    }

    /** @arg 截取PriceLIst， PriceList为空 */
    result = PRICELIST(PriceList());
    BOOST_CHECK(result.size() == 0);

    /** @arg 截取PriceList, start == end */
    result = PRICELIST(tmp_list, 1, 1);
    BOOST_CHECK(result.size() == 0);

    /** @arg 截取PriceList, start > end */
    result = PRICELIST(tmp_list, 1, 0);
    BOOST_CHECK(result.size() == 0);

    /** @arg 截取PriceList, end > size() */
    result = PRICELIST(tmp_list, 0, 100);
    BOOST_CHECK(result.size() == 10);
    for (size_t i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == tmp_list[i]);
    }

    /** @arg 截取PriceList，正常情况 */
    result = PRICELIST(tmp_list, 1, 2);
    BOOST_CHECK(result.size() == 1);
    BOOST_CHECK(result[0] == 1);

    /** @arg 截取Indicator，Indicator为空 */
    result = PRICELIST(Indicator(), 0, 0, 1);
    BOOST_CHECK(result.size() == 0);

    /** @arg 截取Indicator，start == end */
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KQuery query(0, 30);
    KData kdata = stock.getKData(query);
    Indicator ikdata = KDATA(kdata);
    result = PRICELIST(ikdata, 0, 1, 1);
    BOOST_CHECK(result.size() == 0);

    /** @arg 截取Indicator, start > end */
    result = PRICELIST(ikdata, 0, 1, 0);
    BOOST_CHECK(result.size() == 0);

    /** @arg 截取Indicator，end > size() */
    result = PRICELIST(ikdata, 0, 0, 100);
    BOOST_CHECK(result.size() == 30);
    for (size_t i = 0; i < result.size(); ++i) {
        BOOST_CHECK(result[i] == ikdata.get(i, 0));
    }

    /** @arg 截取Indicator, 指定结果集大于等于结果集数量 */
    result = PRICELIST(ikdata, 6, 0, 100);
    BOOST_CHECK(result.size() == 0);
    result = PRICELIST(ikdata, 7, 0, 100);
    BOOST_CHECK(result.size() == 0);

    /** @arg 截取Indicator全部，Indicator的抛弃数量不为零 */
    Indicator close = CLOSE(kdata);
    Indicator ma = MA(close, 10);
    result = PRICELIST(ma, 0, 0, 30);
    BOOST_CHECK(result.size() == 30);
    BOOST_CHECK(result.discard() == ma.discard());
    for (size_t i = 0; i < result.size(); ++i) {
        BOOST_CHECK(result[i] == ma[i]);
    }

    /** @arg 截取Indicator，Indicator的抛弃数量不为零，start > discard */
    result = PRICELIST(ma, 0, 10, 20);
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result[0] == ma[10]);
    BOOST_CHECK(result[0] != Null<price_t>());
    BOOST_CHECK(result[9] == ma[19]);

    /** @arg 截取Indicator，Indicator的抛弃数量不为零，start == discard */
    BOOST_CHECK(ma.discard() == 9);
    result = PRICELIST(ma, 0, 9, 20);
    BOOST_CHECK(result.size() == 11);
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result[0] == ma[9]);
    BOOST_CHECK(result[0] != Null<price_t>());
    BOOST_CHECK(result[1] == ma[10]);

    /** @arg 截取Indicator，Indicator的抛弃数量不为零，start < discard */
    result = PRICELIST(ma, 0, 8, 20);
    BOOST_CHECK(result.size() == 12);
    BOOST_CHECK(result.discard() == 1);
    BOOST_CHECK(result[0] == ma[8]);
    BOOST_CHECK(result[0] == Null<price_t>());
    BOOST_CHECK(result[1] == ma[9]);

    /** @arg 传入的PriceList中有Null<price_t>() */
    tmp_list.clear();
    for (size_t i = 0; i < 3; ++i) {
        tmp_list.push_back(Null<price_t>());
    }
    for (size_t i = 3; i < 10; ++i) {
        tmp_list.push_back(i);
    }
    result = PRICELIST(tmp_list);
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.discard() == 3);
    for (size_t i = 0; i < result.size(); ++i) {
        BOOST_CHECK(result[i] == tmp_list[i]);
    }
    for (size_t i = 0; i < result.discard(); ++i) {
        BOOST_CHECK(result[i] == Null<price_t>());
    }

    tmp_list[6] = Null<price_t>();
    result = PRICELIST(tmp_list);
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.discard() == 7);
    for (size_t i = 0; i < result.discard(); ++i) {
        BOOST_CHECK(result[i] == Null<price_t>());
    }
    for (size_t i = result.discard(); i < result.size(); ++i) {
        BOOST_CHECK(result[i] == tmp_list[i]);
    }

    result = PRICELIST(tmp_list, 4, tmp_list.size());
    BOOST_CHECK(result.size() == 6);
    BOOST_CHECK(result.discard() == 3);
    for (size_t i = 0; i < result.discard(); ++i) {
        BOOST_CHECK(result[i] == Null<price_t>());
    }
    for (size_t i = result.discard(); i < result.size(); ++i) {
        BOOST_CHECK(result[i] == tmp_list[i + 4]);
    }

    for (size_t i = 0; i < 10; ++i) {
        tmp[i] = i;
    }
    tmp[4] = Null<price_t>();
    result = PRICELIST(tmp, 10);
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.discard() == 5);
    for (size_t i = 0; i < result.discard(); ++i) {
        BOOST_CHECK(result[i] == Null<price_t>());
    }
    for (size_t i = result.discard(); i < result.size(); ++i) {
        BOOST_CHECK(result[i] == tmp[i]);
    }
}

/** @} */
