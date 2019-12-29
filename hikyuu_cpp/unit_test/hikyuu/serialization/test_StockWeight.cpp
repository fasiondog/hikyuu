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
#include <hikyuu/serialization/StockWeight_serialization.h>

using namespace hku;

#if HKU_SUPPORT_SERIALIZATION

/**
 * @defgroup test_StockWeight_serialize test_StockWeight_serialize
 * @ingroup test_hikyuu_serialize_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_StockWeight_serialize") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/StockWeight.xml";

    Stock stock = sm.getStock("sh600000");
    StockWeightList weight_list_1 = stock.getWeight();
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(weight_list_1);
    }

    StockWeightList weight_list_2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(weight_list_2);
    }

    CHECK_EQ(weight_list_1.size(), weight_list_2.size());
    CHECK_NE(weight_list_1.size(), 0);
    size_t total = weight_list_2.size();
    for (size_t i = 0; i < total; ++i) {
        CHECK_EQ(weight_list_1[i].datetime(), weight_list_2[i].datetime());
        CHECK_EQ(weight_list_1[i].countAsGift(), weight_list_2[i].countAsGift());
        CHECK_EQ(weight_list_1[i].countForSell(), weight_list_2[i].countForSell());
        CHECK_EQ(weight_list_1[i].priceForSell(), weight_list_2[i].priceForSell());
        CHECK_EQ(weight_list_1[i].bonus(), weight_list_2[i].bonus());
        CHECK_EQ(weight_list_1[i].increasement(), weight_list_2[i].increasement());
        CHECK_EQ(weight_list_1[i].totalCount(), weight_list_2[i].totalCount());
        CHECK_EQ(weight_list_1[i].freeCount(), weight_list_2[i].freeCount());
    }
}

/** @} */

#endif /* HKU_SUPPORT_SERIALIZATION */
