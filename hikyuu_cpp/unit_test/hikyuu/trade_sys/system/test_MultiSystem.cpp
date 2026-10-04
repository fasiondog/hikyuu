/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-16
 *      Author: fasiondog
 *
 *  Regression tests for the aggregated system MultiSystem.
 *  Coverage:
 *    1. MultiSystem structure: run mode, clone fidelity, composite check, circular reference
 *       detection (direct/nested), arbitrary-depth nesting, mode B quota write-back
 *       (checkin/checkout), driving axis mode and fixed axis injection, the run(query)
 *       compatible overload (master Portfolio::run(query) semantics), and external
 *       rebalancing date table injection (normalize & dedupe / overwrite / clear);
 *    2. Portfolio-level funds allocation engine AllocateFundsBase (AF L1/L2/L3):
 *       - L1 system-level allocation: equal weight / weight-list fixed weights (migrated
 *                        from AF_FixedWeight/FixedWeightList) / mode B real quota (including
 *                        fixed-amount migrated from AF_FixedAmount);
 *       - L2 behavior-level conversion: mode A conversion by assets_ratio (full semantics
 *                        pass-through of design 8.3) / fixed-amount / SELL-CLEAR closing the
 *                        whole position / over-allocation converted to position reduction /
 *                        mode B pass-through clipping;
 *       - L3 portfolio risk control: max-single-position concentration clipping / skipped
 *                        in mode B.
 *  @note MM has been restricted to the single-system form; the portfolio-level allocation
 *        (L1/L2/L3) is migrated entirely to AF (see docs/design/pf_af_compat/design.md §5).
 */

#include "../../test_config.h"
#include <unordered_map>
#include <type_traits>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/system/imp/MultiSystem.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include <hikyuu/trade_sys/portfolio/build_in.h>
#include <hikyuu/trade_sys/allocatefunds/build_in.h>
#include <hikyuu/trade_sys/selector/crt/SE_Fixed.h>
#include <hikyuu/trade_sys/signal/crt/SG_AllwaysBuy.h>
#include <hikyuu/trade_sys/signal/crt/SG_Cycle.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_Nothing.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include "create_test_sys.h"

using namespace hku;

/**
 * @defgroup test_MultiSystem test_MultiSystem
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** Deterministic always-buy test system (signal timing independent of the market window) */
static SYSPtr create_alway_buy_sys() {
    auto sys = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_AllwaysBuy());
    // The default buy_delay/sell_delay is true (deferred to the next open); here it is changed to
    // an immediate trade, to make the signal timing deterministic
    sys->setParam<bool>("buy_delay", false);
    sys->setParam<bool>("sell_delay", false);
    sys->name("test_sys_alway_buy");
    return sys;
}

// ============================================================================
// MultiSystem structure
// ============================================================================

/** @par Check point: run mode semantics (default A; B/b normalized to B; invalid values fall
 * back to A) */
TEST_CASE("test_MultiSystem_mode") {
    MultiSystem ms;
    CHECK_EQ(ms.getMode(), "A");

    ms.setMode("B");
    CHECK_EQ(ms.getMode(), "B");

    ms.setMode("b");  // lowercase is also recognized as B
    CHECK_EQ(ms.getMode(), "B");

    ms.setMode("X");  // invalid value normalized to the default A
    CHECK_EQ(ms.getMode(), "A");
}

/** @par Check point: clone preserves the run mode and the aggregation parameters (guards the
 * defect that _clone loses the run mode/m_sell_at_not_selected) */
TEST_CASE("test_MultiSystem_clone_preserve_mode") {
    auto ms = std::make_shared<MultiSystem>("outer");
    ms->add(create_test_sys(3, 5));
    ms->setMode("B");
    ms->setAdjustCycle(5);
    ms->setSellAtNotSelected(false);
    ms->setSubInitCash(50000.0);
    REQUIRE_EQ(ms->getMode(), "B");

    auto cloned = std::dynamic_pointer_cast<MultiSystem>(ms->clone());
    REQUIRE(cloned != nullptr);

    CHECK_EQ(cloned->name(), "outer");
    CHECK_EQ(cloned->getMode(), "B");       // clone preserves the run mode (defect fix)
    CHECK_EQ(cloned->getAdjustCycle(), 5);  // clone preserves the rebalancing cycle
    CHECK_EQ(cloned->getSellAtNotSelected(),
             false);  // clone preserves sell_at_not_selected (defect fix)
    CHECK_EQ(cloned->getSubInitCash(), doctest::Approx(50000.0));
    REQUIRE_EQ(cloned->getSystemList().size(), 1);  // sub-system is cloned
    CHECK(cloned->getSystemList()[0] !=
          ms->getSystemList()[0]);  // deep copy: a different instance from the source
}

/** @par Check point: driving timeline mode (default kdata; calendar takes effect; invalid
 * values fall back) and fixed axis injection/clear */
TEST_CASE("test_MultiSystem_axis_mode") {
    MultiSystem ms("ms");
    /** @arg the default driving axis is kdata (the input KData carries its own dates; existing
     * behavior kept) */
    CHECK_EQ(ms.getAxisMode(), "kdata");
    CHECK_EQ(ms.getParam<string>("axis-mode"), "kdata");

    /** @arg switchable to calendar (the injected fixed date table drives the axis) */
    ms.setAxisMode("calendar");
    CHECK_EQ(ms.getAxisMode(), "calendar");

    /** @arg an invalid value falls back to kdata */
    ms.setAxisMode("month");
    CHECK_EQ(ms.getAxisMode(), "kdata");

    /** @arg fixed date table injection / read / clear */
    CHECK_EQ(ms.getDateAxis().size(), 0);
    ms.setDateAxis({Datetime(20250102), Datetime(20250103)});
    REQUIRE_EQ(ms.getDateAxis().size(), 2);
    CHECK_EQ(ms.getDateAxis()[0], Datetime(20250102));
    CHECK_EQ(ms.getDateAxis()[1], Datetime(20250103));
    ms.clearDateAxis();
    CHECK_EQ(ms.getDateAxis().size(), 0);
}

/** @par Check point: the run(query) compatible overload (equivalent to master
 * Portfolio::run(query), see design.md §4.5) */
TEST_CASE("test_MultiSystem_run_query") {
    /** @arg with no fixed timeline injected and no sub-system, run(query) warns and returns
     * without throwing */
    auto ms = std::make_shared<MultiSystem>("ms");
    ms->setTM(crtTM(Datetime(201111010000LL), 100000.0));
    CHECK_NOTHROW(ms->run(KQuery(Datetime(20111101), Datetime(20111230))));
    CHECK_EQ(ms->getSystemList().size(), 0);

    /** @arg with a fixed timeline injected, run(query) uses it as the driving axis, and does not
     * modify the input query nor the injected axis */
    auto ms2 = std::make_shared<MultiSystem>("ms2");
    ms2->setTM(crtTM(Datetime(201111010000LL), 100000.0));
    auto sys = create_test_sys(3, 5);
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(20111101), Datetime(20111230));
    sys->setTO(stk.getKData(query));
    ms2->add(sys);
    ms2->setAxisMode("calendar");
    ms2->setDateAxis({Datetime(20111122), Datetime(20111123), Datetime(20111124)});

    ms2->run(query);
    /** @arg the context KData carries the input query (sub-system calculation and price queries
     * both use it as the context) */
    CHECK_EQ(ms2->getQuery(), query);
    /** @arg the injected fixed timeline is not rewritten */
    REQUIRE_EQ(ms2->getDateAxis().size(), 3);
    CHECK_EQ(ms2->getDateAxis()[0], Datetime(20111122));
    CHECK_EQ(ms2->getDateAxis()[2], Datetime(20111124));
    /** @arg after running, the sub-system's context KData matches the input query (run(kdata)
     * semantics preserved) */
    REQUIRE_EQ(ms2->getSystemList().size(), 1);
    CHECK_EQ(ms2->getSystemList()[0]->getQuery(), query);
}

/** @par Check point: external rebalancing date table injection (normalize & dedupe, ignore
 * Null) / overwrite-style write / clone fidelity / clear falls back */
TEST_CASE("test_MultiSystem_adjust_dates") {
    auto ms = std::make_shared<MultiSystem>("ms");
    /** @arg initially empty → falls back to the closing-day counting check of m_adjust_cycle */
    CHECK_EQ(ms->getAdjustDates().size(), 0);

    /** @arg after injection, dates are normalized to midnight of the day and deduplicated; Null
     * dates (default constructed) are ignored */
    ms->setAdjustDates(
      {Datetime(20250106), Datetime(2025, 1, 6, 10, 30), Datetime(20250107), Datetime()});
    CHECK_EQ(ms->getAdjustDates().size(), 2);
    CHECK_UNARY(ms->getAdjustDates().count(Datetime(20250106)) == 1);
    CHECK_UNARY(ms->getAdjustDates().count(Datetime(20250107)) == 1);

    /** @arg repeated injection overwrites (no accumulation) */
    ms->setAdjustDates({Datetime(20250210)});
    CHECK_EQ(ms->getAdjustDates().size(), 1);

    /** @arg clone preserves the rebalancing date table (clone fidelity) */
    auto cloned = std::dynamic_pointer_cast<MultiSystem>(ms->clone());
    REQUIRE(cloned != nullptr);
    CHECK_EQ(cloned->getAdjustDates().size(), 1);
    CHECK_UNARY(cloned->getAdjustDates().count(Datetime(20250210)) == 1);

    /** @arg after clearing, falls back to the counting check */
    ms->clearAdjustDates();
    CHECK_EQ(ms->getAdjustDates().size(), 0);
}

/** @par Check point: composite check and the sub-system list */
TEST_CASE("test_MultiSystem_isComposite") {
    MultiSystem ms;
    /** @arg the aggregated system's isComposite is always true */
    CHECK(ms.isComposite());
    /** @arg no sub-system initially */
    CHECK_EQ(ms.getSubSystemList().size(), 0);
    /** @arg a single-security system's isComposite is false */
    auto sys = create_test_sys(3, 5);
    CHECK(!sys->isComposite());
}

/** @par Check point: add's direct circular reference and duplicate detection */
TEST_CASE("test_MultiSystem_add") {
    auto ms = std::make_shared<MultiSystem>("ms");
    auto sys1 = create_test_sys(3, 5);
    /** @arg adding a sub-system normally */
    ms->add(sys1);
    CHECK_EQ(ms->getSystemList().size(), 1);
    /** @arg a null pointer is rejected */
    ms->add(SystemPtr());
    CHECK_EQ(ms->getSystemList().size(), 1);
    /** @arg adding the same instance twice is rejected */
    ms->add(sys1);
    CHECK_EQ(ms->getSystemList().size(), 1);
    /** @arg adding itself is rejected (direct circular reference) */
    ms->add(ms);
    CHECK_EQ(ms->getSystemList().size(), 1);
}

/** @par Check point: indirect circular reference detection in nested aggregation */
TEST_CASE("test_MultiSystem_add_cycle_nested") {
    auto outer = std::make_shared<MultiSystem>("outer");
    auto inner = std::make_shared<MultiSystem>("inner");
    auto leaf = create_test_sys(3, 5);
    inner->add(leaf);
    /** @arg inner as a sub-system of outer (normal nesting) */
    outer->add(inner);
    CHECK_EQ(outer->getSystemList().size(), 1);
    /** @arg an indirect cycle is rejected: outer already contains inner, so inner cannot contain
     * outer */
    inner->add(outer);
    CHECK_EQ(inner->getSystemList().size(), 1);  // inner still only has leaf
}

/** @par Check point: nested structure of arbitrary depth (outer → inner → leaf) */
TEST_CASE("test_MultiSystem_nesting") {
    auto outer = std::make_shared<MultiSystem>("outer");
    auto inner = std::make_shared<MultiSystem>("inner");
    inner->add(create_test_sys(3, 5));
    outer->add(inner);
    /** @arg the outer direct sub-system is inner */
    REQUIRE_EQ(outer->getSubSystemList().size(), 1);
    CHECK(outer->getSubSystemList()[0] == inner);
    /** @arg the direct sub-system is itself a composite (recursion allowed) */
    CHECK(outer->getSubSystemList()[0]->isComposite());
    /** @arg the inner layer holds the leaf sub-system */
    CHECK_EQ(inner->getSubSystemList().size(), 1);
    CHECK(!inner->getSubSystemList()[0]->isComposite());
}

/** @par Check point: mode B quota write-back (checkin/checkout adjusts the sub-system virtual
 * account total assets) */
TEST_CASE("test_MultiSystem_setSubSystemQuota") {
    auto ms = std::make_shared<MultiSystem>("ms");
    auto sub = create_test_sys(3, 5);  // with its own crtTM, initial cash 100000
    ms->add(sub);
    Datetime date(200001010000LL);
    REQUIRE_EQ(sub->getTM()->getFunds(date, KQuery::DAY).total_assets(), doctest::Approx(100000.0));

    /** @arg quota increase: checkin the difference, total assets rise to the quota */
    ms->setSubSystemQuota(sub, date, 150000.0);
    CHECK_EQ(sub->getTM()->getFunds(date, KQuery::DAY).total_assets(), doctest::Approx(150000.0));

    /** @arg quota decrease: checkout the difference, total assets drop to the quota */
    ms->setSubSystemQuota(sub, date, 80000.0);
    CHECK_EQ(sub->getTM()->getFunds(date, KQuery::DAY).total_assets(), doctest::Approx(80000.0));

    /** @arg quota<=0 is rejected (total assets unchanged) */
    ms->setSubSystemQuota(sub, date, 0.0);
    CHECK_EQ(sub->getTM()->getFunds(date, KQuery::DAY).total_assets(), doctest::Approx(80000.0));

    /** @arg a null sub-system is rejected (no crash) */
    ms->setSubSystemQuota(SystemPtr(), date, 100000.0);
    CHECK_EQ(sub->getTM()->getFunds(date, KQuery::DAY).total_assets(), doctest::Approx(80000.0));
}

// ============================================================================
// Portfolio allocation engine L1: system-level allocation (nominal weights / real quota)
// ============================================================================

/** @par Check point: L1 default equal-weight allocation (AF_EqualWeight) */
TEST_CASE("test_AllocateFunds_L1_equal_weight") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto af = AF_EqualWeight();
    af->setMode("A");
    Datetime date(200001010000LL);

    /** @arg empty contexts returns an empty weight table (boundary: 0 sub-systems) */
    {
        SubSystemContextList empty;
        auto w = af->_allocate(date, tm, empty, KQuery());
        CHECK_EQ(w.size(), 0);
    }

    /** @arg 2 sub-systems get equal weight 1/N */
    auto sys1 = create_test_sys(3, 5);
    auto sys2 = create_test_sys(5, 10);
    SubSystemContextList contexts(2);
    contexts[0].sys = sys1;
    contexts[1].sys = sys2;
    auto w = af->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(0.5));
    CHECK_EQ(w[sys2], doctest::Approx(0.5));

    /** @arg mode A does not write quota (quota is produced only in mode B) */
    CHECK_EQ(contexts[0].quota, doctest::Approx(0.0));
    CHECK_EQ(contexts[1].quota, doctest::Approx(0.0));
}

/** @par Check point: L1 weight-list fixed weights (an AF base-class parameter with
 * normalization semantics) */
