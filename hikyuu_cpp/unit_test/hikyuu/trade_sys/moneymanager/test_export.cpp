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
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_sys/moneymanager/build_in.h>

using namespace hku;

/**
 * @defgroup test_money_manager_serialization test_money_manager_serialization
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
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

/** @par Test points */
TEST_CASE("test_MM_FixedCountTps_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/FixedCountTps_MM.xml";

    MoneyManagerPtr mm1 = MM_FixedCountTps({100., 200.}, {200., 100.});
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

    /** @arg The tier tables survive the round-trip */
    CHECK_EQ(mm1->name(), mm2->name());
    Stock stock = sm.getStock("sh600000");
    auto tm = crtTM(Datetime(199001010000LL), 1000000.0);
    mm2->setTM(tm);
    CHECK_EQ(mm2->getBuyNumber(Datetime(200001200000), stock, 24.11, 24.11, PART_SIGNAL), 100.0);
    tm->buy(Datetime(200001200000), stock, 24.11, 200, 0, 0, 24.11, PART_SIGNAL);
    CHECK_EQ(mm2->getSellNumber(Datetime(200001200000), stock, 24.11, 24.11, PART_SIGNAL), 200.0);
}

/** @} */

#endif /* HKU_SUPPORT_SERIALIZATION */
