/*
 * test_TimeLine.cpp
 *
 *  Created on: 2019年2月10日
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
#include <hikyuu/serialization/TimeLineRecord_serialization.h>

using namespace hku;

#if HKU_SUPPORT_SERIALIZATION

/**
 * @defgroup test_TimeLine_serialize test_TimeLine_serialize
 * @ingroup test_hikyuu_serialize_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_TimeLineRecord_serialize ) {
    string filename(StockManager::instance().tmpdir());
    filename += "/TimeLineRecord.xml";

    TimeLineRecord t1(Datetime(201101010000), 10.0, 80);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(t1);
    }

    TimeLineRecord t2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(t2);
    }

    BOOST_CHECK(t1.datetime == t2.datetime);
    BOOST_CHECK(t1.price == t2.price);
    BOOST_CHECK(t1.vol == t2.vol);
}


/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_TimeLine_serialize ) {
    string filename(StockManager::instance().tmpdir());
    filename += "/TimeLine.xml";

    TimeLineList line1;
    line1.push_back(TimeLineRecord(Datetime(201101010000), 10.0, 80));
    line1.push_back(TimeLineRecord(Datetime(201101020000), 20.0, 90));
    line1.push_back(TimeLineRecord(Datetime(201101030000), 20.0, 100));
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(line1);
    }

    TimeLineList line2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(line2);
    }

    BOOST_CHECK(line1.size() != 0);
    BOOST_CHECK(line1.size() == line2.size());
    size_t total = line1.size();
    for (size_t i = 0; i < total; i++) {
        BOOST_CHECK(line1[i] == line2[i]);
    }
}


/** @} */

#endif /* HKU_SUPPORT_SERIALIZATION */


