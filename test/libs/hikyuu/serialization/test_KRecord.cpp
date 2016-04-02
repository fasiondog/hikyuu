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
#include <hikyuu/serialization/KRecord_serialization.h>

using namespace hku;

#if HKU_SUPPORT_SERIALIZATION

/**
 * @defgroup test_KRecord_serialize test_KRecord_serialize
 * @ingroup test_hikyuu_serialize_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_KRecord_serialize ) {
    string filename(StockManager::instance().tmpdir());
    filename += "/KRecord.xml";

    KRecord k1(Datetime(201101010000), 10.0, 13.3, 9.7, 12.01, 405.6, 80);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(k1);
    }

    KRecord k2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(k2);
    }

    BOOST_CHECK(k1.datetime == k2.datetime);
    BOOST_CHECK(k1.openPrice == k2.openPrice);
    BOOST_CHECK(k1.highPrice == k2.highPrice);
    BOOST_CHECK(k1.lowPrice == k2.lowPrice);
    BOOST_CHECK(k1.closePrice == k2.closePrice);
    BOOST_CHECK(k1.datetime == k2.datetime);
    BOOST_CHECK(k1.datetime == k2.datetime);
}

/** @} */

#endif /* HKU_SUPPORT_SERIALIZATION */



