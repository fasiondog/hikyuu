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
#include <hikyuu/serialization/StockTypeInfo_serialization.h>

using namespace hku;

#if HKU_SUPPORT_SERIALIZATION

/**
 * @defgroup test_StockTypeInfo_serialize test_StockTypeInfo_serialize
 * @ingroup test_hikyuu_serialize_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_StockTypeInfo_serialize") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/StockTypeInfo.xml";

    StockTypeInfo m1 = sm.getStockTypeInfo(1);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(m1);
    }

    StockTypeInfo m2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(m2);
    }

    CHECK_EQ(m1.type(), m2.type());
    CHECK_EQ(m1.description(), m2.description());
    CHECK_EQ(m1.tick(), m2.tick());
    CHECK_EQ(m1.tickValue(), m2.tickValue());
    CHECK_EQ(m1.unit(), m2.unit());
    CHECK_EQ(m1.precision(), m2.precision());
    CHECK_EQ(m1.minTradeNumber(), m2.minTradeNumber());
    CHECK_EQ(m1.maxTradeNumber(), m2.maxTradeNumber());
}

/** @} */

#endif /* HKU_SUPPORT_SERIALIZATION */