TEST_CASE("test_AllocateFunds_L1_weight_list") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto af = AF_EqualWeight();
    af->setMode("A");
    Datetime date(200001010000LL);
    auto sys1 = create_test_sys(3, 5);
    auto sys2 = create_test_sys(5, 10);
    SubSystemContextList contexts(2);
    contexts[0].sys = sys1;
    contexts[1].sys = sys2;

    /** @arg weights summing to 1 are adopted as is */
    af->setParam<string>("weight-list", "0.6,0.4");
    auto w = af->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(0.6));
    CHECK_EQ(w[sys2], doctest::Approx(0.4));

    /** @arg weights not summing to 1 are normalized automatically (3:7 → 0.3/0.7) */
    af->setParam<string>("weight-list", "3,7");
    w = af->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(0.3));
    CHECK_EQ(w[sys2], doctest::Approx(0.7));

    /** @arg count mismatch (3 items vs 2 sub-systems): falls back to equal weight */
    af->setParam<string>("weight-list", "0.5,0.3,0.2");
    w = af->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(0.5));
    CHECK_EQ(w[sys2], doctest::Approx(0.5));

    /** @arg negative weights are treated as 0 and then normalized (0.5,-0.5 → 1.0/0.0) */
    af->setParam<string>("weight-list", "0.5,-0.5");
    w = af->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(1.0));
    CHECK_EQ(w[sys2], doctest::Approx(0.0));

    /** @arg all items invalid (parsed sum is 0), falls back to equal weight */
    af->setParam<string>("weight-list", "abc,def");
    w = af->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(0.5));
    CHECK_EQ(w[sys2], doctest::Approx(0.5));

    /** @arg an empty string falls back to equal weight */
    af->setParam<string>("weight-list", "");
    w = af->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(0.5));
    CHECK_EQ(w[sys2], doctest::Approx(0.5));
}

/** @par Check point: L1 mode B real quota (equal weight / weight-list / fixed-amount) */
TEST_CASE("test_AllocateFunds_L1_modeB_quota") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto af = AF_EqualWeight();
    af->setMode("B");
    Datetime date(200001010000LL);
    auto sys1 = create_test_sys(3, 5);
    auto sys2 = create_test_sys(5, 10);

    /** @arg single sub-system equal weight: quota = weight(1.0) × parent total assets(100000) */
    SubSystemContextList contexts(1);
    contexts[0].sys = sys1;
    af->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(contexts[0].quota, doctest::Approx(100000.0));

    /** @arg fixed-amount>0: quota = the fixed amount (migrated from AF_FixedAmount) */
    af->setParam<double>("fixed-amount", 30000.0);
    af->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(contexts[0].quota, doctest::Approx(30000.0));

    /** @arg weight-list affects quota allocation (0.7/0.3 → 70000/30000) */
    af->setParam<double>("fixed-amount", 0.0);
    af->setParam<string>("weight-list", "0.7,0.3");
    SubSystemContextList ctx2(2);
    ctx2[0].sys = sys1;
    ctx2[1].sys = sys2;
    af->_allocate(date, tm, ctx2, KQuery());
    CHECK_EQ(ctx2[0].quota, doctest::Approx(70000.0));
    CHECK_EQ(ctx2[1].quota, doctest::Approx(30000.0));
}

/** @par Check point: L1 fixed weights (AF_FixedWeight / AF_FixedWeightList, no normalization) */
TEST_CASE("test_AllocateFunds_L1_fixed_weight") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    Datetime date(200001010000LL);
    auto sys1 = create_test_sys(3, 5);
    auto sys2 = create_test_sys(5, 10);
    SubSystemContextList contexts(2);
    contexts[0].sys = sys1;
    contexts[1].sys = sys2;

    /** @arg AF_FixedWeight(0.3): each sub-system gets 0.3, **no normalization** (vs. weight-list)
     */
    auto af = AF_FixedWeight(0.3);
    af->setMode("A");
    auto w = af->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(0.3));
    CHECK_EQ(w[sys2], doctest::Approx(0.3));

    /** @arg contrast with the weight-list normalization semantics (0.3,0.3 → 0.5,0.5),
     * confirming AF_FixedWeight does not normalize */
    auto af_norm = AF_EqualWeight();
    af_norm->setMode("A");
    af_norm->setParam<string>("weight-list", "0.3,0.3");
    auto w_norm = af_norm->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(w_norm[sys1], doctest::Approx(0.5));
    CHECK_EQ(w_norm[sys2], doctest::Approx(0.5));

    /** @arg mode B: quota = 0.3 × parent total assets(100000) = 30000 */
    af->setMode("B");
    af->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(contexts[0].quota, doctest::Approx(30000.0));
    CHECK_EQ(contexts[1].quota, doctest::Approx(30000.0));

    /** @arg empty contexts returns an empty table (boundary: 0 sub-systems) */
    {
        SubSystemContextList empty;
        auto we = af->_allocate(date, tm, empty, KQuery());
        CHECK_EQ(we.size(), 0);
    }

    /** @arg AF_FixedWeightList takes weights in order, no normalization */
    auto afl = AF_FixedWeightList({0.4, 0.2});
    afl->setMode("A");
    w = afl->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(0.4));
    CHECK_EQ(w[sys2], doctest::Approx(0.2));

    /** @arg count mismatch (3 items vs 2 sub-systems): falls back to equal weight */
    auto afl3 = AF_FixedWeightList({0.4, 0.2, 0.1});
    afl3->setMode("A");
    w = afl3->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(0.5));
    CHECK_EQ(w[sys2], doctest::Approx(0.5));

    /** @arg an empty weight list falls back to equal weight (boundary) */
    auto afl0 = AF_FixedWeightList(PriceList{});
    afl0->setMode("A");
    w = afl0->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(0.5));
    CHECK_EQ(w[sys2], doctest::Approx(0.5));
}

/** @par Check point: L1 multi-factor score weights (AF_MultiFactor: SubSystemContext::score
 * as the weight) */
TEST_CASE("test_AllocateFunds_L1_multifactor") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    Datetime date(200001010000LL);
    auto sys1 = create_test_sys(3, 5);
    auto sys2 = create_test_sys(5, 10);
    SubSystemContextList contexts(2);
    contexts[0].sys = sys1;
    contexts[0].score = 0.7;
    contexts[1].sys = sys2;
    contexts[1].score = 0.3;

    auto af = AF_MultiFactor();
    af->setMode("A");

    /** @arg scores are used directly as weights (no normalization) */
    auto w = af->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(0.7));
    CHECK_EQ(w[sys2], doctest::Approx(0.3));

    /** @arg negative scores are treated as 0 */
    contexts[1].score = -1.0;
    w = af->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(0.7));
    CHECK_EQ(w[sys2], doctest::Approx(0.0));

    /** @arg all scores are 0 (no SE / not a rebalancing day): falls back to equal weight, to
     * avoid zero weights that cannot be allocated */
    contexts[0].score = 0.0;
    contexts[1].score = 0.0;
    w = af->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(0.5));
    CHECK_EQ(w[sys2], doctest::Approx(0.5));

    /** @arg mode B: quota = score × parent total assets */
    contexts[0].score = 0.6;
    contexts[1].score = 0.4;
    af->setMode("B");
    af->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(contexts[0].quota, doctest::Approx(60000.0));
    CHECK_EQ(contexts[1].quota, doctest::Approx(40000.0));

    /** @arg empty contexts returns an empty table (boundary) */
    {
        SubSystemContextList empty;
        CHECK_EQ(af->_allocate(date, tm, empty, KQuery()).size(), 0);
    }
}

// ============================================================================
// Portfolio allocation engine L2: behavior-level conversion (mode A ratio / mode B pass-through)
// ============================================================================

/** @par Check point: L2 mode A conversion by assets_ratio (full semantics pass-through of
 * design 8.3, F2) */
