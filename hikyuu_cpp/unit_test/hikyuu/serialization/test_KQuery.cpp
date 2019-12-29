/*
 * test_KQuery.cpp
 *
 *  Created on: 2013-5-1
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
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
TEST_CASE("test_KQuery_serialize") {
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

    CHECK_EQ(q1.queryType(), q1.queryType());
    CHECK_EQ(q1.kType(), q1.kType());
    CHECK_EQ(q1.recoverType(), q1.recoverType());
    CHECK_EQ(q1.start(), q1.start());
    CHECK_EQ(q1.end(), q1.end());
}

/** @par 检测点 */
TEST_CASE("test_KQueryByDate_serialize") {
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

    CHECK_EQ(q1.queryType(), q1.queryType());
    CHECK_EQ(q1.kType(), q1.kType());
    CHECK_EQ(q1.recoverType(), q1.recoverType());
    CHECK_EQ(q1.startDatetime(), q1.startDatetime());
    CHECK_EQ(q1.endDatetime(), q1.endDatetime());

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

    CHECK_EQ(q3.queryType(), q4.queryType());
    CHECK_EQ(q3.kType(), q4.kType());
    CHECK_EQ(q3.recoverType(), q4.recoverType());
    CHECK_EQ(q3.startDatetime(), q4.startDatetime());
    CHECK_EQ(q3.endDatetime(), q4.endDatetime());
}
/** @} */

#endif /* HKU_SUPPORT_SERIALIZATION */
