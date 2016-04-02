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
#include <hikyuu/serialization/KData_serialization.h>

using namespace hku;

#if HKU_SUPPORT_SERIALIZATION

/**
 * @defgroup test_KData_serialize test_KData_serialize
 * @ingroup test_hikyuu_serialize_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_KData_serialize ) {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/KData.xml";

    Stock stock = sm.getStock("sh600000");
    KQuery query(10, 20);
    KData k1 = stock.getKData(query);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(k1);
    }

    KData k2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(k2);
    }

    BOOST_CHECK(k1.size() == k2.size());
    BOOST_CHECK(k1.size() != 0);
    size_t total = k1.size();
    for (size_t i = 0; i < total; ++i) {
        BOOST_CHECK(k1[i] == k2[i]);
    }
}

/** @} */

#endif /* HKU_SUPPORT_SERIALIZATION */