TEST_CASE("test_AllocateFunds_L2_modeA_by_ratio") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);  // no position yet: all cash, predictable
    auto af = AF_EqualWeight();
    af->setMode("A");
    Datetime date(200001010000LL);
    Stock stk = getStock("sz000001");
    REQUIRE(!stk.isNull());
    auto sys = create_test_sys(3, 5);
    std::unordered_map<SYSPtr, double> weights{{sys, 1.0}};

    /** @arg assets_ratio>0: target value = weight(1.0) × ratio(0.5) × parent total assets(100000)
     * = 50000 → 5000 shares */
    TradeSuggestion s;
    s.stock = stk;
    s.sys = sys;
    s.type = SuggestionType::BUY;
    s.plan_price = 10.0;
    s.number = 100;  // the original number should be rewritten
    s.assets_ratio = 0.5;
    TradeSuggestionList suggestions{s};
    af->_toTargets(date, tm, suggestions, weights, KQuery());
    CHECK(suggestions[0].type == SuggestionType::BUY);
    CHECK_EQ(suggestions[0].number, doctest::Approx(5000.0));

    /** @arg assets_ratio<=0: falls back to full position (ratio=1) → 100000/10 = 10000 shares */
    s.assets_ratio = 0.0;
    suggestions[0] = s;
    af->_toTargets(date, tm, suggestions, weights, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(10000.0));

    /** @arg plan_price<=0: number set to 0 (division-by-zero boundary) */
    s.plan_price = 0.0;
    s.assets_ratio = 0.5;
    suggestions[0] = s;
    af->_toTargets(date, tm, suggestions, weights, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(0.0));
}

/** @par Check point: L2 mode A over-allocation converted to position reduction (delta<0 →
 * SELL, the F2 rebalancing branch) */
TEST_CASE("test_AllocateFunds_L2_overweight_to_sell") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto af = AF_EqualWeight();
    af->setMode("A");
    // use fixed-amount to pin the target value, so total_assets is not affected by market moves
    af->setParam<double>("fixed-amount", 5000.0);
    Datetime date(200001010000LL);
    Stock stk = getStock("sz000001");
    REQUIRE(!stk.isNull());
    auto sys = create_test_sys(3, 5);
    std::unordered_map<SYSPtr, double> weights{{sys, 1.0}};
    tm->buy(date, stk, 10.0, 1000.0);  // build a position of 1000 shares (deterministic)
    REQUIRE_EQ(tm->getPosition(date, stk).number, doctest::Approx(1000.0));

    /** @arg target(5000/10=500 shares) < current(1000 shares): converted to SELL of 500 shares */
    TradeSuggestion s;
    s.stock = stk;
    s.sys = sys;
    s.type = SuggestionType::BUY;
    s.plan_price = 10.0;
    TradeSuggestionList suggestions{s};
    af->_toTargets(date, tm, suggestions, weights, KQuery());
    CHECK(suggestions[0].type == SuggestionType::SELL);
    CHECK_EQ(suggestions[0].number, doctest::Approx(500.0));
}

/** @par Check point: L2 mode A SELL/CLEAR closes the whole current position */
TEST_CASE("test_AllocateFunds_L2_sell_clear") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto af = AF_EqualWeight();
    af->setMode("A");
    Datetime date(200001010000LL);
    Stock stk = getStock("sz000001");
    REQUIRE(!stk.isNull());
    auto sys = create_test_sys(3, 5);
    std::unordered_map<SYSPtr, double> weights{{sys, 1.0}};
    tm->buy(date, stk, 10.0, 1000.0);  // build a position of 1000 shares

    /** @arg SELL: number = -current (close the whole position), the original number is rewritten */
    TradeSuggestion s;
    s.stock = stk;
    s.sys = sys;
    s.type = SuggestionType::SELL;
    s.plan_price = 10.0;
    s.number = 999;
    TradeSuggestionList suggestions{s};
    af->_toTargets(date, tm, suggestions, weights, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(-1000.0));

    /** @arg CLEAR: also closes the whole position */
    s.type = SuggestionType::CLEAR;
    suggestions[0] = s;
    af->_toTargets(date, tm, suggestions, weights, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(-1000.0));

    /** @arg SELL with an empty position: number = -0 = 0 (boundary) */
    auto tm2 = crtTM(Datetime(200001010000LL), 100000.0);
    s.type = SuggestionType::SELL;
    s.number = 500;
    suggestions[0] = s;
    af->_toTargets(date, tm2, suggestions, weights, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(0.0));
}

/** @par Check point: L2 fixed-amount per instrument (migrated from AF_FixedAmount, F4) */
TEST_CASE("test_AllocateFunds_L2_fixed_amount") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto af = AF_EqualWeight();
    af->setMode("A");
    af->setParam<double>("fixed-amount", 20000.0);
    Datetime date(200001010000LL);
    Stock stk = getStock("sz000001");
    REQUIRE(!stk.isNull());
    auto sys = create_test_sys(3, 5);
    std::unordered_map<SYSPtr, double> weights{{sys, 1.0}};

    /** @arg fixed-amount takes precedence over assets_ratio: target value = 20000 → 2000 shares */
    TradeSuggestion s;
    s.stock = stk;
    s.sys = sys;
    s.type = SuggestionType::BUY;
    s.plan_price = 10.0;
    s.assets_ratio = 0.9;  // should be overridden by fixed-amount
    TradeSuggestionList suggestions{s};
    af->_toTargets(date, tm, suggestions, weights, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(2000.0));
}

/** @par Check point: L2 mode B passes sub-system instructions through (the parent does no
 * conversion, only defensively clips SELL) */
TEST_CASE("test_AllocateFunds_L2_modeB_passthrough") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto af = AF_EqualWeight();
    af->setMode("B");
    Datetime date(200001010000LL);
    Stock stk = getStock("sz000001");
    REQUIRE(!stk.isNull());
    auto sys = create_test_sys(3, 5);
    std::unordered_map<SYSPtr, double> weights{{sys, 1.0}};

    /** @arg BUY passes the original number through (no conversion by parent assets) */
    TradeSuggestion s;
    s.stock = stk;
    s.sys = sys;
    s.type = SuggestionType::BUY;
    s.plan_price = 10.0;
    s.number = 888;
    TradeSuggestionList suggestions{s};
    af->_toTargets(date, tm, suggestions, weights, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(888.0));

    /** @arg SELL clipped to not exceed the parent's current position (current=0 → clipped to 0) */
    s.type = SuggestionType::SELL;
    s.number = 1000;
    suggestions[0] = s;
    af->_toTargets(date, tm, suggestions, weights, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(0.0));
}

// ============================================================================
// Portfolio allocation engine L3: portfolio risk-control clipping
// ============================================================================

/** @par Check point: L3 concentration risk control (max-single-position clipping / no limit /
 * mode B skips) */
TEST_CASE("test_AllocateFunds_L3_check_risk") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);  // no position yet, total_assets=100000
    auto af = AF_EqualWeight();
    af->setMode("A");
    Datetime date(200001010000LL);
    Stock stk = getStock("sz000001");
    REQUIRE(!stk.isNull());
    auto sys = create_test_sys(3, 5);
    TradeSuggestion s;
    s.stock = stk;
    s.sys = sys;
    s.type = SuggestionType::BUY;
    s.plan_price = 10.0;
    s.number = 20000;  // target value 200000

    /** @arg max-single-position=0.5: clipped to a value of 50000 = 5000 shares */
    af->setParam<double>("max-single-position", 0.5);
    TradeSuggestionList suggestions{s};
    af->_checkRisk(date, tm, suggestions, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(5000.0));

    /** @arg max-single-position>=1.0: no limit (number unchanged) */
    af->setParam<double>("max-single-position", 1.0);
    suggestions[0] = s;
    af->_checkRisk(date, tm, suggestions, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(20000.0));

    /** @arg mode B: risk control is skipped (respecting sub-manager autonomy, number unchanged) */
    af->setMode("B");
    af->setParam<double>("max-single-position", 0.5);
    suggestions[0] = s;
    af->_checkRisk(date, tm, suggestions, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(20000.0));
}

// ============================================================================
// adjust-mode internalization (rebalancing date expansion) and the PF/AF preset factories
// ============================================================================

/** @par Check point: the adjust-mode parameter (default value, case normalization, invalid
 * fallback) and the delay-to-trading-day switch */
TEST_CASE("test_MultiSystem_adjust_mode") {
    MultiSystem ms("ms");
    /** @arg the default adjust-mode is "query", delay-to-trading-day is true */
    CHECK_EQ(ms.getAdjustMode(), "query");
    CHECK_EQ(ms.getParam<string>("adjust-mode"), "query");
    CHECK_EQ(ms.getDelayToTradingDay(), true);

    /** @arg all valid values can be set (case normalized to lowercase) */
    ms.setAdjustMode("WEEK");
    CHECK_EQ(ms.getAdjustMode(), "week");
    ms.setAdjustMode("Month");
    CHECK_EQ(ms.getAdjustMode(), "month");
    ms.setAdjustMode("quarter");
    CHECK_EQ(ms.getAdjustMode(), "quarter");
    ms.setAdjustMode("year");
    CHECK_EQ(ms.getAdjustMode(), "year");
    ms.setAdjustMode("day");
    CHECK_EQ(ms.getAdjustMode(), "day");
    ms.setAdjustMode("query");
    CHECK_EQ(ms.getAdjustMode(), "query");

    /** @arg an invalid value falls back to query (no exception thrown) */
    ms.setAdjustMode("monthly");
    CHECK_EQ(ms.getAdjustMode(), "query");
    ms.setAdjustMode("");
    CHECK_EQ(ms.getAdjustMode(), "query");

    /** @arg delay-to-trading-day can be toggled independently */
    ms.setDelayToTradingDay(false);
    CHECK_EQ(ms.getDelayToTradingDay(), false);
    ms.setDelayToTradingDay(true);
    CHECK_EQ(ms.getDelayToTradingDay(), true);
}

/** @par Check point: the calcAdjustDates pure function (week/month/quarter/year × delay
 * true/false + boundaries) */
TEST_CASE("test_MultiSystem_calc_adjust_dates") {
    /** @arg an empty axis returns empty (boundary: 0 trading days) */
    CHECK_EQ(MultiSystem::calcAdjustDates(DatetimeList(), "month", 1, true).size(), 0);

    /** @arg an invalid mode returns empty */
    DatetimeList axis{Datetime(2025, 1, 1), Datetime(2025, 1, 2), Datetime(2025, 1, 6)};
    CHECK_EQ(MultiSystem::calcAdjustDates(axis, "query", 1, true).size(), 0);
    CHECK_EQ(MultiSystem::calcAdjustDates(axis, "day", 1, false).size(), 0);
    CHECK_EQ(MultiSystem::calcAdjustDates(axis, "", 1, false).size(), 0);

    // 2025-01 real calendar: Jan1=Wed, Jan2=Thu, Jan3=Fri, Jan6=Mon, Jan7=Tue, ... Jan10=Fri,
    //                        Jan13=Mon, Jan14=Tue, Jan15=Wed, Jan16=Thu, Jan17=Fri, Jan20=Mon,
    //                        Jan21=Tue, Jan22=Wed, Jan23=Thu, Jan24=Fri (simulated trading-day
    //                        axis; see below for when 1/1 is skipped)
    DatetimeList jan{Datetime(2025, 1, 1),  Datetime(2025, 1, 2),  Datetime(2025, 1, 3),
                     Datetime(2025, 1, 6),  Datetime(2025, 1, 7),  Datetime(2025, 1, 8),
                     Datetime(2025, 1, 9),  Datetime(2025, 1, 10), Datetime(2025, 1, 13),
                     Datetime(2025, 1, 14), Datetime(2025, 1, 15), Datetime(2025, 1, 16),
                     Datetime(2025, 1, 17), Datetime(2025, 1, 20), Datetime(2025, 1, 21),
                     Datetime(2025, 1, 22), Datetime(2025, 1, 23), Datetime(2025, 1, 24)};

    /** @arg week/1 without deferral: hits all Mondays (dayOfWeek()==1) */
    {
        auto r = MultiSystem::calcAdjustDates(jan, "week", 1, false);
        REQUIRE_EQ(r.size(), 3);
        CHECK_EQ(r[0], Datetime(2025, 1, 6));
        CHECK_EQ(r[1], Datetime(2025, 1, 13));
        CHECK_EQ(r[2], Datetime(2025, 1, 20));
    }

    /** @arg week/2 without deferral: hits all Tuesdays (Jan 7/14/21) */
    {
        auto r = MultiSystem::calcAdjustDates(jan, "week", 2, false);
        REQUIRE_EQ(r.size(), 3);
        CHECK_EQ(r[0], Datetime(2025, 1, 7));
        CHECK_EQ(r[2], Datetime(2025, 1, 21));
    }

    /** @arg month/1 without deferral: hits only the trading day with "day==1" (Jan 1 here) */
    {
        auto r = MultiSystem::calcAdjustDates(jan, "month", 1, false);
        REQUIRE_EQ(r.size(), 1);
        CHECK_EQ(r[0], Datetime(2025, 1, 1));
    }

    /** @arg month/15 without deferral: hits only the trading day with "day==15" */
    {
        auto r = MultiSystem::calcAdjustDates(jan, "month", 15, false);
        REQUIRE_EQ(r.size(), 1);
        CHECK_EQ(r[0], Datetime(2025, 1, 15));
    }

    /** @arg month/32 without deferral: there is no day 32 (boundary: out of range) */
    CHECK_EQ(MultiSystem::calcAdjustDates(jan, "month", 32, false).size(), 0);

    /** @arg quarter/1 | year/1 without deferral: the 1st day of the month/year (Jan 1) */
    {
        auto rq = MultiSystem::calcAdjustDates(jan, "quarter", 1, false);
        REQUIRE_EQ(rq.size(), 1);
        CHECK_EQ(rq[0], Datetime(2025, 1, 1));
        auto ry = MultiSystem::calcAdjustDates(jan, "year", 1, false);
        REQUIRE_EQ(ry.size(), 1);
        CHECK_EQ(ry[0], Datetime(2025, 1, 1));
    }

    /** @arg adjust_cycle<=0 is treated as 1 (boundary: 0 and negative values) */
    {
        auto r0 = MultiSystem::calcAdjustDates(jan, "week", 0, false);
        auto r1 = MultiSystem::calcAdjustDates(jan, "week", 1, false);
        REQUIRE_EQ(r0.size(), r1.size());
        CHECK_EQ(r0[0], r1[0]);
        auto rn = MultiSystem::calcAdjustDates(jan, "week", -5, false);
        REQUIRE_EQ(rn.size(), r1.size());
        CHECK_EQ(rn[0], r1[0]);
    }

    /** @arg month/1 with deferral: when Jan 1 is not a trading day, deferred to the first
     * trading day of the period (Jan 2) */
    {
        DatetimeList jan2{Datetime(2025, 1, 2), Datetime(2025, 1, 3), Datetime(2025, 1, 6),
                          Datetime(2025, 1, 7), Datetime(2025, 1, 8)};
        auto r = MultiSystem::calcAdjustDates(jan2, "month", 1, true);
        REQUIRE_EQ(r.size(), 1);
        CHECK_EQ(r[0], Datetime(2025, 1, 2));
        /** @arg the same period is not hit twice (Jan 3/6/7/8 no longer hit) */
    }

    /** @arg month/1 with deferral: when the target day is a trading day, that very day is hit */
    {
        auto r = MultiSystem::calcAdjustDates(jan, "month", 1, true);
        REQUIRE_EQ(r.size(), 1);
        CHECK_EQ(r[0], Datetime(2025, 1, 1));
    }

    /** @arg week/1 with deferral: when Monday is a holiday, deferred to the first trading day of
     * the week (Jan 6 is Monday → hit directly) */
    {
        DatetimeList noMon{Datetime(2025, 1, 7), Datetime(2025, 1, 8), Datetime(2025, 1, 9),
                           Datetime(2025, 1, 10), Datetime(2025, 1, 14)};
        auto r = MultiSystem::calcAdjustDates(noMon, "week", 1, true);
        REQUIRE_EQ(r.size(), 2);
        CHECK_EQ(r[0], Datetime(2025, 1, 7));   // Jan 6 missing → deferred to first trading day
        CHECK_EQ(r[1], Datetime(2025, 1, 14));  // Jan 13 missing → deferred to Jan 14
    }

    /** @arg cross-month axis quarter/1 with deferral: hit only once within the same quarter
     * (Jan 1 missing → Jan 2 hits; no hit again in February) */
    {
        DatetimeList two{Datetime(2025, 1, 2), Datetime(2025, 1, 3), Datetime(2025, 2, 3),
                         Datetime(2025, 2, 4)};
        auto rq = MultiSystem::calcAdjustDates(two, "quarter", 1, true);
        REQUIRE_EQ(rq.size(), 1);
        CHECK_EQ(rq[0], Datetime(2025, 1, 2));

        /** @arg month/1 with deferral: on a cross-month axis, hits once per month */
        auto rm = MultiSystem::calcAdjustDates(two, "month", 1, true);
        REQUIRE_EQ(rm.size(), 2);
        CHECK_EQ(rm[0], Datetime(2025, 1, 2));
        CHECK_EQ(rm[1], Datetime(2025, 2, 3));
    }
}

