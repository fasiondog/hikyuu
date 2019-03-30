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

#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/KDATA.h>

using namespace hku;

/**
 * @defgroup test_indicator_PRICELIST test_indicator_PRICELIST
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

    /** @arg 从PriceList构造 */
    result = PRICELIST(tmp_list);
    BOOST_CHECK(result.size() == 10);
    for (size_t i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == tmp_list[i]);
    }

    /** @arg 从PriceLIst， discard为1 */
    result = PRICELIST(tmp_list, 1);
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.discard() == 1);
    BOOST_CHECK(result[0] == Null<price_t>());
    for (size_t i = 1; i < 10; ++i) {
        BOOST_CHECK(result[i] == tmp_list[i]);
    }

    /** @arg 待转化数据为Indicator，Indicator为空 */
    result = PRICELIST(Indicator());
    BOOST_CHECK(result.size() == 0);

    /** @arg 待转化数据为Indicator, result_num=0 */
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KQuery query(0, 30);
    KData kdata = stock.getKData(query);
    Indicator ikdata = KDATA(kdata);
    BOOST_CHECK(ikdata.size() == 30);
    result = PRICELIST(ikdata);
    BOOST_CHECK(result.size() == ikdata.size());
    BOOST_CHECK(result.discard() == ikdata.discard());
    for (size_t i = 0; i < result.size(); ++i) {
        BOOST_CHECK(result[i] == ikdata[i]);
    }

    /** @arg 待转化数据为Indicator, result_num=1 */
    result = PRICELIST(ikdata, 1);
    BOOST_CHECK(result.size() == ikdata.size());
    BOOST_CHECK(result.discard() == ikdata.discard());
    for (size_t i = 0; i < result.size(); ++i) {
        BOOST_CHECK(result[i] == ikdata.get(i, 1));
    }
}


//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_PRICELIST_export ) {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/PRICELIST.xml";

    PriceList d;
    for (size_t i = 0; i < 20; ++i) {
        d.push_back(i);
    }

    Indicator ma1 = PRICELIST(d);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(ma1);
    }

    Indicator ma2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(ma2);
    }

    BOOST_CHECK(ma1.size() == ma2.size());
    BOOST_CHECK(ma1.discard() == ma2.discard());
    BOOST_CHECK(ma1.getResultNumber() == ma2.getResultNumber());
    for (size_t i = 0; i < ma1.size(); ++i) {
        BOOST_CHECK_CLOSE(ma1[i], ma2[i], 0.00001);
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
