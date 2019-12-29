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
#include <hikyuu/trade_sys/system/build_in.h>

using namespace hku;

/**
 * @defgroup test_system_serialization test_system_serialization
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_TradeRequest_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/TradeRequest.xml";

    TradeRequest tr1;
    tr1.valid = 1;
    tr1.business = BUSINESS_BUY;
    tr1.datetime = Datetime(201001100000);
    tr1.stoploss = 11.13;
    tr1.number = 1000;
    tr1.from = PART_SIGNAL;
    tr1.count = 1;
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(tr1);
    }

    TradeRequest tr2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(tr2);
    }

    CHECK_EQ(tr1.valid, tr2.valid);
    CHECK_EQ(tr1.business, tr2.business);
    CHECK_EQ(tr1.datetime, tr2.datetime);
    CHECK_EQ(tr1.stoploss, tr2.stoploss);
    CHECK_EQ(tr1.number, tr2.number);
    CHECK_EQ(tr1.from, tr2.from);
    CHECK_EQ(tr1.count, tr2.count);
}

/** @par 检测点 */
TEST_CASE("test_SYS_Simple_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/SYS_Simple.xml";

    SystemPtr sg1 = SYS_Simple();
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(sg1);
    }

    SystemPtr sg2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(sg2);
    }

    CHECK_EQ(sg1->name(), sg2->name());
}

/** @} */

#endif /* HKU_SUPPORT_SERIALIZATION */
