/*
 * test_AD.cpp
 *
 *  Created on: 2019-5-18
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
#include <hikyuu/indicator/crt/AD.h>

using namespace hku;

/**
 * @defgroup test_indicator_AD test_indicator_AD
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_AD ) {
    StockManager& sm = StockManager::instance();
    
    Stock stk = getStock("SH600004");
    KQuery query = KQueryByIndex(-10);
    KData k = stk.getKData(query);

    Indicator ad = AD(k);
    BOOST_CHECK(ad.name() == "AD");
    BOOST_CHECK(ad.size() == k.size());
    BOOST_CHECK(ad.discard() == 0);
    BOOST_CHECK(ad[0] == 0);
    BOOST_CHECK_CLOSE(ad[1], 458.65, 0.1);
    BOOST_CHECK_CLOSE(ad[2], 63.99, 0.1);
    BOOST_CHECK_CLOSE(ad[5], 30.77, 0.1);
}


//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_AD_export ) {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/AD.xml";

    KData k = getStock("SH600000").getKData(KQuery(-10));
    Indicator x1 = AD(k);
    x1.setContext(k);

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

    BOOST_CHECK(x2.name() == "AD");
    BOOST_CHECK(x1.size() == x2.size());
    BOOST_CHECK(x1.discard() == x2.discard());
    BOOST_CHECK(x1.getResultNumber() == x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        BOOST_CHECK_CLOSE(x1[i], x2[i], 0.00001);
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */

