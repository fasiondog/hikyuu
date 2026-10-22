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

/** @par Test points */
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

/** @par Test points
 * Regression lock for the cycle-date serialization gap: a standalone SG_Cycle must survive
 * save -> load -> setTO without losing its declared rebalancing days.
 */
TEST_CASE("test_SG_Cycle_export") {
    StockManager& sm = StockManager::instance();
    KData k = sm.getStock("sh000001").getKData(KQueryByIndex(-30));
    REQUIRE(k.size() >= 21);

    SignalPtr sg1 = SG_Cycle();
    DatetimeList dates = {k[0].datetime, k[10].datetime, k[20].datetime};
    sg1->setCycleDates(dates);
    sg1->setTO(k);
    REQUIRE(sg1->getBuySignal().size() == 3);

    string filename(sm.tmpdir());
    filename += "/SG_Cycle.xml";
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

    /** @arg declared rebalancing days survive the load */
    CHECK_UNARY(sg2->getCycleDates() == dates);
    /** @arg signals loaded from the archive are visible before any recalculation */
    CHECK_EQ(sg2->getBuySignal().size(), 3u);
    /** @arg after setTO the cycle dates drive recalculation, so the signals are not silently lost
     */
    sg2->setTO(k);
    CHECK_EQ(sg2->getBuySignal().size(), 3u);
    CHECK_UNARY(sg2->getBuySignal() == sg1->getBuySignal());
}

/** @} */

#endif /* HKU_SUPPORT_SERIALIZATION */
