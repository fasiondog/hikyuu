/*
 * test_TransRecord.cpp
 *
 *  Created on: 2019-2-11
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <hikyuu/StockManager.h>
#include <hikyuu/serialization/TransRecord_serialization.h>

using namespace hku;

#if HKU_SUPPORT_SERIALIZATION

/**
 * @defgroup test_Trans_serialize test_Trans_serialize
 * @ingroup test_hikyuu_serialize_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_TransRecord_serialize") {
    string filename(StockManager::instance().tmpdir());
    filename += "/TransRecord.xml";

    TransRecord t1(Datetime(201101010000), 10.0, 80, 1);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(t1);
    }

    TransRecord t2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(t2);
    }

    CHECK_EQ(t1, t2);
}

/** @par 检测点 */
TEST_CASE("test_TransList_serialize") {
    string filename(StockManager::instance().tmpdir());
    filename += "/TransList.xml";

    TransList line1;
    line1.push_back(TransRecord(Datetime(201101010000), 10.0, 80, 0));
    line1.push_back(TransRecord(Datetime(201101020000), 20.0, 90, 1));
    line1.push_back(TransRecord(Datetime(201101030000), 20.0, 100, 2));
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(line1);
    }

    TransList line2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(line2);
    }

    CHECK_NE(line1.size(), 0);
    CHECK_EQ(line1.size(), line2.size());
    size_t total = line1.size();
    for (size_t i = 0; i < total; i++) {
        CHECK_EQ(line1[i], line2[i]);
    }
}

/** @} */

#endif /* HKU_SUPPORT_SERIALIZATION */
