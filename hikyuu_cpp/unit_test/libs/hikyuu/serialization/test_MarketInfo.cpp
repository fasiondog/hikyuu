/*
 * test_KRecord.cpp
 *
 *  Created on: 2013-5-1
 *      Author: fasiondog
 */

#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_serialize_suite
    #include <boost/test/unit_test.hpp>
#endif

#include <fstream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <hikyuu/StockManager.h>
#include <hikyuu/serialization/MarketInfo_serialization.h>

using namespace hku;

#if HKU_SUPPORT_SERIALIZATION

/**
 * @defgroup test_MarketInfo_serialize test_MarketInfo_serialize
 * @ingroup test_hikyuu_serialize_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_MarketInfo_serialize ) {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/MarketInfo.xml";

    MarketInfo m1 = sm.getMarketInfo("SH");
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(m1);
    }

    MarketInfo m2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(m2);
    }

    BOOST_CHECK(m1.market() == m2.market());
    BOOST_CHECK(m1.name() == m2.name());
    BOOST_CHECK(m1.description() == m2.description());
    BOOST_CHECK(m1.code() == m2.code());
    BOOST_CHECK(m1.lastDate() == m2.lastDate());

}

/** @} */

#endif /* HKU_SUPPORT_SERIALIZATION */