/** @par Check point: the PF compatible factories (PF_Simple → mode B / PF_WithoutAF → mode A,
 * parameters mapped item by item) */
TEST_CASE("test_PF_factories") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);

    /** @arg PF_Simple: mode B; the af of type AFPtr is mapped into MultiSystem; SE/rebalancing
     * parameters passed through item by item */
    auto pf = PF_Simple(tm, SE_Fixed(), AF_FixedWeight(0.2), 5, "month", false);
    REQUIRE(pf != nullptr);
    CHECK_EQ(pf->name(), "PF_Simple");
    CHECK_EQ(pf->getMode(), "B");
    CHECK_EQ(pf->getAdjustCycle(), 5);
    CHECK_EQ(pf->getAdjustMode(), "month");
    CHECK_EQ(pf->getDelayToTradingDay(), false);
    REQUIRE(pf->getAF() != nullptr);
    CHECK_EQ(pf->getAF()->name(), "AF_FixedWeight");
    CHECK_EQ(pf->getAF()->getParam<double>("weight"), doctest::Approx(0.2));
    CHECK_UNARY(pf->getSE() != nullptr);
    /** @arg aligned with master SimplePortfolio: trade on close + force sell when not selected */
    CHECK_EQ(pf->getTradeOnClose(), true);
    CHECK_EQ(pf->getSellAtNotSelected(), true);

    /** @arg PF_Simple default parameters: the af defaults to equal weight (AF_EqualWeight),
     * adjust_cycle=1, mode="query", delay=true */
    auto pf_def = PF_Simple(tm, SE_Fixed());
    CHECK_EQ(pf_def->getMode(), "B");
    CHECK_EQ(pf_def->getAdjustCycle(), 1);
    CHECK_EQ(pf_def->getAdjustMode(), "query");
    CHECK_EQ(pf_def->getDelayToTradingDay(), true);
    REQUIRE(pf_def->getAF() != nullptr);
    CHECK_EQ(pf_def->getAF()->name(), "AF_EqualWeight");

    /** @arg PF_WithoutAF: mode A; parameters passed through item by item; sys_use_self_tm has no
     * corresponding semantics but must not crash */
    auto pf2 = PF_WithoutAF(tm, SE_Fixed(), 3, "week", true, false, true, true);
    REQUIRE(pf2 != nullptr);
    CHECK_EQ(pf2->name(), "PF_WithoutAF");
    CHECK_EQ(pf2->getMode(), "A");
    CHECK_EQ(pf2->getAdjustCycle(), 3);
    CHECK_EQ(pf2->getAdjustMode(), "week");
    CHECK_EQ(pf2->getDelayToTradingDay(), true);
    CHECK_EQ(pf2->getTradeOnClose(), false);
    CHECK_EQ(pf2->getSellAtNotSelected(), true);
    REQUIRE(pf2->getAF() != nullptr);
    CHECK_EQ(pf2->getAF()->name(), "AF_EqualWeight");

    /** @arg PF_WithoutAF defaults: trade_on_close=true, sell_at_not_selected=false */
    auto pf2_def = PF_WithoutAF(tm, SE_Fixed());
    CHECK_EQ(pf2_def->getTradeOnClose(), true);
    CHECK_EQ(pf2_def->getSellAtNotSelected(), false);
    REQUIRE(pf2_def->getAF() != nullptr);
    CHECK_EQ(pf2_def->getAF()->name(), "AF_EqualWeight");
}

/** @par Check point: the AF factories (return AFPtr; each built-in algorithm's name and
 * parameters passed through) */
TEST_CASE("test_AF_factories") {
    /** @arg AF_EqualWeight → equal-weight allocation (L1 equal weight 1/N) */
    auto af1 = AF_EqualWeight();
    REQUIRE(af1 != nullptr);
    CHECK_EQ(af1->name(), "AF_EqualWeight");

    /** @arg AF_FixedWeight → fixed weight (weight passed through) */
    auto af2 = AF_FixedWeight(0.25);
    REQUIRE(af2 != nullptr);
    CHECK_EQ(af2->name(), "AF_FixedWeight");
    CHECK_EQ(af2->getParam<double>("weight"), doctest::Approx(0.25));

    /** @arg AF_FixedWeightList → fixed weight list (the weight list passed through) */
    auto af3 = AF_FixedWeightList({0.3, 0.7});
    REQUIRE(af3 != nullptr);
    CHECK_EQ(af3->name(), "AF_FixedWeightList");
    auto ws = af3->getParam<PriceList>("weights");
    REQUIRE_EQ(ws.size(), 2);
    CHECK_EQ(ws[0], doctest::Approx(0.3));
    CHECK_EQ(ws[1], doctest::Approx(0.7));

    /** @arg AF_FixedAmount → fixed amount (the fixed-amount parameter passed through) */
    auto af4 = AF_FixedAmount(30000.0);
    REQUIRE(af4 != nullptr);
    CHECK_EQ(af4->name(), "AF_FixedAmount");
    CHECK_EQ(af4->getParam<double>("fixed-amount"), doctest::Approx(30000.0));

    /** @arg AF_MultiFactor → multi-factor */
    auto af5 = AF_MultiFactor();
    REQUIRE(af5 != nullptr);
    CHECK_EQ(af5->name(), "AF_MultiFactor");
}

/** @par Check point: master compatible aliases (PortfolioPtr → MultiSystemPtr, AFPtr →
 * AllocateFundsPtr, see design.md §4.5 / §5) */
TEST_CASE("test_PF_AF_compat_aliases") {
    /** @arg PortfolioPtr and MultiSystemPtr are the same type, so legacy
     * `PortfolioPtr pf = PF_Simple(...)` still compiles */
    static_assert(std::is_same_v<PortfolioPtr, MultiSystemPtr>,
                  "PortfolioPtr must alias MultiSystemPtr");
    /** @arg AFPtr and AllocateFundsPtr are the same type (AF is independent of MM) */
    static_assert(std::is_same_v<AFPtr, AllocateFundsPtr>, "AFPtr must alias AllocateFundsPtr");

    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    PortfolioPtr pf = PF_Simple(tm, SE_Fixed(), AF_FixedWeight(0.2));
    REQUIRE(pf != nullptr);
    CHECK_EQ(pf->getMode(), "B");

    AFPtr af = AF_FixedWeightList({0.4, 0.6});
    REQUIRE(af != nullptr);
    CHECK_EQ(af->name(), "AF_FixedWeightList");
}

