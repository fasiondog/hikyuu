/*
 * test_KQuery.cpp
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
#include <hikyuu/serialization/KQuery_serialization.h>

using namespace hku;

#if HKU_SUPPORT_SERIALIZATION

/**
 * @defgroup test_KQuery_serialize test_KQuery_serialize
 * @ingroup test_hikyuu_serialize_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_KQuery_serialize ) {
    string filename(StockManager::instance().tmpdir());
    filename += "/KQuery.xml";

    KQuery q1(10, 100);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(q1);
    }

    KQuery q2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(q2);
    }

    BOOST_CHECK(q1.queryType() == q1.queryType());
    BOOST_CHECK(q1.kType() == q1.kType());
    BOOST_CHECK(q1.recoverType() == q1.recoverType());
    BOOST_CHECK(q1.start() == q1.start());
    BOOST_CHECK(q1.end() == q1.end());
}


/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_KQueryByDate_serialize ) {
    string filename(StockManager::instance().tmpdir());
    filename += "/KQueryByDate.xml";

    /** KQueryByDate保存，KQuery读取 */
    KQuery q1 = KQueryByDate(Datetime(200101010000), Datetime(200102100000));
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(q1);
    }

    KQuery q2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(q2);
    }

    BOOST_CHECK(q1.queryType() == q1.queryType());
    BOOST_CHECK(q1.kType() == q1.kType());
    BOOST_CHECK(q1.recoverType() == q1.recoverType());
    BOOST_CHECK(q1.startDatetime() == q1.startDatetime());
    BOOST_CHECK(q1.endDatetime() == q1.endDatetime());

    /* KQueryByDate读取 */
    KQuery q3 = KQueryByDate(Datetime(200101010000), Datetime(200102100000));
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(q3);
    }

    KQuery q4;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(q4);
    }

    BOOST_CHECK(q3.queryType() == q4.queryType());
    BOOST_CHECK(q3.kType() == q4.kType());
    BOOST_CHECK(q3.recoverType() == q4.recoverType());
    BOOST_CHECK(q3.startDatetime() == q4.startDatetime());
    BOOST_CHECK(q3.endDatetime() == q4.endDatetime());
}
/** @} */

#endif /* HKU_SUPPORT_SERIALIZATION */
