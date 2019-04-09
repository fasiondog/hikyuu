/*
 * test_MIN.cpp
 *
 *  Created on: 2019-4-9
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
#include <hikyuu/indicator/crt/MIN.h>
#include <hikyuu/indicator/crt/REF.h>

using namespace hku;

/**
 * @defgroup test_indicator_MIN test_indicator_MIN
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_MIN ) {
    /** @arg n > 0 */
    KData k =getStock("sh600004").getKData(KQuery(-8));
    Indicator C = CLOSE(k);
    Indicator O = OPEN(k);
    Indicator x = MIN(C, O);

    BOOST_CHECK(x.name() == "MIN");
    BOOST_CHECK(x.size() == 8);
    BOOST_CHECK(x.discard() == 0);
    size_t total = x.size();
    for (int i = 0; i < total; i++) {
        if (C[i] < O[i]) {
            BOOST_CHECK(x[i] == C[i]);
        } else {
            BOOST_CHECK(x[i] == O[i]);
        }
    }
}


//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_MIN_export ) {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/MIN.xml";

    Indicator x1 = MIN(CLOSE(), OPEN());
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

