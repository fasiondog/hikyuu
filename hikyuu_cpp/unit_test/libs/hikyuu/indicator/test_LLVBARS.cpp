/*
 * test_LLVBARS.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 * 
 *  Created on: 2019-4-14
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
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/LLVBARS.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_LLVBARS test_indicator_LLVBARS
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_LLVBARS ) {
    StockManager& sm = StockManager::instance();
    Stock stk = sm.getStock("sh000001");
    KData k = stk.getKData(-10);
    Indicator c = CLOSE(k);

    Indicator result;

    /** @arg n = 0, 低点顺序下降 */
    PriceList a;
    for (int i = 0; i < 10; ++i) {
        a.push_back(10-i);
    }

    Indicator data = PRICELIST(a);
    result = LLVBARS(data, 0);
    BOOST_CHECK(result.name() == "LLVBARS");
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.discard() == 0);
    for (int i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == 0);
    }

    /** @arg n = 0, 低点升序 */
    a.clear();
    for (int i = 0; i < 10; ++i) {
        a.push_back(i);
    }

    data = PRICELIST(a);
    result = LLVBARS(data, 0);
    BOOST_CHECK(result.name() == "LLVBARS");
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.discard() == 0);
    for (int i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == i);
    }

    /** @arg n = 0， 顺序随机 */
    result = LLVBARS(c, 0);
    BOOST_CHECK(result.name() == "LLVBARS");
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result[0] == 0);
    BOOST_CHECK(result[1] == 1);
    BOOST_CHECK(result[2] == 0);
    BOOST_CHECK(result[3] == 1);
    BOOST_CHECK(result[4] == 2);
    BOOST_CHECK(result[5] == 0);
    BOOST_CHECK(result[6] == 1);
    BOOST_CHECK(result[9] == 0);

    /** @arg n = 1 */
    result = LLVBARS(c, 1);
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.discard() == 0);
    for (int i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == 0);
    }

    /** @arg n = 5 */
    result = LLVBARS(c, 5);
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result[0] == 0);
    BOOST_CHECK(result[1] == 1);
    BOOST_CHECK(result[2] == 0);
    BOOST_CHECK(result[3] == 1);
    BOOST_CHECK(result[4] == 2);
    BOOST_CHECK(result[5] == 0);
    BOOST_CHECK(result[8] == 0);
    BOOST_CHECK(result[9] == 0);

    /** @arg n = 10 */
    result = LLVBARS(c, 10);
    BOOST_CHECK(result.name() == "LLVBARS");
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result[0] == 0);
    BOOST_CHECK(result[1] == 1);
    BOOST_CHECK(result[5] == 0);
    BOOST_CHECK(result[6] == 1);
    BOOST_CHECK(result[7] == 2);
    BOOST_CHECK(result[9] == 0);

    /** @arg ind.size() == 1 */
    result = LLVBARS(CVAL(1));
    BOOST_CHECK(result.size() == 1);
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result[0] == 0);

    /** @arg n > ind.size() */
    result = LLVBARS(c, 20);
    BOOST_CHECK(20 > c.size());
    BOOST_CHECK(result.name() == "LLVBARS");
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result[0] == 0);
    BOOST_CHECK(result[1] == 1);
    BOOST_CHECK(result[2] == 0);
    BOOST_CHECK(result[3] == 1);
    BOOST_CHECK(result[4] == 2);
    BOOST_CHECK(result[5] == 0);
    BOOST_CHECK(result[6] == 1);
    BOOST_CHECK(result[9] == 0);
}


//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_LLVBARS_export ) {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/LLVBARS.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = LLVBARS(CLOSE(kdata), 2);
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

    BOOST_CHECK(x2.name() == "LLVBARS");
    BOOST_CHECK(x1.size() == x2.size());
    BOOST_CHECK(x1.discard() == x2.discard());
    BOOST_CHECK(x1.getResultNumber() == x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        BOOST_CHECK_CLOSE(x1[i], x2[i], 0.00001);
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */


