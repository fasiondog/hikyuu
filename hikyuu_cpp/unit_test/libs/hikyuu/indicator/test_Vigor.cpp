/*
 * test_Vigor.cpp
 *
 *  Created on: 2013-4-12
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
#include <hikyuu/indicator/crt/VIGOR.h>

using namespace hku;

/**
 * @defgroup test_indicator_VIGOR test_indicator_VIGOR
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_VIGOR ) {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData kdata;
    Indicator vigor;

    /** @arg kdata为空时 */
    vigor = VIGOR(kdata, 1);
    BOOST_CHECK(vigor.size() == 0);
    BOOST_CHECK(vigor.empty() == true);

    /** @arg 正常情况 */
    KQuery query = KQuery(0, 10);
    kdata = stock.getKData(query);
    BOOST_CHECK(kdata.size() == 10);
    vigor = VIGOR(kdata, 1);
    BOOST_CHECK(vigor.discard() == 1);
    BOOST_CHECK(vigor.size()== 10);
    BOOST_CHECK(vigor.empty() == false);
    BOOST_CHECK(std::isnan(vigor[0]));
    BOOST_CHECK(std::fabs(vigor[1] + 11761.36) < 0.0001 );

    vigor = VIGOR(kdata, 2);
    BOOST_CHECK(vigor.discard() == 1);
}


//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_VIGOR_export ) {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/VIGOR.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = VIGOR(kdata, 10);
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


