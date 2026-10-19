/*
 * test_export.cpp
 *
 * Serialization round-trip tests for the environment component.
 */

#include "doctest/doctest.h"
#include <hikyuu/config.h>

#if HKU_SUPPORT_SERIALIZATION

#include <fstream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/environment/build_in.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_Environment_serialization test_Environment_serialization
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

namespace {

// Write the environment to the XML archive and read it back
EnvironmentPtr roundTrip(EnvironmentPtr ev, const string& filename) {
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(ev);
    }

    EnvironmentPtr result;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(result);
    }
    return result;
}

}  // namespace

/** @par Test points */
TEST_CASE("test_EV_Bool_export") {
    StockManager& sm = StockManager::instance();
    KQuery query(-100);
    KData kdata = sm.getStock("sh000001").getKData(query);
    REQUIRE(!kdata.empty());

    EVPtr ev1 = EV_Bool(PRICELIST(kdata.size(), 1.0));
    ev1->setQuery(query);
    EVPtr ev2 = roundTrip(ev1, sm.tmpdir() + "/EV_Bool.xml");

    /** @arg the name and every parameter survive the round-trip */
    CHECK_EQ(ev1->name(), ev2->name());
    CHECK_EQ(ev1->getParameter(), ev2->getParameter());

    /** @arg the computed valid dates survive the round-trip and need no recalculation */
    CHECK(ev2->isValid(kdata.front().datetime));
    CHECK(ev2->isValid(kdata.back().datetime));
    CHECK_EQ(ev2->getValues().size(), kdata.size());
}

/** @par Test points */
TEST_CASE("test_EV_Manual_export") {
    StockManager& sm = StockManager::instance();
    KQuery query(-100);
    KData kdata = sm.getStock("sh000001").getKData(query);
    REQUIRE(kdata.size() > 2);

    EVPtr ev1 = EV_Manual();
    ev1->setQuery(query);
    ev1->_addValid(kdata[0].datetime, 2.0);
    ev1->_addValid(kdata[1].datetime, -1.0);
    EVPtr ev2 = roundTrip(ev1, sm.tmpdir() + "/EV_Manual.xml");

    /** @arg the manually registered dates and their values survive the round-trip */
    CHECK_EQ(ev1->name(), ev2->name());
    CHECK(ev2->isValid(kdata[0].datetime));
    CHECK_FALSE(ev2->isValid(kdata[1].datetime));
    CHECK_EQ(ev2->getValue(kdata[0].datetime), 2.0);
    CHECK_EQ(ev2->getValue(kdata[1].datetime), -1.0);
    CHECK_FALSE(ev2->isValid(kdata[2].datetime));
}

/** @par Test points */
TEST_CASE("test_EV_Multi_export") {
    StockManager& sm = StockManager::instance();
    KQuery query(-100);
    KQuery query2(-90);
    KData kdata = sm.getStock("sh000001").getKData(query);
    KData kdata2 = sm.getStock("sh000001").getKData(query2);
    REQUIRE(!kdata.empty());
    REQUIRE(!kdata2.empty());

    EVPtr ev1 = EV_Bool(PRICELIST(kdata.size(), 2.0));
    EVPtr ev2 = EV_Bool(PRICELIST(kdata.size(), 3.0));
    EVPtr combo1 = ev1 * ev2;
    combo1->setQuery(query);
    EVPtr combo2 = roundTrip(combo1, sm.tmpdir() + "/EV_Multi.xml");

    /** @arg the computed result survives the round-trip; EV_Bool marks its dates with the
     *       default value 1.0, so the Multi product is 1.0 too */
    CHECK_EQ(combo1->name(), combo2->name());
    CHECK(combo2->isValid(kdata.front().datetime));
    CHECK(combo2->isValid(kdata.back().datetime));
    CHECK_EQ(combo2->getValue(kdata.front().datetime), 1.0);
    CHECK_EQ(combo2->getValues().size(), kdata.size());

    /** @arg the serialized sides are intact: a new query recalculates correctly */
    combo2->setQuery(query2);
    CHECK_EQ(combo2->getValues().size(), kdata2.size());
}

/** @} */

#endif /* HKU_SUPPORT_SERIALIZATION */
