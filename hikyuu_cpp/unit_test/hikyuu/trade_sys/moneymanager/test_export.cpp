/*
 * test_export.cpp
 *
 *  Created on: 2013-4-30
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/config.h>

#if HKU_SUPPORT_SERIALIZATION

#include <fstream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/moneymanager/build_in.h>

using namespace hku;

/**
 * @defgroup test_money_manager_serialization test_money_manager_serialization
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_MM_FixedCount_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/Fixed_MM.xml";

    MoneyManagerPtr mm1 = MM_FixedCount(100);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(mm1);
    }

    MoneyManagerPtr mm2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(mm2);
    }

    CHECK_EQ(mm1->name(), mm2->name());
}

/** @} */

#endif /* HKU_SUPPORT_SERIALIZATION */
