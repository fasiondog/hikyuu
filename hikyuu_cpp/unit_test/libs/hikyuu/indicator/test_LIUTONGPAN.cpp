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
#include <hikyuu/indicator/crt/LIUTONGPAN.h>

using namespace hku;

/**
 * @defgroup test_indicator_LIUTONGPAN test_indicator_LIUTONGPAN
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_LIUTONGPAN ) {
    StockManager& sm = StockManager::instance();
    
    Stock stk = getStock("SH600004");
    KQuery query = KQueryByDate(Datetime(200301010000), Datetime(200708250000));
    KData k = stk.getKData(query);

    Indicator liutong = LIUTONGPAN(k);
    BOOST_CHECK(liutong.size() == k.size());
    size_t total = k.size();
    for (int i = 0; i < total; i++) {
        if (k[i].datetime < Datetime(200512200000)) {
            BOOST_CHECK(liutong[i] == 40000);
        } else if (k[i].datetime >= Datetime(200512200000) 
                && k[i].datetime < Datetime(200612200000)) {
            BOOST_CHECK(liutong[i] == 47600);
        } else {
            BOOST_CHECK(liutong[i] == 49696);
        }
    }
}


//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_LIUTONGPAN_export ) {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/IF.xml";

    KData k = getStock("SH600000").getKData(KQuery(-10));
    Indicator x1 = LIUTONGPAN(k);
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

    BOOST_CHECK(x1.size() == x2.size());
    BOOST_CHECK(x1.discard() == x2.discard());
    BOOST_CHECK(x1.getResultNumber() == x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        BOOST_CHECK_CLOSE(x1[i], x2[i], 0.00001);
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */

