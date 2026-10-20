/*
 * test_export.cpp
 *
 *  Created on: 2026-10-20
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/config.h>

#if HKU_SUPPORT_SERIALIZATION

#include <fstream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/profitgoal/build_in.h>

using namespace hku;

/**
 * @defgroup test_profitgoal_serialization test_profitgoal_serialization
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @arg PG_FixedPercent keeps name and the p parameter across a round trip */
TEST_CASE("test_PG_FixedPercent_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/PG_FixedPercent.xml";

    PGPtr pg1 = PG_FixedPercent(0.03);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(pg1);
    }

    PGPtr pg2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(pg2);
    }

    CHECK_EQ(pg1->name(), pg2->name());
    CHECK_EQ(pg2->getParam<double>("p"), 0.03);
}

/** @arg PG_FixedHoldDays keeps name and the days parameter across a round trip */
TEST_CASE("test_PG_FixedHoldDays_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/PG_FixedHoldDays.xml";

    PGPtr pg1 = PG_FixedHoldDays(8);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(pg1);
    }

    PGPtr pg2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(pg2);
    }

    CHECK_EQ(pg1->name(), pg2->name());
    CHECK_EQ(pg2->getParam<int>("days"), 8);
}

/** @arg PG_NoGoal keeps name across a round trip */
TEST_CASE("test_PG_NoGoal_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/PG_NoGoal.xml";

    PGPtr pg1 = PG_NoGoal();
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(pg1);
    }

    PGPtr pg2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(pg2);
    }

    CHECK_EQ(pg1->name(), pg2->name());
}

/** @} */

#endif /* HKU_SUPPORT_SERIALIZATION */