// ============================================================================
// Re-review issue regressions and the L2 same-stock aggregation
// ============================================================================

/** @par Check point: the clone with SE forces recalculation on the first run (after cloning, the SE
 * cache points to the original system, so the clone selects no sub-system) */
TEST_CASE("test_MultiSystem_clone_with_se_recalculate") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData kdata = stk.getKData(query);
    REQUIRE(kdata.size() > 0);

    auto ms = std::make_shared<MultiSystem>("ms");
    ms->setTM(crtTM(Datetime(199001010000LL), 100000.0));
    auto sys = create_alway_buy_sys();
    sys->setTO(kdata);
    ms->add(sys);
    ms->setSE(SE_Fixed());
    ms->run(kdata);
    auto trades = ms->getTM()->getTradeList();
    REQUIRE_EQ(trades.size(), 2);  // INIT + first-day buy

    /** @arg the clone runs with the same data, and its trades match the original system
     * (selection and execution work normally) */
    auto cloned = std::dynamic_pointer_cast<MultiSystem>(ms->clone());
    REQUIRE(cloned != nullptr);
    cloned->run(kdata);
    CHECK_EQ(cloned->getTM()->getTradeList().size(), trades.size());
}

/** @par Check point: the shadow account is created only the first time (repeated readyForRun must
 * not rebuild the sub-system account) */
TEST_CASE("test_MultiSystem_shadow_tm_reuse") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData kdata = stk.getKData(query);

    auto ms = std::make_shared<MultiSystem>("ms");
    ms->setTM(crtTM(Datetime(199001010000LL), 100000.0));
    auto sys = create_alway_buy_sys();
    sys->setTO(kdata);
    ms->add(sys);
    ms->run(kdata);
    auto sub_tm = ms->getSystemList()[0]->getTM();
    REQUIRE(sub_tm != nullptr);

    /** @arg replay with reset=false: the shadow account instance is unchanged (delayed
     * requests/mode B quota are preserved) */
    ms->run(kdata, false);
    CHECK(ms->getSystemList()[0]->getTM() == sub_tm);

    /** @arg re-run with reset=true: the account is not rebuilt; the sub-system state is reset
     * first and then re-driven */
    ms->run(kdata, true);
    CHECK(ms->getSystemList()[0]->getTM() == sub_tm);
}

/** @par Check point: trade_on_close=false defers execution to the next day's open (previously this
 * parameter silently failed with zero trades) */
TEST_CASE("test_MultiSystem_trade_on_close_false") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData kdata = stk.getKData(query);
    REQUIRE(kdata.size() > 2);

    auto tm = crtTM(Datetime(199001010000LL), 100000.0);
    auto ms = PF_WithoutAF(tm, SE_Fixed(), 1, "query", true, /*trade_on_close=*/false);
    REQUIRE(ms != nullptr);
    auto sys = create_alway_buy_sys();
    sys->setTO(kdata);
    ms->add(sys);
    ms->run(kdata);

    /** @arg the portfolio produces actual trades (rebalancing suggestions executed at the open
     * phase) */
    CHECK_GE(ms->getTM()->getTradeList().size(), 2);
}

/** @par Check point: suggestions on non-rebalancing days accumulate to the next rebalancing
 * day (previously sub-system suggestions were dropped on non-rebalancing days) */
TEST_CASE("test_MultiSystem_adjust_cycle_accumulate") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData kdata = stk.getKData(query);
    REQUIRE(kdata.size() > 3);

    auto ms = std::make_shared<MultiSystem>("ms");
    ms->setTM(crtTM(Datetime(199001010000LL), 100000.0));
    auto sys = create_alway_buy_sys();
    sys->setTO(kdata);
    ms->add(sys);
    ms->setSE(SE_Fixed());

    /** @arg only the 2nd bar is a rebalancing day: the 1st bar's buy suggestion accumulates and
     * executes on the 2nd bar */
    ms->setAdjustDates({kdata.getDatetimeList()[1]});
    ms->run(kdata);
    auto trades = ms->getTM()->getTradeList();
    REQUIRE_GE(trades.size(), 2);
    CHECK_EQ(trades[1].datetime, kdata.getDatetimeList()[1]);
    CHECK_EQ(trades[1].business, BUSINESS_BUY);
}

/** @par Check point: buy suggestions exceeding cash are scaled down proportionally (previously
 * over-limit orders were silently rejected by the TM) */
TEST_CASE("test_MultiSystem_buy_cash_scale") {
    Stock stk1 = getStock("sh600000");
    Stock stk2 = getStock("sz000001");
    REQUIRE(!stk1.isNull());
    REQUIRE(!stk2.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData kdata1 = stk1.getKData(query);
    KData kdata2 = stk2.getKData(query);
    REQUIRE(kdata1.size() > 0);
    REQUIRE(kdata2.size() > 0);

    auto ms = std::make_shared<MultiSystem>("ms");
    ms->setTM(crtTM(Datetime(199001010000LL), 20000.0));
    ms->getAF()->setParam<double>("fixed-amount", 20000.0);  // total demand 40000 > cash 20000
    auto sys1 = create_alway_buy_sys();
    sys1->setTO(kdata1);
    ms->add(sys1);
    auto sys2 = create_alway_buy_sys();
    sys2->setTO(kdata2);
    ms->add(sys2);
    ms->run(kdata1);

    /** @arg both instruments get positions (each can trade after proportional scaling) */
    CHECK_UNARY(ms->getTM()->have(stk1));
    CHECK_UNARY(ms->getTM()->have(stk2));
    /** @arg cash is not overdrawn */
    CHECK_GE(ms->getTM()->currentCash(), 0.0);
}

/** @par Check point: multiple close drives on the same day do not re-merge the open buffer */
TEST_CASE("test_MultiSystem_open_buffer_consume") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData kdata = stk.getKData(query);
    REQUIRE(kdata.size() > 1);

    auto ms = std::make_shared<MultiSystem>("ms");
    ms->setTM(crtTM(Datetime(199001010000LL), 100000.0));
    ms->getAF()->setParam<double>("fixed-amount", 20000.0);
    auto sys = create_alway_buy_sys();
    sys->setTO(kdata);
    ms->add(sys);
    ms->setTO(kdata);
    ms->readyForRun();

    Datetime d0 = kdata.getDatetimeList()[0];
    ms->runMomentOnOpen(d0);
    ms->runMomentOnClose(d0);
    auto pos1 = ms->getTM()->getPosition(d0, stk).number;
    REQUIRE_GT(pos1, 0.0);

    /** @arg the second close drive on the same day: the buffer is already consumed and the
     * position unchanged (before the fix, duplicated orders doubled the position) */
    ms->runMomentOnClose(d0);
    CHECK_EQ(ms->getTM()->getPosition(d0, stk).number, doctest::Approx(pos1));
}

/** @par Check point: clone copies the hierarchical path */
TEST_CASE("test_MultiSystem_clone_path") {
    auto ms = std::make_shared<MultiSystem>("ms");
    ms->setTM(crtTM(Datetime(199001010000LL), 100000.0));
    ms->add(create_alway_buy_sys());
    ms->readyForRun();
    REQUIRE_FALSE(ms->getPath().empty());

    /** @arg the aggregated system's clone preserves the path */
    auto cloned = std::dynamic_pointer_cast<MultiSystem>(ms->clone());
    REQUIRE(cloned != nullptr);
    CHECK_EQ(cloned->getPath(), ms->getPath());

    /** @arg the single-security system's clone also copies the path */
    auto sys = create_test_sys(3, 5);
    sys->setPath("I/A");
    CHECK_EQ(sys->clone()->getPath(), "I/A");
}

/** @par Check point: L2 aggregates multiple BUYs on the same instrument before conversion
 * (previously each computed its delta against the same position) */
