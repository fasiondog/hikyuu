/*
 * test_PartRegistry_create.cpp
 *
 *  Created on: 2026-10-30
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/part/BuiltinParts.h>
#include <hikyuu/part/PartRegistry.h>
#include <hikyuu/trade_manage/crt/TC_FixedETF.h>

using namespace hku;

/**
 * @defgroup test_part_PartRegistry_create test_part_PartRegistry_create
 * @ingroup test_hikyuu_part_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_PartRegistry_create_default") {
    registerBuiltinParts();
    auto& reg = PartRegistry::instance();

    /** @arg create with all params at their defaults equals the direct crt call */
    Indicator a = tryGetAs<Indicator>(reg.create("core.ind.MA")).value();
    Indicator b = MA();
    CHECK_UNARY(a.alike(b));

    /** @arg an unregistered name throws */
    CHECK_THROWS_AS(reg.create("testhub.ind.nosuch"), std::out_of_range);
}

/** @par Test points */
TEST_CASE("test_PartRegistry_create_with_params") {
    registerBuiltinParts();
    auto& reg = PartRegistry::instance();

    /** @arg provided scalars override the defaults */
    Parameter sc;
    sc.set<int>("n", 10);
    Indicator a = tryGetAs<Indicator>(reg.create("core.ind.MA", sc)).value();
    CHECK_UNARY(a.alike(MA(10)));

    /** @arg an empty Parameter falls back to the defaults */
    Indicator b = tryGetAs<Indicator>(reg.create("core.ind.MA", Parameter())).value();
    CHECK_UNARY(b.alike(MA()));
}

/** @par Test points */
TEST_CASE("test_PartRegistry_create_component") {
    registerBuiltinParts();
    auto& reg = PartRegistry::instance();

    /** @arg a part-typed param is passed via the PartParamMap and the scalars via Parameter */
    Parameter sc;
    sc.set<double>("lower", 10.0);
    sc.set<double>("upper", 20.0);
    PartRegistry::PartParamMap parts;
    parts["ind"] = {Part(MA(5))};
    Part part = reg.create("core.sg.SG_Band", sc, parts);

    auto sg = tryGetAs<SignalPtr>(part);
    CHECK_UNARY(sg.has_value());
    CHECK_UNARY(*sg != nullptr);
    CHECK(partName(part) == "SG_Band");

    /** @arg a missing required part param throws with the param name */
    CHECK_THROWS_AS(reg.create("core.sg.SG_Band"), std::invalid_argument);

    /** @arg a missing required scalar param throws */
    Parameter bad_sc;
    bad_sc.set<double>("lower", 10.0);
    CHECK_THROWS_AS(reg.create("core.sg.SG_Band", bad_sc, parts), std::invalid_argument);

    /** @arg a wrong part type in the param channel throws with the param name */
    PartRegistry::PartParamMap bad_parts;
    bad_parts["ind"] = {Part(TradeManagerPtr())};
    CHECK_THROWS_AS(reg.create("core.sg.SG_Band", sc, bad_parts), std::invalid_argument);

    /** @arg a non-list part param takes exactly 1 element */
    PartRegistry::PartParamMap multi_parts;
    multi_parts["ind"] = {Part(MA(5)), Part(MA(6))};
    CHECK_THROWS_AS(reg.create("core.sg.SG_Band", sc, multi_parts), std::invalid_argument);

    /** @arg an unknown scalar param key is rejected */
    Parameter unknown_sc;
    unknown_sc.set<int>("lowerr", 1);
    CHECK_THROWS_AS(reg.create("core.sg.SG_Band", unknown_sc, parts), std::invalid_argument);

    /** @arg an unknown part param key is rejected */
    PartRegistry::PartParamMap unknown_map;
    unknown_map["indd"] = {Part(MA(5))};
    CHECK_THROWS_AS(reg.create("core.sg.SG_Band", sc, unknown_map), std::invalid_argument);

    /** @arg a part-typed param must not be passed through Parameter */
    Parameter wrong_channel;
    wrong_channel.set<int>("ind", 5);
    CHECK_THROWS_AS(reg.create("core.sg.SG_Band", wrong_channel, parts), std::invalid_argument);

    /** @arg a scalar param must not be passed through the PartParamMap */
    PartRegistry::PartParamMap wrong_map;
    wrong_map["lower"] = {Part(MA(5))};
    CHECK_THROWS_AS(reg.create("core.sg.SG_Band", sc, wrong_map), std::invalid_argument);
}

/** @par Test points */
TEST_CASE("test_PartRegistry_create_list_param") {
    auto& reg = PartRegistry::instance();
    string name = "testhub.sys.listtest";
    reg.unregister(name);

    PartMeta meta;
    meta.type = PartType::SYS;
    meta.name = name;
    meta.full_name = name;
    ParamMeta list_param;
    list_param.name = "sys_list";
    list_param.type = "System";
    list_param.is_part_param = true;
    list_param.is_list = true;
    list_param.required = true;
    meta.params.push_back(list_param);

    reg.registerPart(meta, [](const Parameter&, const PartRegistry::PartParamMap& pp) -> Part {
        SYSPtr sys;
        const auto& lst = pp.at("sys_list");
        if (!lst.empty()) {
            sys = std::get<SYSPtr>(lst.at(0));
        }
        return Part(sys);
    });

    /** @arg a list param accepts N elements and passes them in order */
    PartRegistry::PartParamMap parts;
    parts["sys_list"] = {Part(SYSPtr()), Part(SYSPtr())};
    Part part = reg.create(name, Parameter(), parts);
    CHECK_UNARY(tryGetAs<SYSPtr>(part).has_value());

    /** @arg a missing list param throws when required */
    CHECK_THROWS_AS(reg.create(name), std::invalid_argument);

    /** @arg a wrong element type in the list throws */
    PartRegistry::PartParamMap bad_parts;
    bad_parts["sys_list"] = {Part(MA(5))};
    CHECK_THROWS_AS(reg.create(name, Parameter(), bad_parts), std::invalid_argument);

    reg.unregister(name);
}

