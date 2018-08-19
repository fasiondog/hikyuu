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
#include <hikyuu/trade_sys/stoploss/build_in.h>

using namespace hku;

/**
 * @defgroup test_stoploss_serialization test_stoploss_serialization
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_ST_FixedPercent_export ) {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/ST_FixedPercent.xml";

    StoplossPtr sg1 = ST_FixedPercent(0.03);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(sg1);
    }

    StoplossPtr sg2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(sg2);
    }

    BOOST_CHECK(sg1->name() == sg2->name());
}


/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_ST_Saftyloss_export ) {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/ST_Saftyloss.xml";

    StoplossPtr sg1 = ST_Saftyloss();
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(sg1);
    }

    StoplossPtr sg2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(sg2);
    }

    BOOST_CHECK(sg1->name() == sg2->name());
}


/** @} */

#endif /* HKU_SUPPORT_SERIALIZATION */




