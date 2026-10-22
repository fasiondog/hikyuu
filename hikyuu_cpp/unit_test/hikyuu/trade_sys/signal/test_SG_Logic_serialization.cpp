/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2026-10-23
 *      Author: fasiondog
 */

#include "../../test_config.h"
#include <hikyuu/config.h>

#if HKU_SUPPORT_SERIALIZATION

#include <sstream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/signal/build_in.h>

using namespace hku;

/**
 * @defgroup test_SG_Logic_serialization test_SG_Logic_serialization
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

static void round_trip(const SignalPtr& src) {
    std::ostringstream os;
    {
        boost::archive::xml_oarchive oa(os);
        SignalPtr copy = src;
        oa << BOOST_SERIALIZATION_NVP(copy);
    }
    const std::string xml = os.str();

    SignalPtr dst;
    {
        std::istringstream is(xml);
        boost::archive::xml_iarchive ia(is);
        ia >> BOOST_SERIALIZATION_NVP(dst);
    }

    REQUIRE(dst);
    CHECK_EQ(dst->name(), src->name());
    CHECK(dst->getBuySignal() == src->getBuySignal());
    CHECK(dst->getSellSignal() == src->getSellSignal());
    CHECK_UNARY(dst->getTO().empty());
}

/** @par Test points */
TEST_CASE("test_SG_logic_operator_serialization") {
    KData k = getKData("sz000001", KQueryByIndex(-30));
    REQUIRE(k.size() >= 10);

    auto a = SG_AllwaysBuy();
    auto b = SG_AllwaysBuy();
    a->setTO(k);
    b->setTO(k);

    SUBCASE("SG_And") {
        auto sg = SG_And(a, b, false);
        sg->setTO(k);
        round_trip(sg);
    }
    SUBCASE("SG_Or") {
        auto sg = SG_Or(a, b, false);
        sg->setTO(k);
        round_trip(sg);
    }
    SUBCASE("SG_Add") {
        auto sg = SG_Add(a, b, false);
        sg->setTO(k);
        round_trip(sg);
    }
    SUBCASE("SG_Sub") {
        auto sg = SG_Sub(a, b, false);
        sg->setTO(k);
        round_trip(sg);
    }
    SUBCASE("SG_Mul") {
        auto sg = SG_Mul(a, b, false);
        sg->setTO(k);
        round_trip(sg);
    }
    SUBCASE("SG_Div") {
        auto sg = SG_Div(a, b, false);
        sg->setTO(k);
        round_trip(sg);
    }
}

/** @par Test points */
TEST_CASE("test_SG_value_operator_serialization") {
    KData k = getKData("sz000001", KQueryByIndex(-30));
    REQUIRE(k.size() >= 10);

    auto a = SG_AllwaysBuy();
    a->setTO(k);

    SUBCASE("AddValue") {
        auto sg = a + 1.0;
        sg->setTO(k);
        round_trip(sg);
    }
    SUBCASE("SubValue") {
        auto sg = a - 1.0;
        sg->setTO(k);
        round_trip(sg);
    }
    SUBCASE("MulValue") {
        auto sg = a * 2.0;
        sg->setTO(k);
        round_trip(sg);
    }
    SUBCASE("DivValue") {
        auto sg = a / 2.0;
        sg->setTO(k);
        round_trip(sg);
    }
}

/** @par Test points
 * Regression lock for the base-of-base duplicate: the derived OPERATOR_SIGNAL macro used to write
 * both SignalBase and OperatorSignal, so the base subobject (and its m_buySig / m_sellSig) landed
 * in the archive twice per composite. Only the immediate base should be listed.
 */
TEST_CASE("test_SG_logic_serialization_no_duplicate_base") {
    KData k = getKData("sz000001", KQueryByIndex(-5));
    REQUIRE(k.size() >= 3);

    auto a = SG_AllwaysBuy();
    auto b = SG_AllwaysBuy();
    a->setTO(k);
    b->setTO(k);
    auto sg = SG_And(a, b, false);
    sg->setTO(k);

    std::ostringstream os;
    {
        boost::archive::xml_oarchive oa(os);
        SignalPtr copy = sg;
        oa << BOOST_SERIALIZATION_NVP(copy);
    }
    const std::string xml = os.str();

    // Boost xml writes the base subobject tag once per serialization of that base. Composite +
    // m_sg1 + m_sg2 = 3 opening <SignalBase> tags. Pre-fix the composite emitted two tags for
    // its own base (once directly, once via OperatorSignal), giving 4.
    size_t count = 0;
    const std::string needle = "<SignalBase";
    for (size_t pos = 0; (pos = xml.find(needle, pos)) != std::string::npos; pos += needle.size()) {
        ++count;
    }
    CHECK_EQ(count, 3u);
}

/** @} */

#endif /* HKU_SUPPORT_SERIALIZATION */
