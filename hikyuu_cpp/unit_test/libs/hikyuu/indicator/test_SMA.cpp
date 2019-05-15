/*
 * test_SMA.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 * 
 *  Created on: 2019-5-16
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
#include <hikyuu/indicator/crt/SMA.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_SMA test_indicator_SMA
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_SMA ) {
    Indicator result;
    PriceList a;
    Indicator data = PRICELIST(a);

    /** @arg 输入指标长度为0 */
    result = SMA(data);
    BOOST_CHECK(result.name() == "SMA");
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result.size() == 0);

    /** @arg 输入指标长度为1，n = 2, m=2 */
    a.push_back(10);
    data = PRICELIST(a);
    result = SMA(data, 2, 2);
    BOOST_CHECK(result.name() == "SMA");
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result.size() == 1);
    BOOST_CHECK(result[0] == 10);

    /** @arg 输入指标长度为1，n = 1, m=2 */
    a.clear();
    a.push_back(10);
    data = PRICELIST(a);
    result = SMA(data, 1, 2);
    BOOST_CHECK(result.name() == "SMA");
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result.size() == 1);
    BOOST_CHECK(result[0] == 10);

    /** @arg 输入指标长度为10，n = 1, m=1 */
    a.clear();    
    for (int i = 0; i < 10; ++i) {
        a.push_back(i/10.0);
    }
    data = PRICELIST(a);
    result = SMA(data, 1, 1);
    BOOST_CHECK(result.name() == "SMA");
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result.size() == data.size());
    for (size_t i = result.discard(); i < data.size(); i++) {
        BOOST_CHECK(result[i] == data[i]);
    }

    /** @arg 输入指标长度为1，n = 4, m=2 */
    result = SMA(data, 4, 2);
    BOOST_CHECK(result.name() == "SMA");
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result.size() == data.size());
    BOOST_CHECK(result[0] == 0);
    BOOST_CHECK(result[1] == 0.05);
    BOOST_CHECK_CLOSE(result[5], 0.4031, 0.01);
    BOOST_CHECK_CLOSE(result[9], 0.8002, 0.01);
}


//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_SMA_export ) {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/SMA.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = SMA(CLOSE(kdata));
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

    BOOST_CHECK(x2.name() == "SMA");
    BOOST_CHECK(x1.size() == x2.size());
    BOOST_CHECK(x1.discard() == x2.discard());
    BOOST_CHECK(x1.getResultNumber() == x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        BOOST_CHECK_CLOSE(x1[i], x2[i], 0.00001);
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */


