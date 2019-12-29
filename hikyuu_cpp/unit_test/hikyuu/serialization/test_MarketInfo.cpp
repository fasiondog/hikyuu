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
#include <hikyuu/serialization/MarketInfo_serialization.h>

using namespace hku;

#if HKU_SUPPORT_SERIALIZATION

/**
 * @defgroup test_MarketInfo_serialize test_MarketInfo_serialize
 * @ingroup test_hikyuu_serialize_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_MarketInfo_serialize") {
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

    CHECK_EQ(m1.market(), m2.market());
    CHECK_EQ(m1.name(), m2.name());
    CHECK_EQ(m1.description(), m2.description());
    CHECK_EQ(m1.code(), m2.code());
    CHECK_EQ(m1.lastDate(), m2.lastDate());
}

/** @} */

#endif /* HKU_SUPPORT_SERIALIZATION */