TEST_CASE("test_AllocateFunds_L2_same_stock_aggregate") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto af = AF_EqualWeight();
    af->setMode("A");
    af->setParam<double>("fixed-amount", 5000.0);
    Datetime date(200001010000LL);
    Stock stk = getStock("sz000001");
    REQUIRE(!stk.isNull());
    auto sys1 = create_test_sys(3, 5);
    auto sys2 = create_test_sys(5, 10);
    std::unordered_map<SYSPtr, double> weights{{sys1, 0.5}, {sys2, 0.5}};

    TradeSuggestion s1;
    s1.stock = stk;
    s1.sys = sys1;
    s1.type = SuggestionType::BUY;
    s1.plan_price = 10.0;
    TradeSuggestion s2 = s1;
    s2.sys = sys2;

    /** @arg no position: aggregated target value 5000+5000=10000 → the first gets 1000 shares,
     * and the second is merged into the first as 0 */
    TradeSuggestionList suggestions{s1, s2};
    af->_toTargets(date, tm, suggestions, weights, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(1000.0));
    CHECK_EQ(suggestions[1].number, doctest::Approx(0.0));

    /** @arg already holding 1000 shares (exactly the aggregated target): no more trading
     * (before the fix, each turned into SELL 500, repeatedly closing) */
    tm->buy(date, stk, 10.0, 1000.0);
    TradeSuggestionList suggestions2{s1, s2};
    af->_toTargets(date, tm, suggestions2, weights, KQuery());
    CHECK(suggestions2[0].type == SuggestionType::BUY);
    CHECK_EQ(suggestions2[0].number, doctest::Approx(0.0));
    CHECK_EQ(suggestions2[1].number, doctest::Approx(0.0));
}

/** @par Check point: master compatibility (the hub/legacy style) -- no sub-system added explicitly:
 * the PF adopts the SE proto systems on run, builds the sub-system TO with the run query
 * automatically, and drives the cycle-type signals (SG_Cycle) on the rebalancing days */
TEST_CASE("test_MultiSystem_master_pf_compat") {
    Stock stk1 = getStock("sh600000");
    Stock stk2 = getStock("sz000001");
    REQUIRE(!stk1.isNull());
    REQUIRE(!stk2.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));

    auto build_proto = []() {
        auto proto =
          SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
        // SG_Cycle emits its buy signal only via startCycle on the rebalancing day (the master
        // Portfolio driving); buy_delay=false keeps the trade on the same close
        proto->setParam<bool>("buy_delay", false);
        return proto;
    };

    /** @arg adjust_cycle=1: adopt the protos, auto-build the TOs, and trade on every day */
    auto se = SE_Fixed();
    se->addStockList({stk1, stk2}, build_proto());
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto pf = PF_Simple(tm, se, AF_EqualWeight(), 1, "query", true);
    REQUIRE_EQ(pf->getSystemList().size(), 0);
    pf->run(query);
    REQUIRE_EQ(pf->getSystemList().size(), 2);
    CHECK_UNARY(!pf->getSystemList()[0]->getTO().empty());
    CHECK_UNARY(!pf->getSystemList()[1]->getTO().empty());
    CHECK_GE(pf->getTM()->getTradeList().size(), 2);  // one BUY per instrument

    /** @arg adjust_cycle=10: SG_Cycle must be driven only on the rebalancing day, so each
     * instrument is bought exactly once (the subsequent days hold without re-buying) */
    auto se2 = SE_Fixed();
    se2->addStockList({stk1, stk2}, build_proto());
    auto tm2 = crtTM(Datetime(200001010000LL), 100000.0);
    auto pf2 = PF_Simple(tm2, se2, AF_EqualWeight(), 10, "query", true);
    pf2->run(query);
    size_t buy_count = 0;
    for (const auto& tr : tm2->getTradeList()) {
        if (tr.business == BUSINESS_BUY) {
            buy_count++;
        }
    }
    CHECK_EQ(buy_count, 2);
}

/** @par Check point: the sub-system TO is built automatically from its instrument with the run
 * query when it was not set */
TEST_CASE("test_MultiSystem_auto_set_sub_to") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));

    auto ms = std::make_shared<MultiSystem>("ms");
    ms->setTM(crtTM(Datetime(199001010000LL), 100000.0));
    auto sys = create_alway_buy_sys();
    sys->setStock(stk);  // the instrument is set, the trading object (TO) is NOT set
    ms->add(sys);

    ms->run(stk.getKData(query));
    CHECK_EQ(sys->getTO().getQuery(), query);
    CHECK_GE(ms->getTM()->getTradeList().size(), 2);
}

/** @par Check point: mode B with an SE (the hub/legacy style) -- the quota is calibrated into the
 * sub shadow accounts before driving them (recycle the cash, reduce the over quota part, inject
 * the gap), so the sub books keep in step with the parent account and the portfolio runs fully
 * invested (the master SimplePortfolio behavior) */
TEST_CASE("test_MultiSystem_mode_b_quota_calibration") {
    Stock stk1 = getStock("sh600000");
    Stock stk2 = getStock("sz000001");
    REQUIRE(!stk1.isNull());
    REQUIRE(!stk2.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));

    auto se = SE_Fixed();
    se->addStockList({stk1, stk2}, create_alway_buy_sys());
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto pf = PF_Simple(tm, se, AF_EqualWeight(), 1, "query", true);
    pf->run(query);
    auto subs = pf->getSystemList();
    REQUIRE_EQ(subs.size(), 2);

    Datetime last_date(20000224);
    auto pf_funds = tm->getFunds(last_date, query.kType());
    REQUIRE_GT(pf_funds.total_assets(), 0.0);

    // The sum of the sub shadow accounts keeps in step with the parent account (the calibration
    // aligns every sub to its quota; only the round-lot dust may differ)
    price_t sub_sum = 0.0;
    for (auto& sub : subs) {
        auto sub_tm = sub->getTM();
        REQUIRE(sub_tm != nullptr);
        sub_sum += sub_tm->getFunds(last_date, query.kType()).total_assets();
    }
    CHECK_LT(std::abs(sub_sum - pf_funds.total_assets()), 1500.0);

    // The portfolio runs (nearly) fully invested: the idle cash stays a small part of the total
    // assets (the sub cash is recycled on every rebalancing day)
    CHECK_LT(pf_funds.cash, pf_funds.total_assets() * 0.1);
}

/** @par Check point: mode A (signal aggregation) -- the sub shadow signal cash is reset on every
 * rebalancing day, so the sub-systems can keep submitting buy intents after their first position
 * (before the fix the shadow ran out of cash and the portfolio stopped rebalancing) */
TEST_CASE("test_MultiSystem_mode_a_signal_cash_reset") {
    Stock stk1 = getStock("sh600000");
    Stock stk2 = getStock("sz000001");
    REQUIRE(!stk1.isNull());
    REQUIRE(!stk2.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));

    auto se = SE_Fixed();
    se->addStockList({stk1, stk2}, create_alway_buy_sys());
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto pf = PF_WithoutAF(tm, se, 1, "query", true);  // mode A
    REQUIRE_EQ(pf->getMode(), "A");
    pf->run(query);
    auto subs = pf->getSystemList();
    REQUIRE_EQ(subs.size(), 2);

    // The shadow is reset to the initial signal cash on every rebalancing day and the sub-system
    // rebuilds its position intent with it, so the sub book total assets stay around the init
    // cash (without the reset the shadow would only hold the first position worth of assets)
    Datetime last_date(20000224);
    for (auto& sub : subs) {
        price_t sub_total = sub->getTM()->getFunds(last_date, query.kType()).total_assets();
        CHECK_GT(sub_total, 90000.0);
    }

    // The portfolio runs (nearly) fully invested instead of leaving the parent cash idle
    auto pf_funds = tm->getFunds(last_date, query.kType());
    REQUIRE_GT(pf_funds.total_assets(), 0.0);
    CHECK_LT(pf_funds.cash, pf_funds.total_assets() * 0.1);
}

/** @} */
