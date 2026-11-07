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
#include <hikyuu/trade_sys/system/imp/MultiSystem.h>

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
    CHECK_EQ(reg.getNameList(PartType::PF).size(), 6);
    CHECK_EQ(reg.getNameList(PartType::SE).size(), 5);
    CHECK_EQ(reg.getNameList(PartType::SYS).size(), 2);
    CHECK_EQ(reg.getNameList(PartType::TM).size(), 1);
    CHECK_EQ(reg.getNameList(PartType::TC).size(), 7);
    CHECK_UNARY(reg.has("core.tc.fixed"));
    CHECK_EQ(reg.getNameList(PartType::MG).size(), 3);

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
    for (auto type :
         {PartType::IND, PartType::SG, PartType::MM, PartType::ST, PartType::PG, PartType::SP,
          PartType::AF, PartType::CN, PartType::EV, PartType::MF, PartType::PF, PartType::SE,
          PartType::SYS, PartType::TM, PartType::TC, PartType::MG}) {
        for (const auto& m : reg.getMetaList(type)) {
            bool inserted = names.insert(m.name).second;
            CHECK_UNARY(inserted);
        }
    }
    CHECK_EQ(names.size(), 209);
}

/** @par Test points */
TEST_CASE("test_registerPortfolioModeAliasParts") {
    PartRegistry::release();
    registerAllBuiltinParts();
    auto& reg = PartRegistry::instance();

    /** @arg every mode alias is exported as a part */
    vector<string> mode_names = {"core.pf.modea", "core.pf.modeb", "core.pf.modec"};
    for (const string& name : mode_names) {
        CHECK_UNARY(reg.has(name));
    }

    /** @arg the lookup is case insensitive, as for any other part */
    CHECK_EQ(reg.getMeta("core.pf.ModeA").full_name, "core.pf.modea");

    /** @arg the alias keeps the C++ alias name, the meta of its semantic part, and creates the
     *  same running mode */
    struct AliasCase {
        string alias_name;
        string alias_meta_name;
        string semantic_name;
    };
    for (const auto& c : {AliasCase{"core.pf.modea", "PF_ModeA", "core.pf.signalaggregate"},
                          AliasCase{"core.pf.modeb", "PF_ModeB", "core.pf.simple"},
                          AliasCase{"core.pf.modec", "PF_ModeC", "core.pf.withoutaf"}}) {
        const PartMeta& alias_meta = reg.getMeta(c.alias_name);
        const PartMeta& semantic_meta = reg.getMeta(c.semantic_name);
        CHECK_EQ(alias_meta.type, PartType::PF);
        CHECK_EQ(alias_meta.name, c.alias_meta_name);
        REQUIRE_EQ(alias_meta.params.size(), semantic_meta.params.size());
        for (size_t i = 0; i < alias_meta.params.size(); ++i) {
            CHECK_EQ(alias_meta.params[i].name, semantic_meta.params[i].name);
            CHECK_EQ(alias_meta.params[i].default_value, semantic_meta.params[i].default_value);
        }

        auto alias = std::get<PFPtr>(reg.create(c.alias_name));
        auto semantic = std::get<PFPtr>(reg.create(c.semantic_name));
        CHECK_UNARY(alias != nullptr);
        CHECK_EQ(alias->getMode(), semantic->getMode());
    }

    /** @arg the alias forwards the scalar overrides to its semantic factory */
    Parameter sc;
    sc.set<int>("adjust_cycle", 2);
    auto mode_b = std::get<PFPtr>(reg.create("core.pf.modeb", sc));
    CHECK_EQ(mode_b->getAdjustCycle(), 2);

    /** @arg an unknown param is rejected against the alias meta */
    Parameter bad;
    bad.set<double>("lower", 10.0);
    CHECK_THROWS_AS(reg.create("core.pf.modea", bad), std::exception);

    /** @arg the registration is idempotent */
    registerAllBuiltinParts();
    CHECK_EQ(reg.getNameList(PartType::PF).size(), 6);
}

/** @} */
