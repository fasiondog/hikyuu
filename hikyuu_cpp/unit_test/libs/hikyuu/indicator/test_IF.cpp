/*
 * test_IF.cpp
 *
 *  Created on: 2019-3-30
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
#include <hikyuu/indicator/crt/CVAL.h>

using namespace hku;

/**
 * @defgroup test_indicator_IF test_indicator_IF
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_IF ) {
    KData kdata = getStock("SH600000").getKData(KQuery(-10));
    Indicator x = IF(CLOSE() > OPEN(), CVAL(1), CVAL(0));
    x.setContext(kdata);
    Indicator c = CLOSE(kdata);
    Indicator o = OPEN(kdata);
    for (int i = 0; i < x.size(); i++) {
        if (c[i] > o[i]) {
            BOOST_CHECK(x[i] == 1);
        } else {
            BOOST_CHECK(x[i] == 0);
        }
    }
}


//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_IF_export ) {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/IF.xml";

    KData kdata = getStock("SH600000").getKData(KQuery(-10));
    Indicator x1 = IF(CLOSE() > OPEN(), CVAL(1), CVAL(0));
    x1.setContext(kdata);

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

