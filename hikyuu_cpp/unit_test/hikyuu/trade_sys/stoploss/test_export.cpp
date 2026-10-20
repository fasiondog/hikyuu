/*
 * test_export.cpp
 *
 *  Created on: 2013-4-30
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/config.h>

#if HKU_SUPPORT_SERIALIZATION

#include <cmath>
#include <fstream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/trade_sys/stoploss/build_in.h>

using namespace hku;

/**
 * @defgroup test_stoploss_serialization test_stoploss_serialization
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_ST_FixedPercent_export") {
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

    CHECK_EQ(sg1->name(), sg2->name());

    /** @arg the p parameter survives the round-trip */
    CHECK_EQ(sg2->getParam<double>("p"), doctest::Approx(0.03));

    /** @arg the loaded instance computes the same level as the original one */
    CHECK_EQ(sg2->getPrice(Datetime(200101010000), 10.0),
             doctest::Approx(sg1->getPrice(Datetime(200101010000), 10.0)));
}

/** @par Test points */
TEST_CASE("test_ST_Indicator_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/ST_Indicator.xml";

    StoplossPtr sg1 = ST_Indicator(CLOSE());
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

    CHECK_EQ(sg1->name(), sg2->name());

    /** @arg the wrapped indicator survives the round-trip: after setTO both instances return the
     * same level */
    KData kdata = sm.getStock("sz000001").getKData(KQuery(0, 100));
    sg1->setTO(kdata);
    sg2->setTO(kdata);
    const Datetime date = kdata.back().datetime;
    price_t level1 = sg1->getPrice(date, 10.0);
    CHECK_UNARY(std::isfinite(level1));
    CHECK_EQ(sg2->getPrice(date, 10.0), doctest::Approx(level1));
}

/** @par Test points */
TEST_CASE("test_ST_Saftyloss_export") {
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

    CHECK_EQ(sg1->name(), sg2->name());

    /** @arg the wrapped SAFTYLOSS formula survives the round-trip: after setTO both instances
     * return the same level */
    KData kdata = sm.getStock("sz000001").getKData(KQuery(0, 100));
    sg1->setTO(kdata);
    sg2->setTO(kdata);
    // The first n1 + n2 - 2 bars are the warm-up of SAFTYLOSS, take a later bar
    const Datetime date = kdata.back().datetime;
    price_t level1 = sg1->getPrice(date, 10.0);
    CHECK_UNARY(std::isfinite(level1));
    CHECK_EQ(sg2->getPrice(date, 10.0), doctest::Approx(level1));
}

/** @} */

#endif /* HKU_SUPPORT_SERIALIZATION */
