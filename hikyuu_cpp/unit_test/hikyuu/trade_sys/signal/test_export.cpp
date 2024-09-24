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
#include <hikyuu/trade_sys/signal/build_in.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/AMA.h>

using namespace hku;

/**
 * @defgroup test_signal_serialization test_signal_serialization
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_SG_AMA_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/SG_AMA.xml";

    SignalPtr sg1 = SG_Single(AMA(CLOSE()));
    auto k = getKData("sz000001", KQueryByIndex(-100));
    sg1->setTO(k);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(sg1);
    }

    SignalPtr sg2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(sg2);
    }

    CHECK_EQ(sg1->name(), sg2->name());
    CHECK_UNARY(sg2->getTO().empty());
}

/** @} */

#endif /* HKU_SUPPORT_SERIALIZATION */
