/*
 * test_LIUTONGPAN.cpp
 *
 *  Created on: 2019-3-29
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
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/COUNT.h>
#include <hikyuu/indicator/crt/REF.h>

using namespace hku;

/**
 * @defgroup test_indicator_COUNT test_indicator_COUNT
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_COUNT ) {
    Indicator C = CLOSE();
    Indicator x = COUNT(C > REF(C, 1), 5);
    x.setContext(getStock("sh600004"), KQuery(-8));
    BOOST_CHECK(x.size() == 8);
    BOOST_CHECK(x.discard() == 5);
    BOOST_CHECK(x[5] == 3);
    BOOST_CHECK(x[6] == 2);
    BOOST_CHECK(x[7] == 2);
    /*C.setContext(getStock("sh600004"), KQuery(-8));
    for (int i = 1; i < 8 ; i++) {
        std::cout << i << " " << C[i-1] << " " << C[i] << " " << x[i] << std::endl;
    }*/
}


//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_COUNT_export ) {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/COUNT.xml";

    Indicator C = CLOSE();
    Indicator x1 = COUNT(C > REF(C, 1), 5);
    x1.setContext(getStock("sh600004"), KQuery(-8));

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

