/*
 * test_export.cpp
 *
 * Serialization round-trip tests for the slippage component.
 */

#include "doctest/doctest.h"
#include <hikyuu/config.h>

#if HKU_SUPPORT_SERIALIZATION

#include <fstream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/slippage/build_in.h>

using namespace hku;

/**
 * @defgroup test_slippage_serialization test_slippage_serialization
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

namespace {

// Write the slippage to the XML archive and read it back
SlippagePtr roundTrip(SlippagePtr sp, const string& filename) {
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(sp);
    }

    SlippagePtr result;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(result);
    }
    return result;
}

}  // namespace

/** @par Test points */
TEST_CASE("test_SP_FixedPercent_export") {
    StockManager& sm = StockManager::instance();
    SlippagePtr sp1 = SP_FixedPercent(0.02);
    SlippagePtr sp2 = roundTrip(sp1, sm.tmpdir() + "/SP_FixedPercent.xml");
    /** @arg the name and every parameter survive the round-trip */
    CHECK_EQ(sp1->name(), sp2->name());
    CHECK_EQ(sp1->getParameter(), sp2->getParameter());
}

/** @par Test points */
TEST_CASE("test_SP_FixedValue_export") {
    StockManager& sm = StockManager::instance();
    SlippagePtr sp1 = SP_FixedValue(0.03);
    SlippagePtr sp2 = roundTrip(sp1, sm.tmpdir() + "/SP_FixedValue.xml");
    /** @arg the name and every parameter survive the round-trip */
    CHECK_EQ(sp1->name(), sp2->name());
    CHECK_EQ(sp1->getParameter(), sp2->getParameter());
}

/** @par Test points */
TEST_CASE("test_SP_Uniform_export") {
    StockManager& sm = StockManager::instance();
    SlippagePtr sp1 = SP_Uniform(-0.05, 0.05);
    sp1->setParam<int64_t>("seed", 42);
    SlippagePtr sp2 = roundTrip(sp1, sm.tmpdir() + "/SP_Uniform.xml");
    /** @arg the name and every parameter (including the seed) survive the round-trip */
    CHECK_EQ(sp1->name(), sp2->name());
    CHECK_EQ(sp1->getParameter(), sp2->getParameter());
    CHECK_EQ(sp2->getParam<int64_t>("seed"), 42);
}

/** @par Test points */
TEST_CASE("test_SP_Normal_export") {
    StockManager& sm = StockManager::instance();
    SlippagePtr sp1 = SP_Normal(0.01, 0.05);
    SlippagePtr sp2 = roundTrip(sp1, sm.tmpdir() + "/SP_Normal.xml");
    /** @arg the name and every parameter survive the round-trip */
    CHECK_EQ(sp1->name(), sp2->name());
    CHECK_EQ(sp1->getParameter(), sp2->getParameter());
}

/** @par Test points */
TEST_CASE("test_SP_LogNormal_export") {
    StockManager& sm = StockManager::instance();
    SlippagePtr sp1 = SP_LogNormal(0.01, 0.05);
    SlippagePtr sp2 = roundTrip(sp1, sm.tmpdir() + "/SP_LogNormal.xml");
    /** @arg the name and every parameter survive the round-trip */
    CHECK_EQ(sp1->name(), sp2->name());
    CHECK_EQ(sp1->getParameter(), sp2->getParameter());
}

/** @par Test points */
TEST_CASE("test_SP_TruncNormal_export") {
    StockManager& sm = StockManager::instance();
    SlippagePtr sp1 = SP_TruncNormal(0.01, 0.05, -0.1, 0.1);
    SlippagePtr sp2 = roundTrip(sp1, sm.tmpdir() + "/SP_TruncNormal.xml");
    /** @arg the name and every parameter survive the round-trip */
    CHECK_EQ(sp1->name(), sp2->name());
    CHECK_EQ(sp1->getParameter(), sp2->getParameter());
}

/** @par Test points */
TEST_CASE("test_SP_short_param_export") {
    StockManager& sm = StockManager::instance();
    SlippagePtr sp1 = SP_Uniform(-0.05, 0.05, true, -0.2, 0.2);
    SlippagePtr sp2 = roundTrip(sp1, sm.tmpdir() + "/SP_Uniform_short.xml");
    /** @arg the short-side parameters survive the round-trip */
    CHECK_EQ(sp1->getParameter(), sp2->getParameter());
    CHECK_EQ(sp2->getParam<double>("short_min_value"), -0.2);
    CHECK_EQ(sp2->getParam<double>("short_max_value"), 0.2);
}

/** @} */

#endif /* HKU_SUPPORT_SERIALIZATION */
