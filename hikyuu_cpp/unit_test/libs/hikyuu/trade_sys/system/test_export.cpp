/*
 * test_export.cpp
 *
 *  Created on: 2013-4-30
 *      Author: fasiondog
 */

#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_trade_sys_suite
    #include <boost/test/unit_test.hpp>
#endif

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
BOOST_AUTO_TEST_CASE( test_TradeRequest_export ) {
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

    BOOST_CHECK(tr1.valid == tr2.valid);
    BOOST_CHECK(tr1.business == tr2.business);
    BOOST_CHECK(tr1.datetime == tr2.datetime);
    BOOST_CHECK(tr1.stoploss == tr2.stoploss);
    BOOST_CHECK(tr1.number == tr2.number);
    BOOST_CHECK(tr1.from == tr2.from);
    BOOST_CHECK(tr1.count == tr2.count);
}


/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_SYS_Simple_export ) {
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

    BOOST_CHECK(sg1->name() == sg2->name());
}

/** @} */

#endif /* HKU_SUPPORT_SERIALIZATION */




