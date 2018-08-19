/*
 * test_export.cpp
 *
 *  Created on: 2018-2-10
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
#include <hikyuu/trade_sys/selector/crt/SE_Fixed.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>

using namespace hku;

/**
 * @defgroup test_selector_serialization test_selector_serialization
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_SE_FIXED_export ) {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/SE_FIXED.xml";

    SYSPtr sys = SYS_Simple();
    StockList stkList;
    stkList.push_back(sm["sh600000"]);
    stkList.push_back(sm["sz000001"]);

    SEPtr se1 = SE_Fixed(stkList, sys);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(se1);
    }

    SEPtr se2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(se2);
    }

    BOOST_CHECK(se1->name() == se2->name());
}


/** @} */

#endif /* HKU_SUPPORT_SERIALIZATION */




