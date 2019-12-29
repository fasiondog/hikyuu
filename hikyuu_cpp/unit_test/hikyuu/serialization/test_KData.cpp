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
#include <hikyuu/serialization/KData_serialization.h>

using namespace hku;

#if HKU_SUPPORT_SERIALIZATION

/**
 * @defgroup test_KData_serialize test_KData_serialize
 * @ingroup test_hikyuu_serialize_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_KData_serialize") {
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

    CHECK_EQ(k1.size(), k2.size());
    CHECK_NE(k1.size(), 0);
    size_t total = k1.size();
    for (size_t i = 0; i < total; ++i) {
        CHECK_EQ(k1[i], k2[i]);
    }
}

/** @} */

#endif /* HKU_SUPPORT_SERIALIZATION */
