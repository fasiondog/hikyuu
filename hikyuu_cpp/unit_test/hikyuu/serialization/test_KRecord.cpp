/*
 * test_KRecord.cpp
 *
 *  Created on: 2013-5-1
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
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
TEST_CASE("test_KRecord_serialize") {
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

    CHECK_EQ(k1.datetime, k2.datetime);
    CHECK_EQ(k1.openPrice, k2.openPrice);
    CHECK_EQ(k1.highPrice, k2.highPrice);
    CHECK_EQ(k1.lowPrice, k2.lowPrice);
    CHECK_EQ(k1.closePrice, k2.closePrice);
    CHECK_EQ(k1.datetime, k2.datetime);
    CHECK_EQ(k1.datetime, k2.datetime);
}

/** @} */

#endif /* HKU_SUPPORT_SERIALIZATION */
