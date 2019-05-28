/*
 * test_ALIGN.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 * 
 *  Created on: 2019-5-1
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
#include <hikyuu/indicator/crt/ALIGN.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_ALIGN test_indicator_ALIGN
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_ALIGN ) {
    Indicator result;
    Stock stk = getStock("sh000001");

    PriceList a;
    for (int i = 0; i < 10; i++) {
        a.push_back(i);
    }

    /** @arg 输入指标本身和上下文无关，参考日期长度为0 */
    DatetimeList ref;
    Indicator data = PRICELIST(a);
    result = ALIGN(data, ref);
    BOOST_CHECK(result.name() == "ALIGN");
    BOOST_CHECK(ref.size() == 0);
    BOOST_CHECK(result.size() == 0);
    BOOST_CHECK(result.discard() == 0);

    /** @arg 输入指标本身和上下文无关，和参考日期列表等长 */
    ref = stk.getDatetimeList(KQuery(-10));
    data = PRICELIST(a);
    result = ALIGN(data, ref);
    BOOST_CHECK(result.name() == "ALIGN");
    BOOST_CHECK(result.size() == ref.size());
    BOOST_CHECK(result.discard() == 0);
    DatetimeList result_dates = result.getDatetimeList();
    for (int i = 0; i < result.size(); i++) {
        BOOST_CHECK(result[i] == data[i]);
        BOOST_CHECK(result_dates[i] == ref[i]);
    }

    /** @arg 输入指标本身和上下文无关，且长度长于参考日期列表 */
    a.push_back(11);
    data = PRICELIST(a);
    result = ALIGN(data, ref);
    BOOST_CHECK(result.name() == "ALIGN");
    BOOST_CHECK(result.size() == ref.size());
    BOOST_CHECK(result.discard() == 0);
    result_dates = result.getDatetimeList();
    for (int i = 0; i < result.size(); i++) {
        BOOST_CHECK(result[i] == data[i+1]);
        BOOST_CHECK(result_dates[i] == ref[i]);
    }

    /** @arg 输入指标本身和上下文无关，且长度小于参考日期列表 */
    a.clear();
    a.push_back(1);
    data = PRICELIST(a);
    result = ALIGN(data, ref);
    BOOST_CHECK(result.name() == "ALIGN");
    BOOST_CHECK(result.size() == ref.size());
    BOOST_CHECK(result.discard() == 9);
    result_dates = result.getDatetimeList();
    for (int i = 0; i < result.discard(); i++) {
        BOOST_CHECK(std::isnan(result[i]));
        BOOST_CHECK(result_dates[i] == ref[i]);
    }
    BOOST_CHECK(result[9] == 1);
    BOOST_CHECK(result_dates[9] == ref[9]);

    /** @arg 输入指标本身和上下文无关，且长度为0 */
    a.clear();
    data = PRICELIST(a);
    result = ALIGN(data, ref);
    BOOST_CHECK(result.name() == "ALIGN");
    BOOST_CHECK(result.size() == ref.size());
    BOOST_CHECK(result.discard() == ref.size());
    result_dates = result.getDatetimeList();
    for (int i = 0; i < result.discard(); i++) {
        BOOST_CHECK(std::isnan(result[i]));
        BOOST_CHECK(result_dates[i] == ref[i]);
    }

    /** @arg ind对应日期全部大于参考日期 */
    KData k = stk.getKData(KQuery(-10));
    ref.clear();
    ref.push_back(Datetime(201901010000));
    ref.push_back(Datetime(201901020000));
    ref.push_back(Datetime(201901030000));
    data = CLOSE(k);
    result = ALIGN(data, ref);
    BOOST_CHECK(result.name() == "ALIGN");
    BOOST_CHECK(result.size() == ref.size());
    BOOST_CHECK(result.discard() == ref.size());
    result_dates = result.getDatetimeList();
    for (int i = 0; i < result.discard(); i++) {
        BOOST_CHECK(std::isnan(result[i]));
        BOOST_CHECK(result_dates[i] == ref[i]);
    }

    /** @arg ind对应日期全部小于参考日期 */
    ref.clear();
    ref.push_back(Datetime(191901010000));
    ref.push_back(Datetime(191901020000));
    ref.push_back(Datetime(191901030000));
    data = CLOSE(k);
    result = ALIGN(data, ref);
    BOOST_CHECK(result.name() == "ALIGN");
    BOOST_CHECK(result.size() == ref.size());
    BOOST_CHECK(result.discard() == ref.size());
    result_dates = result.getDatetimeList();
    for (int i = 0; i < result.discard(); i++) {
        BOOST_CHECK(std::isnan(result[i]));
        BOOST_CHECK(result_dates[i] == ref[i]);
    }
 
    /** @arg ind对应日期等于参考日期 */
    ref = k.getDatetimeList();
    data = CLOSE(k);
    result = ALIGN(data, ref);
    BOOST_CHECK(result.name() == "ALIGN");
    BOOST_CHECK(result.size() == ref.size());
    BOOST_CHECK(result.discard() == 0);
    result_dates = result.getDatetimeList();
    for (int i = 0; i < result.size(); i++) {
        BOOST_CHECK(result[i] == data[i]);
        BOOST_CHECK(result_dates[i] == ref[i]);
    }

    /** @arg ind对应部分日期 */
    ref.clear();
    ref.push_back(Datetime(201111220000));
    ref.push_back(Datetime(201111230000));
    ref.push_back(Datetime(201111240000));
    ref.push_back(Datetime(201111260000));
    ref.push_back(Datetime(201112060000));
    ref.push_back(Datetime(201112070000));
    ref.push_back(Datetime(201112100000));
    data = CLOSE(k);
    result = ALIGN(data, ref);
    BOOST_CHECK(result.name() == "ALIGN");
    BOOST_CHECK(result.size() == ref.size());
    BOOST_CHECK(result.discard() == 1);
    BOOST_CHECK(std::isnan(result[0]));
    BOOST_CHECK_CLOSE(result[1], 2395.07, 0.01);
    BOOST_CHECK_CLOSE(result[2], 2397.55, 0.01);
    BOOST_CHECK(std::isnan(result[3]));
    BOOST_CHECK_CLOSE(result[4], 2325.91, 0.01);
    BOOST_CHECK(std::isnan(result[5]));
    BOOST_CHECK(std::isnan(result[6]));
    for (int i = 0; i < result.size(); i++) {
        BOOST_CHECK(result.getDatetime(i) == ref[i]);
    }
}


//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_ALIGN_export ) {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/ALIGN.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = ALIGN(CLOSE(kdata), sm.getStock("sh600004").getDatetimeList(KQuery(-20)));
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(x1);
    }

    Indicator x2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(x2);
    }

    BOOST_CHECK(x2.name() == "ALIGN");
    BOOST_CHECK(x1.size() == x2.size());
    BOOST_CHECK(x1.discard() == x2.discard());
    BOOST_CHECK(x1.getResultNumber() == x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        BOOST_CHECK_CLOSE(x1[i], x2[i], 0.00001);
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */


