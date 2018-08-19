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
#include <hikyuu/serialization/StockTypeInfo_serialization.h>

using namespace hku;

#if HKU_SUPPORT_SERIALIZATION

/**
 * @defgroup test_StockTypeInfo_serialize test_StockTypeInfo_serialize
 * @ingroup test_hikyuu_serialize_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_StockTypeInfo_serialize ) {
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

    BOOST_CHECK(m1.type() == m2.type());
    BOOST_CHECK(m1.description() == m2.description());
    BOOST_CHECK(m1.tick() == m2.tick());
    BOOST_CHECK(m1.tickValue() == m2.tickValue());
    BOOST_CHECK(m1.unit() == m2.unit());
    BOOST_CHECK(m1.precision() == m2.precision());
    BOOST_CHECK(m1.minTradeNumber() == m2.minTradeNumber());
    BOOST_CHECK(m1.maxTradeNumber() == m2.maxTradeNumber());
}

/** @} */

#endif /* HKU_SUPPORT_SERIALIZATION */



