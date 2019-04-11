/*
 * test_HHVBARS.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 * 
 *  Created on: 2019年4月1日
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
#include <hikyuu/indicator/crt/HHV.h>
#include <hikyuu/indicator/crt/HHVBARS.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_HHVBARS test_indicator_HHVBARS
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_HHVBARS ) {
    StockManager& sm = StockManager::instance();
    Stock stk = sm.getStock("sh000001");
    KData k = stk.getKData(-10);
    Indicator c = CLOSE(k);

    Indicator result;

    /** @arg n = 0, 高点顺序上升 */
    PriceList a;
    for (int i = 0; i < 10; ++i) {
        a.push_back(i);
    }

    Indicator data = PRICELIST(a);
    result = HHVBARS(data, 0);
    BOOST_CHECK(result.name() == "HHVBARS");
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.discard() == 0);
    for (int i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == 0);
    }

    /** @arg n = 0, 高点降序 */
    a.clear();
    for (int i = 0; i < 10; ++i) {
        a.push_back(10-i);
    }

    data = PRICELIST(a);
    result = HHVBARS(data, 0);
    BOOST_CHECK(result.name() == "HHVBARS");
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.discard() == 0);
    for (int i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == i);
    }

    /** @arg n = 0， 顺序随机 */
    result = HHVBARS(c, 0);
    BOOST_CHECK(result.name() == "HHVBARS");
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result[0] == 0);
    BOOST_CHECK(result[1] == 0);
    BOOST_CHECK(result[2] == 1);
    BOOST_CHECK(result[3] == 2);
    BOOST_CHECK(result[4] == 0);
    BOOST_CHECK(result[5] == 1);
    BOOST_CHECK(result[6] == 2);
    BOOST_CHECK(result[9] == 5);

    /** @arg n = 1 */
    result = HHVBARS(c, 1);
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.discard() == 0);
    for (int i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == 0);
    }

    /** @arg n = 3 */
    result = HHVBARS(c, 5);
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.discard() == 0);
    /*for (int i = 0; i < 10; i++) {
        std::cout << i << " " << c[i] << " " << result[i] <<std::endl;
    }*/
    BOOST_CHECK(result[0] == 0);
    BOOST_CHECK(result[1] == 0);
    BOOST_CHECK(result[2] == 1);
    BOOST_CHECK(result[3] == 2);
    BOOST_CHECK(result[4] == 0);
    BOOST_CHECK(result[5] == 1);
    BOOST_CHECK(result[8] == 4);
    BOOST_CHECK(result[9] == 4);

    /** @arg n = 10 */
    result = HHVBARS(c, 10);
    BOOST_CHECK(result.name() == "HHVBARS");
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result[0] == 0);
    BOOST_CHECK(result[1] == 0);
    BOOST_CHECK(result[5] == 1);
    BOOST_CHECK(result[6] == 2);
    BOOST_CHECK(result[9] == 5);
}


//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_HHVBARS_export ) {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/HHVBARS.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = HHVBARS(CLOSE(kdata), 2);
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

    BOOST_CHECK(x1.size() == x2.size());
    BOOST_CHECK(x1.discard() == x2.discard());
    BOOST_CHECK(x1.getResultNumber() == x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        BOOST_CHECK_CLOSE(x1[i], x2[i], 0.00001);
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */


