/*
 * test_Part.cpp
 *
 *  Created on: 2026-10-30
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/part/Part.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_sys/signal/crt/SG_Band.h>

using namespace hku;

/**
 * @defgroup test_part_Part test_part_Part
 * @ingroup test_hikyuu_part_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_Part_variant_size") {
    /** @arg the variant holds exactly the declared 15 alternatives */
    static_assert(std::variant_size_v<Part> == 15, "Part alternative count changed");
    CHECK(std::variant_size_v<Part> == 15);
}

/** @par Test points */
TEST_CASE("test_partTypeName_parsePartType") {
    /** @arg every PartType round-trips through its string name */
    const PartType types[] = {PartType::IND, PartType::SG, PartType::MM, PartType::ST,
                              PartType::CN,  PartType::EV, PartType::PG, PartType::SP,
                              PartType::PF,  PartType::SE, PartType::AF, PartType::MF,
                              PartType::SYS, PartType::TM, PartType::TC};
    for (PartType type : types) {
        const char* name = partTypeName(type);
        CHECK(name != nullptr);
        auto parsed = parsePartType(name);
        CHECK_UNARY(parsed.has_value());
        CHECK(*parsed == type);
    }

    /** @arg invalid names return std::nullopt */
    CHECK_UNARY_FALSE(parsePartType("").has_value());
    CHECK_UNARY_FALSE(parsePartType("xxx").has_value());
    CHECK_UNARY_FALSE(parsePartType("IND").has_value());  // case sensitive, lowercase only
}

/** @par Test points */
TEST_CASE("test_partTypeOf") {
    /** @arg the value-semantics alternative maps to IND */
    CHECK(partTypeOf(Part(Indicator())) == PartType::IND);

    /** @arg each shared_ptr alternative maps to its own category */
    CHECK(partTypeOf(Part(SignalPtr())) == PartType::SG);
    CHECK(partTypeOf(Part(MMPtr())) == PartType::MM);
    CHECK(partTypeOf(Part(STPtr())) == PartType::ST);
    CHECK(partTypeOf(Part(CNPtr())) == PartType::CN);
    CHECK(partTypeOf(Part(EVPtr())) == PartType::EV);
    CHECK(partTypeOf(Part(PGPtr())) == PartType::PG);
    CHECK(partTypeOf(Part(SPPtr())) == PartType::SP);
    CHECK(partTypeOf(Part(PFPtr())) == PartType::PF);
    CHECK(partTypeOf(Part(SEPtr())) == PartType::SE);
    CHECK(partTypeOf(Part(AFPtr())) == PartType::AF);
    CHECK(partTypeOf(Part(MFPtr())) == PartType::MF);
    CHECK(partTypeOf(Part(SYSPtr())) == PartType::SYS);
    CHECK(partTypeOf(Part(TradeManagerPtr())) == PartType::TM);
    CHECK(partTypeOf(Part(TradeCostPtr())) == PartType::TC);
}

/** @par Test points */
TEST_CASE("test_partName") {
    /** @arg the name of a created indicator */
    CHECK(partName(Part(MA())) == "MA");

    /** @arg the name of a created component */
    CHECK(partName(Part(SG_Band(MA(), 10.0, 20.0))) == "SG_Band");

    /** @arg an empty handle yields an empty name */
    CHECK(partName(Part(SignalPtr())) == "");
}

/** @par Test points */
TEST_CASE("test_visitPart") {
    Part part(MA());

    /** @arg the matching branch is selected */
    int called = 0;
    visitPart(part, [&](const Indicator&) { called = 1; }, [](const auto&) { CHECK_UNARY(false); });
    CHECK(called == 1);

    /** @arg the fallback branch catches the alternatives without a dedicated handler */
    Part sg_part{SignalPtr()};
    called = 0;
    visitPart(sg_part, [&](const Indicator&) { called = 1; }, [&](const auto&) { called = 2; });
    CHECK(called == 2);

    /** @arg the mutable overload permits in-place modification of the held alternative */
    Part mut_part{SignalPtr()};
    visitPart(
      mut_part,
      [](SignalPtr& p) {
          CHECK_UNARY(p == nullptr);
          p.reset();
          CHECK_UNARY(p == nullptr);
      },
      [](const auto&) {});
}

/** @par Test points */
TEST_CASE("test_tryGetAs") {
    Part part(MA());

    /** @arg the matching alternative is returned */
    auto ind = tryGetAs<Indicator>(part);
    CHECK_UNARY(ind.has_value());
    CHECK(ind->name() == "MA");

    /** @arg a type mismatch returns std::nullopt instead of throwing */
    CHECK_UNARY_FALSE(tryGetAs<SignalPtr>(part).has_value());
    CHECK_UNARY_FALSE(tryGetAs<TradeManagerPtr>(part).has_value());
}

/** @} */