/** @par Test points */
TEST_CASE("test_PartRegistry_create_tm") {
    registerBuiltinParts();
    auto& reg = PartRegistry::instance();

    /** @arg the TM part is created with its defaults */
    Part part = reg.create("core.tm.crtTM");
    auto tm = tryGetAs<TradeManagerPtr>(part);
    CHECK_UNARY(tm.has_value());
    CHECK_UNARY(*tm != nullptr);
    CHECK(partTypeOf(part) == PartType::TM);

    /** @arg the scalar overrides reach the creator */
    Parameter sc;
    sc.set<string>("name", "TEST_TM");
    auto tm2 = tryGetAs<TradeManagerPtr>(reg.create("core.tm.crtTM", sc)).value();
    CHECK(tm2->name() == "TEST_TM");

    /** @arg the part-typed costfunc is passed via the PartParamMap */
    PartRegistry::PartParamMap parts;
    parts["costfunc"] = {Part(TC_FixedETF())};
    auto tm3 = tryGetAs<TradeManagerPtr>(reg.create("core.tm.crtTM", Parameter(), parts)).value();
    CHECK_UNARY(tm3->costFunc() != nullptr);
    CHECK(tm3->costFunc()->name() == "TC_FixedETF");
}

/** @par Test points */
TEST_CASE("test_PartRegistry_create_tc") {
    registerBuiltinParts();
    auto& reg = PartRegistry::instance();

    /** @arg the TC part is created with its defaults */
    Part part = reg.create("core.tc.TC_FixedETF");
    auto tc = tryGetAs<TradeCostPtr>(part);
    CHECK_UNARY(tc.has_value());
    CHECK_UNARY(*tc != nullptr);
    CHECK(partTypeOf(part) == PartType::TC);

    /** @arg the short C++ name resolves in the string expression */
    auto tc2 = tryGetAs<TradeCostPtr>(reg.createFromString("TC_FixedETF")).value();
    CHECK_UNARY(tc2 != nullptr);
}

/** @par Test points */
TEST_CASE("test_PartRegistry_create_type_mismatch") {
    registerBuiltinParts();
    auto& reg = PartRegistry::instance();

    /** @arg a scalar of the wrong type throws with the expected type */
    Parameter sc;
    sc.set<double>("n", 10.0);
    CHECK_THROWS_AS(reg.create("core.ind.MA", sc), std::invalid_argument);
}

/** @par Test points */
TEST_CASE("test_PartRegistry_createFromString") {
    registerBuiltinParts();
    auto& reg = PartRegistry::instance();

    /** @arg a bare name equals the default construction */
    CHECK_UNARY(tryGetAs<Indicator>(reg.createFromString("MA")).value().alike(MA()));

    /** @arg an empty argument list equals the default construction */
    CHECK_UNARY(tryGetAs<Indicator>(reg.createFromString("MA()")).value().alike(MA()));

    /** @arg a single param overrides the default */
    CHECK_UNARY(tryGetAs<Indicator>(reg.createFromString("MA(n=10)")).value().alike(MA(10)));

    /** @arg the full name also works */
    CHECK_UNARY(
      tryGetAs<Indicator>(reg.createFromString("core.ind.MA(n=10)")).value().alike(MA(10)));

    /** @arg a short C++ name resolves via meta.name; SG_Band has required params, so the
     *  resolution succeeds but creation throws on the missing params */
    CHECK_THROWS_AS(reg.createFromString("SG_Band"), std::invalid_argument);

    /** @arg an unknown param name throws listing the part */
    CHECK_THROWS_AS(reg.createFromString("MA(m=10)"), std::invalid_argument);

    /** @arg a value of the wrong type throws with the expected type */
    CHECK_THROWS_AS(reg.createFromString("MA(n=abc)"), std::invalid_argument);

    /** @arg unbalanced parentheses throw */
    CHECK_THROWS_AS(reg.createFromString("MA(n=10"), std::invalid_argument);

    /** @arg a missing '=' in an argument throws */
    CHECK_THROWS_AS(reg.createFromString("MA(10)"), std::invalid_argument);

    /** @arg an empty expression throws */
    CHECK_THROWS_AS(reg.createFromString(""), std::invalid_argument);
    CHECK_THROWS_AS(reg.createFromString("()"), std::invalid_argument);

    /** @arg an unregistered part throws */
    CHECK_THROWS_AS(reg.createFromString("UNKNOWN_PART(n=1)"), std::out_of_range);

    /** @arg a part-typed param cannot be set in a string expression */
    CHECK_THROWS_AS(reg.createFromString("SG_Band(ind=MA)"), std::invalid_argument);
}

/** @} */
