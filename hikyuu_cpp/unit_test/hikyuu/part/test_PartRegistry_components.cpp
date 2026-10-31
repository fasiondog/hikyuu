/*
 * test_PartRegistry_components.cpp
 *
 *  Created on: 2026-10-30
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <set>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/part/PartRegistration.h>
#include <hikyuu/part/PartRegistry.h>
#include <hikyuu/trade_sys/signal/SignalBase.h>

using namespace hku;

/**
 * @defgroup test_part_PartRegistry_components test_part_PartRegistry_components
 * @ingroup test_hikyuu_part_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_registerComponentParts") {
    PartRegistry::release();
    registerAllBuiltinParts();
    auto& reg = PartRegistry::instance();

    /** @arg every category matches the built-in components of the core library */
    CHECK_EQ(reg.getNameList(PartType::SG).size(), 10);
    CHECK_EQ(reg.getNameList(PartType::MM).size(), 9);
    CHECK_EQ(reg.getNameList(PartType::ST).size(), 3);
    CHECK_EQ(reg.getNameList(PartType::PG).size(), 3);
    CHECK_EQ(reg.getNameList(PartType::SP).size(), 6);
    CHECK_EQ(reg.getNameList(PartType::AF).size(), 5);
    CHECK_EQ(reg.getNameList(PartType::CN).size(), 3);
    CHECK_EQ(reg.getNameList(PartType::EV).size(), 3);
    CHECK_EQ(reg.getNameList(PartType::MF).size(), 4);
    CHECK_EQ(reg.getNameList(PartType::PF).size(), 3);
    CHECK_EQ(reg.getNameList(PartType::SE).size(), 5);
    CHECK_EQ(reg.getNameList(PartType::SYS).size(), 2);
    CHECK_EQ(reg.getNameList(PartType::TM).size(), 1);
    CHECK_EQ(reg.getNameList(PartType::TC).size(), 6);

    /** @arg a component is created from its scalar and part params */
    Parameter sc;
    sc.set<double>("lower", 10.0);
    sc.set<double>("upper", 20.0);
    PartParamMap parts;
    parts["ind"].push_back(MA(10));
    auto band = reg.create("core.sg.band", sc, parts);
    CHECK_UNARY(std::get<SGPtr>(band).get() != nullptr);

    /** @arg a component whose params are all optional is created with the defaults */
    auto tc = reg.create("core.tc.zero");
    CHECK_EQ(partTypeOf(tc), PartType::TC);

    /** @arg a part taking a value list can not be created from the registry */
    CHECK_THROWS_AS(reg.create("core.af.fixedweightlist"), std::exception);

    /** @arg the short names stay unique across all categories (they are a secondary lookup key
     *  of createFromString) */
    std::set<string> names;
    for (auto type : {PartType::IND, PartType::SG, PartType::MM, PartType::ST, PartType::PG,
                      PartType::SP, PartType::AF, PartType::CN, PartType::EV, PartType::MF,
                      PartType::PF, PartType::SE, PartType::SYS, PartType::TM, PartType::TC}) {
        for (const auto& m : reg.getMetaList(type)) {
            bool inserted = names.insert(m.name).second;
            CHECK_UNARY(inserted);
        }
    }
    CHECK_EQ(names.size(), 201);
}

/** @} */
