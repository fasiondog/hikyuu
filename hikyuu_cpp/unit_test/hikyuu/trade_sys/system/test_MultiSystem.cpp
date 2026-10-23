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
 *       - L1 system-level allocation: equal weight / fixed weights (AF_FixedWeight /
 *                        AF_FixedWeightList subclasses) / mode B real quota (including
 *                        fixed_amount migrated from AF_FixedAmount) / max_total_position
 *                        budget cap;
 *       - L2 behavior-level conversion: mode A synthesis of the signed net target from the
 *                        sub-system intent state (target_position_ratio) and trade-the-delta /
 *                        fixed_amount / SELL-CLEAR closing the whole position / over-allocation
 *                        converted to position reduction / the short clamp without a securities-
 *                        lending facility / mode B pass-through clipping;
 *       - L3 portfolio risk control: max_single_position concentration clipping / skipped
 *                        in mode B.
 *  @note MM has been restricted to the single-system form; the portfolio-level allocation
 *        (L1/L2/L3) is migrated entirely to AF (see docs/design/pf_af_compat/design.md §5).
 */

#include "../../test_config.h"
#include <limits>
#include <unordered_map>
#include <type_traits>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/system/imp/MultiSystem.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include <hikyuu/trade_sys/portfolio/build_in.h>
#include <hikyuu/trade_sys/allocatefunds/build_in.h>
#include <hikyuu/trade_sys/selector/crt/SE_Fixed.h>
#include <hikyuu/trade_sys/selector/crt/SE_Optimal.h>
#include <hikyuu/trade_sys/selector/imp/optimal/OptimalSelectorBase.h>
#include <hikyuu/trade_sys/selector/crt/SE_Signal.h>
#if HKU_SUPPORT_SERIALIZATION
#include <fstream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#endif
#include <hikyuu/trade_sys/selector/crt/SE_Logic.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/trade_sys/signal/crt/SG_AllwaysBuy.h>
#include <hikyuu/trade_sys/signal/crt/SG_Manual.h>
#include <hikyuu/trade_sys/signal/crt/SG_Cycle.h>
#include <hikyuu/trade_sys/signal/crt/SG_Cross.h>
#include <hikyuu/trade_sys/signal/crt/SG_Logic.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/trade_sys/signal/SignalBase.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_Nothing.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedCount.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_manage/crt/TC_FixedA2017.h>
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

/** Counting selector: admits every real system on every call and counts the calls. MultiSystem
 *  consults the SE on the rebalancing days only, so the call count exposes the reb-day rhythm. */
class CountingSE : public SelectorBase {
public:
    CountingSE() : SelectorBase("CountingSE") {}

    SelectorPtr _clone() override {
        return std::make_shared<CountingSE>();
    }

    void _calculate() override {}

    SystemWeightList _getSelected(Datetime) override {
        call_count++;
        SystemWeightList ret;
        for (auto& sys : getRealSystemList()) {
            ret.emplace_back(sys, 1.0);
        }
        return ret;
    }

    int call_count{0};
};

// ============================================================================
// MultiSystem structure
// ============================================================================

/** @par Check point: run mode semantics (default A; B/b normalized to B; C/c normalized to C;
 * invalid values fall back to A) */
TEST_CASE("test_MultiSystem_mode") {
    MultiSystem ms;
    CHECK_EQ(ms.getMode(), "A");

    ms.setMode("B");
    CHECK_EQ(ms.getMode(), "B");

    ms.setMode("b");  // lowercase is also recognized as B
    CHECK_EQ(ms.getMode(), "B");

    ms.setMode("C");  // mode C: the shared account (the legacy Portfolio compatibility)
    CHECK_EQ(ms.getMode(), "C");

    ms.setMode("c");  // lowercase is also recognized as C
    CHECK_EQ(ms.getMode(), "C");

    ms.setMode("A");
    CHECK_EQ(ms.getMode(), "A");

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
    CHECK_EQ(ms.getParam<string>("axis_mode"), "kdata");

    /** @arg switchable to calendar (the injected fixed date table drives the axis) */
    ms.setAxisMode("calendar");
    CHECK_EQ(ms.getAxisMode(), "calendar");

    /** @arg an invalid value is rejected */
    CHECK_THROWS_AS(ms.setAxisMode("month"), std::exception);

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
    Datetime date(200001010000LL);

    /** @arg empty contexts returns an empty weight table (boundary: 0 sub-systems) */
    {
        SubSystemContextList empty;
        auto w = af->_allocate(date, tm, empty, KQuery(), "A");
        CHECK_EQ(w.size(), 0);
    }

    /** @arg 2 sub-systems get equal weight 1/N */
    auto sys1 = create_test_sys(3, 5);
    auto sys2 = create_test_sys(5, 10);
    SubSystemContextList contexts(2);
    contexts[0].sys = sys1;
    contexts[1].sys = sys2;
    auto w = af->_allocate(date, tm, contexts, KQuery(), "A");
    CHECK_EQ(w[sys1], doctest::Approx(0.5));
    CHECK_EQ(w[sys2], doctest::Approx(0.5));

    /** @arg mode A does not write quota (quota is produced only in mode B) */
    CHECK_EQ(contexts[0].quota, doctest::Approx(0.0));
    CHECK_EQ(contexts[1].quota, doctest::Approx(0.0));

    /** @arg a context without a system is skipped (no empty key in the weight table) */
    SubSystemContextList ctx_null(2);
    ctx_null[0].sys = nullptr;
    ctx_null[1].sys = sys1;
    auto w_null = af->_allocate(date, tm, ctx_null, KQuery(), "A");
    CHECK_EQ(w_null.size(), 1);
    CHECK_EQ(w_null[sys1], doctest::Approx(0.5));
}

/** @par Check point: L1 mode B real quota (equal weight / fixed_amount / budget cap) */
TEST_CASE("test_AllocateFunds_L1_modeB_quota") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto af = AF_EqualWeight();
    Datetime date(200001010000LL);
    auto sys1 = create_test_sys(3, 5);
    auto sys2 = create_test_sys(5, 10);

    /** @arg single sub-system equal weight: quota = weight(1.0) × parent total assets(100000) */
    SubSystemContextList contexts(1);
    contexts[0].sys = sys1;
    af->_allocate(date, tm, contexts, KQuery(), "B");
    CHECK_EQ(contexts[0].quota, doctest::Approx(100000.0));

    /** @arg fixed_amount>0: quota = the fixed amount (migrated from AF_FixedAmount) */
    af->setParam<double>("fixed_amount", 30000.0);
    af->_allocate(date, tm, contexts, KQuery(), "B");
    CHECK_EQ(contexts[0].quota, doctest::Approx(30000.0));
    af->setParam<double>("fixed_amount", 0.0);

    /** @arg max_total_position caps the quota sum (AF_FixedWeight 0.7 x2, sum 1.4 > 0.5 ->
     * scaled to 0.25 each, the sum equals 0.5 x 100000) */
    auto af_cap = AF_FixedWeight(0.7);
    af_cap->setParam<double>("max_total_position", 0.5);
    SubSystemContextList ctx3(2);
    ctx3[0].sys = sys1;
    ctx3[1].sys = sys2;
    af_cap->_allocate(date, tm, ctx3, KQuery(), "B");
    CHECK_EQ(ctx3[0].quota, doctest::Approx(25000.0));
    CHECK_EQ(ctx3[1].quota, doctest::Approx(25000.0));

    /** @arg max_total_position=0 disables the cap (sum 1.4 stays uncapped) */
    af_cap->setParam<double>("max_total_position", 0.0);
    af_cap->_allocate(date, tm, ctx3, KQuery(), "B");
    CHECK_EQ(ctx3[0].quota, doctest::Approx(70000.0));
    CHECK_EQ(ctx3[1].quota, doctest::Approx(70000.0));

    /** @arg fixed_amount>0 is bounded by the budget cap too: the fixed amount is an absolute
     * amount, so the cap truncates the number of funded sub-systems (2 x 30000 > 0.4 x 100000
     * funds only the first one, the second gets a zero quota) instead of scaling every quota */
    af_cap->setParam<double>("max_total_position", 0.4);
    af_cap->setParam<double>("fixed_amount", 30000.0);
    SubSystemContextList ctx_fa(2);
    ctx_fa[0].sys = sys1;
    ctx_fa[1].sys = sys2;
    af_cap->_allocate(date, tm, ctx_fa, KQuery(), "B");
    CHECK_EQ(ctx_fa[0].quota, doctest::Approx(30000.0));
    CHECK_EQ(ctx_fa[1].quota, doctest::Approx(0.0));

    /** @arg max_total_position=0 disables the cap: both sub-systems keep the full fixed amount */
    af_cap->setParam<double>("max_total_position", 0.0);
    af_cap->_allocate(date, tm, ctx_fa, KQuery(), "B");
    CHECK_EQ(ctx_fa[0].quota, doctest::Approx(30000.0));
    CHECK_EQ(ctx_fa[1].quota, doctest::Approx(30000.0));
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

    /** @arg AF_FixedWeight(0.3): each sub-system gets 0.3, **no normalization** */
    auto af = AF_FixedWeight(0.3);
    auto w = af->_allocate(date, tm, contexts, KQuery(), "A");
    CHECK_EQ(w[sys1], doctest::Approx(0.3));
    CHECK_EQ(w[sys2], doctest::Approx(0.3));

    /** @arg mode B: quota = 0.3 × parent total assets(100000) = 30000 */
    af->_allocate(date, tm, contexts, KQuery(), "B");
    CHECK_EQ(contexts[0].quota, doctest::Approx(30000.0));
    CHECK_EQ(contexts[1].quota, doctest::Approx(30000.0));

    /** @arg empty contexts returns an empty table (boundary: 0 sub-systems) */
    {
        SubSystemContextList empty;
        auto we = af->_allocate(date, tm, empty, KQuery(), "B");
        CHECK_EQ(we.size(), 0);
    }

    /** @arg AF_FixedWeightList takes weights in order, no normalization */
    auto afl = AF_FixedWeightList({0.4, 0.2});
    w = afl->_allocate(date, tm, contexts, KQuery(), "A");
    CHECK_EQ(w[sys1], doctest::Approx(0.4));
    CHECK_EQ(w[sys2], doctest::Approx(0.2));

    /** @arg count mismatch (3 items vs 2 sub-systems): falls back to equal weight */
    auto afl3 = AF_FixedWeightList({0.4, 0.2, 0.1});
    w = afl3->_allocate(date, tm, contexts, KQuery(), "A");
    CHECK_EQ(w[sys1], doctest::Approx(0.5));
    CHECK_EQ(w[sys2], doctest::Approx(0.5));

    /** @arg an empty weight list is rejected at construction (it cannot price anything) */
    CHECK_THROWS_AS(AF_FixedWeightList(PriceList{}), std::exception);

    /** @arg mode B with AF_FixedWeightList: quota = the given weight x parent total assets
     * (0.7/0.3 → 70000/30000) */
    auto aflb = AF_FixedWeightList({0.7, 0.3});
    aflb->_allocate(date, tm, contexts, KQuery(), "B");
    CHECK_EQ(contexts[0].quota, doctest::Approx(70000.0));
    CHECK_EQ(contexts[1].quota, doctest::Approx(30000.0));
}

/** @par Check point: L1 multi-factor score weights (AF_MultiFactor: SubSystemContext::score
 * as the weight, normalized to sum 1 by default) */
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

    /** @arg auto_adjust_weight defaults to true: the scores are the mutual ratios and are
     * normalized to sum 1 (a score sum of 1 is a no-op) */
    auto w = af->_allocate(date, tm, contexts, KQuery(), "A");
    CHECK_EQ(w[sys1], doctest::Approx(0.7));
    CHECK_EQ(w[sys2], doctest::Approx(0.3));

    /** @arg raw scores not summing to 1 (5:3) are normalized to 0.625/0.375 (full-scale) */
    contexts[0].score = 5.0;
    contexts[1].score = 3.0;
    w = af->_allocate(date, tm, contexts, KQuery(), "A");
    CHECK_EQ(w[sys1], doctest::Approx(0.625));
    CHECK_EQ(w[sys2], doctest::Approx(0.375));

    /** @arg a positive score sum below 1 (0.2/0.3) is still normalized to 0.4/0.6 (full-scale
     * allocation, not the raw 0.2/0.3) */
    contexts[0].score = 0.2;
    contexts[1].score = 0.3;
    w = af->_allocate(date, tm, contexts, KQuery(), "A");
    CHECK_EQ(w[sys1], doctest::Approx(0.4));
    CHECK_EQ(w[sys2], doctest::Approx(0.6));

    /** @arg auto_adjust_weight=false: the score itself is the final weight (no normalization) */
    af->setParam<bool>("auto_adjust_weight", false);
    contexts[0].score = 5.0;
    contexts[1].score = 3.0;
    w = af->_allocate(date, tm, contexts, KQuery(), "A");
    CHECK_EQ(w[sys1], doctest::Approx(5.0));
    CHECK_EQ(w[sys2], doctest::Approx(3.0));
    af->setParam<bool>("auto_adjust_weight", true);

    /** @arg negative scores are treated as 0 and the rest is re-normalized (0.7,-1 → 1.0/0.0) */
    contexts[0].score = 0.7;
    contexts[1].score = -1.0;
    w = af->_allocate(date, tm, contexts, KQuery(), "A");
    CHECK_EQ(w[sys1], doctest::Approx(1.0));
    CHECK_EQ(w[sys2], doctest::Approx(0.0));

    /** @arg a non-finite score (NaN / inf) is treated as 0 like the unselected */
    contexts[0].score = std::numeric_limits<double>::quiet_NaN();
    contexts[1].score = 0.3;
    w = af->_allocate(date, tm, contexts, KQuery(), "A");
    CHECK_EQ(w[sys1], doctest::Approx(0.0));
    CHECK_EQ(w[sys2], doctest::Approx(1.0));

    /** @arg an overflowing score sum (1e308 x 2) falls back to equal weight */
    contexts[0].score = 1e308;
    contexts[1].score = 1e308;
    w = af->_allocate(date, tm, contexts, KQuery(), "A");
    CHECK_EQ(w[sys1], doctest::Approx(0.5));
    CHECK_EQ(w[sys2], doctest::Approx(0.5));

    /** @arg all scores are 0 (no SE / not a rebalancing day): falls back to equal weight, to
     * avoid zero weights that cannot be allocated */
    contexts[0].score = 0.0;
    contexts[1].score = 0.0;
    w = af->_allocate(date, tm, contexts, KQuery(), "A");
    CHECK_EQ(w[sys1], doctest::Approx(0.5));
    CHECK_EQ(w[sys2], doctest::Approx(0.5));

    /** @arg mode B: quota = the normalized weight x parent total assets */
    contexts[0].score = 0.6;
    contexts[1].score = 0.4;
    af->_allocate(date, tm, contexts, KQuery(), "B");
    CHECK_EQ(contexts[0].quota, doctest::Approx(60000.0));
    CHECK_EQ(contexts[1].quota, doctest::Approx(40000.0));

    /** @arg empty contexts returns an empty table (boundary) */
    {
        SubSystemContextList empty;
        CHECK_EQ(af->_allocate(date, tm, empty, KQuery(), "B").size(), 0);
    }
}

// ============================================================================
// Portfolio allocation engine L2: behavior-level conversion (mode A ratio / mode B pass-through)
// ============================================================================

/** @par Check point: L2 mode A synthesizes the parent net target from the signed net ratio */
TEST_CASE("test_AllocateFunds_L2_modeA_net_target") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);  // no position yet: all cash, predictable
    auto af = AF_EqualWeight();
    Datetime date(200001010000LL);
    Stock stk = getStock("sz000001");
    REQUIRE(!stk.isNull());
    auto sys = create_test_sys(3, 5);
    std::unordered_map<SYSPtr, double> weights{{sys, 1.0}};

    /** @arg the signed net target ratio maps to the parent net: weight(1.0) x ratio(0.5) x
     * net(100000) = 50000 -> 5000 shares */
    TradeSuggestion s;
    s.stock = stk;
    s.sys = sys;
    s.plan_price = 10.0;
    s.target_position_ratio = 0.5;
    s.number = 100;  // the original number is ignored, the parent re-derives the net delta
    TradeSuggestionList suggestions{s};
    af->_toTargets(date, tm, suggestions, weights, KQuery(), "A");
    REQUIRE_EQ(suggestions.size(), 1);
    CHECK(suggestions[0].type == SuggestionType::BUY);
    CHECK_EQ(suggestions[0].number, doctest::Approx(5000.0));

    /** @arg a zero net target means flat: no order (the instrument drops out) */
    s.target_position_ratio = 0.0;
    suggestions.assign(1, s);
    af->_toTargets(date, tm, suggestions, weights, KQuery(), "A");
    CHECK_UNARY(suggestions.empty());

    /** @arg a non-finite net target is skipped */
    s.target_position_ratio = std::numeric_limits<double>::infinity();
    suggestions.assign(1, s);
    af->_toTargets(date, tm, suggestions, weights, KQuery(), "A");
    CHECK_UNARY(suggestions.empty());
}

/** @par Check point: L2 mode A over-allocation converted to position reduction (delta<0 ->
 * SELL) */
TEST_CASE("test_AllocateFunds_L2_overweight_to_sell") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto af = AF_EqualWeight();
    af->setParam<double>("fixed_amount",
                         5000.0);  // pin the target value (independent of the marks)
    Datetime date(200001010000LL);
    Stock stk = getStock("sz000001");
    REQUIRE(!stk.isNull());
    auto sys = create_test_sys(3, 5);
    std::unordered_map<SYSPtr, double> weights{{sys, 1.0}};
    tm->buy(date, stk, 10.0, 1000.0);  // build a position of 1000 shares (deterministic)
    REQUIRE_EQ(tm->getPosition(date, stk).number, doctest::Approx(1000.0));

    /** @arg the fixed target(500) < current(1000): a SELL of the delta */
    TradeSuggestion s;
    s.stock = stk;
    s.sys = sys;
    s.plan_price = 10.0;
    s.target_position_ratio = 0.9;  // positive => the fixed_amount target (500)
    TradeSuggestionList suggestions{s};
    af->_toTargets(date, tm, suggestions, weights, KQuery(), "A");
    REQUIRE_EQ(suggestions.size(), 1);
    CHECK(suggestions[0].type == SuggestionType::SELL);
    CHECK_EQ(suggestions[0].number, doctest::Approx(500.0));
}

/** @par Check point: L2 mode A zero net target flattens the current position */
TEST_CASE("test_AllocateFunds_L2_sell_clear") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto af = AF_EqualWeight();
    Datetime date(200001010000LL);
    Stock stk = getStock("sz000001");
    REQUIRE(!stk.isNull());
    auto sys = create_test_sys(3, 5);
    std::unordered_map<SYSPtr, double> weights{{sys, 1.0}};
    tm->buy(date, stk, 10.0, 1000.0);  // build a position of 1000 shares

    /** @arg a zero net target flattens the whole position (SELL the current 1000) */
    TradeSuggestion s;
    s.stock = stk;
    s.sys = sys;
    s.plan_price = 10.0;
    s.target_position_ratio = 0.0;
    TradeSuggestionList suggestions{s};
    af->_toTargets(date, tm, suggestions, weights, KQuery(), "A");
    REQUIRE_EQ(suggestions.size(), 1);
    CHECK(suggestions[0].type == SuggestionType::SELL);
    CHECK_EQ(suggestions[0].number, doctest::Approx(1000.0));

    /** @arg a zero target on an empty position: no order (boundary) */
    auto tm2 = crtTM(Datetime(200001010000LL), 100000.0);
    suggestions.assign(1, s);
    af->_toTargets(date, tm2, suggestions, weights, KQuery(), "A");
    CHECK_UNARY(suggestions.empty());
}

/** @par Check point: L2 fixed_amount per instrument (the sign decides the side, the value is the
 * fixed market value) */
TEST_CASE("test_AllocateFunds_L2_fixed_amount") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto af = AF_EqualWeight();
    af->setParam<double>("fixed_amount", 20000.0);
    Datetime date(200001010000LL);
    Stock stk = getStock("sz000001");
    REQUIRE(!stk.isNull());
    auto sys = create_test_sys(3, 5);
    std::unordered_map<SYSPtr, double> weights{{sys, 1.0}};

    /** @arg a positive net target maps to the fixed market value: 20000 / 10 = 2000 shares */
    TradeSuggestion s;
    s.stock = stk;
    s.sys = sys;
    s.plan_price = 10.0;
    s.target_position_ratio = 0.9;
    TradeSuggestionList suggestions{s};
    af->_toTargets(date, tm, suggestions, weights, KQuery(), "A");
    REQUIRE_EQ(suggestions.size(), 1);
    CHECK_EQ(suggestions[0].number, doctest::Approx(2000.0));
}

/** @par Check point: L2 mode B passes sub-system instructions through (the parent does no
 * conversion, only defensively clips SELL) */
TEST_CASE("test_AllocateFunds_L2_modeB_passthrough") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto af = AF_EqualWeight();
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
    af->_toTargets(date, tm, suggestions, weights, KQuery(), "B");
    CHECK_EQ(suggestions[0].number, doctest::Approx(888.0));

    /** @arg SELL clipped to not exceed the parent's current position (current=0 → clipped to 0) */
    s.type = SuggestionType::SELL;
    s.number = 1000;
    suggestions[0] = s;
    af->_toTargets(date, tm, suggestions, weights, KQuery(), "B");
    CHECK_EQ(suggestions[0].number, doctest::Approx(0.0));

    /** @arg a short-side SELL (open/increase a short) is NOT clipped by the long position
     * (current=0): the old code clamped it to 0 and silently dropped the short instruction */
    s.type = SuggestionType::SELL;
    s.short_side = true;
    s.number = 1000;
    suggestions[0] = s;
    af->_toTargets(date, tm, suggestions, weights, KQuery(), "B");
    CHECK_EQ(suggestions[0].number, doctest::Approx(1000.0));
}

/** @par Check point: L2 mode A -- a residual position below one lot is still cleared when the
 * target is flat (the below-lot skip must not leave it untradable forever) */
TEST_CASE("test_AllocateFunds_L2_below_lot_clearance") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto af = AF_EqualWeight();
    Datetime date(200001040000LL);
    Stock stk = getStock("sz000001");
    REQUIRE(!stk.isNull());
    auto sys = create_test_sys(3, 5);
    std::unordered_map<SYSPtr, double> weights{{sys, 1.0}};

    // A sub-lot residual position: buy 150 then sell 100, leaving 50 shares (below one lot)
    TradeRecord tr = tm->buy(date, stk, 10.0, 150.0);
    REQUIRE(!tr.isNull());
    tr = tm->sell(date, stk, 10.0, 100.0);
    REQUIRE(!tr.isNull());
    REQUIRE_EQ(tm->getPosition(date, stk).number, 50.0);

    TradeSuggestion s;
    s.stock = stk;
    s.sys = sys;
    s.plan_price = 10.0;

    /** @arg a below-lot nonzero target stays skipped (no trade) */
    s.target_position_ratio = 0.0005;  // far below one lot of 100 shares
    TradeSuggestionList suggestions{s};
    af->_toTargets(date, tm, suggestions, weights, KQuery(), "A");
    CHECK_UNARY(suggestions.empty());

    /** @arg a flat target emits a CLEAR of the residual 50 shares instead of skipping it */
    s.target_position_ratio = 0.0;
    suggestions.assign(1, s);
    af->_toTargets(date, tm, suggestions, weights, KQuery(), "A");
    REQUIRE_EQ(suggestions.size(), 1);
    CHECK(suggestions[0].type == SuggestionType::CLEAR);
    CHECK_UNARY(!suggestions[0].short_side);
    CHECK_EQ(suggestions[0].number, doctest::Approx(50.0));
}

// ============================================================================
// Portfolio allocation engine L3: portfolio risk-control clipping
// ============================================================================

/** @par Check point: L3 concentration risk control (max_single_position clipping / no limit /
 * mode B skips) */
TEST_CASE("test_AllocateFunds_L3_check_risk") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);  // no position yet, total_assets=100000
    auto af = AF_EqualWeight();
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

    /** @arg max_single_position=0.5: clipped to a value of 50000 = 5000 shares */
    af->setParam<double>("max_single_position", 0.5);
    TradeSuggestionList suggestions{s};
    af->_checkRisk(date, tm, suggestions, KQuery(), "A");
    CHECK_EQ(suggestions[0].number, doctest::Approx(5000.0));

    /** @arg max_single_position=1.0: takes effect as the 100% cap, target 200000 > 100000 is
     * clipped to 10000 shares */
    af->setParam<double>("max_single_position", 1.0);
    suggestions[0] = s;
    af->_checkRisk(date, tm, suggestions, KQuery(), "A");
    CHECK_EQ(suggestions[0].number, doctest::Approx(10000.0));

    /** @arg max_single_position>1.0: clamped to 1.0 */
    af->setParam<double>("max_single_position", 3.0);
    suggestions[0] = s;
    af->_checkRisk(date, tm, suggestions, KQuery(), "A");
    CHECK_EQ(suggestions[0].number, doctest::Approx(10000.0));

    /** @arg mode B: risk control is skipped (respecting sub-manager autonomy, number unchanged) */
    af->setParam<double>("max_single_position", 0.5);
    suggestions[0] = s;
    af->_checkRisk(date, tm, suggestions, KQuery(), "B");
    CHECK_EQ(suggestions[0].number, doctest::Approx(20000.0));
}

/** @par Check point: L3 clips an open short (a SELL that grows the short exposure) too */
TEST_CASE("test_AllocateFunds_L3_short_concentration") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto af = AF_EqualWeight();
    af->setParam<double>("max_single_position", 0.5);  // cap 50000
    Datetime date(200001010000LL);
    Stock stk = getStock("sz000001");
    REQUIRE(!stk.isNull());

    TradeSuggestion s;
    s.stock = stk;
    s.plan_price = 10.0;
    s.type = SuggestionType::SELL;
    s.short_side = true;  // open a short
    s.number = 20000.0;   // 200000 > cap 50000

    /** @arg an open short that exceeds the cap is clipped to 5000 shares */
    TradeSuggestionList suggestions{s};
    af->_checkRisk(date, tm, suggestions, KQuery(), "A");
    CHECK_EQ(suggestions[0].number, doctest::Approx(5000.0));

    /** @arg a cover (a BUY that reduces the short) is not clipped */
    TradeSuggestion c = s;
    c.type = SuggestionType::BUY;
    c.short_side = true;
    c.number = 20000.0;
    TradeSuggestionList suggestions2{c};
    af->_checkRisk(date, tm, suggestions2, KQuery(), "A");
    CHECK_EQ(suggestions2[0].number, doctest::Approx(20000.0));
}

/** @par Check point: L2 total budget cap (max_total_position proportional scaling) */
TEST_CASE("test_AllocateFunds_L2_total_position_cap") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto af = AF_EqualWeight();
    Datetime date(200001010000LL);
    Stock stk = getStock("sz000001");
    REQUIRE(!stk.isNull());
    auto sys = create_test_sys(3, 5);
    std::unordered_map<SYSPtr, double> weights{{sys, 1.0}};

    TradeSuggestion s;
    s.stock = stk;
    s.sys = sys;
    s.plan_price = 10.0;
    s.target_position_ratio = 1.0;

    /** @arg the aggregated gross target (2 x 100000) exceeds the default cap (100000): scaled down
     * proportionally to 10000 shares */
    TradeSuggestionList suggestions{s, s};
    af->_toTargets(date, tm, suggestions, weights, KQuery(), "A");
    REQUIRE_EQ(suggestions.size(), 1);
    CHECK_EQ(suggestions[0].number, doctest::Approx(10000.0));

    /** @arg max_total_position=0 disables the cap: the full 20000 shares */
    af->setParam<double>("max_total_position", 0.0);
    suggestions = TradeSuggestionList{s, s};
    af->_toTargets(date, tm, suggestions, weights, KQuery(), "A");
    REQUIRE_EQ(suggestions.size(), 1);
    CHECK_EQ(suggestions[0].number, doctest::Approx(20000.0));

    /** @arg within the budget: no scaling */
    af->setParam<double>("max_total_position", 1.0);
    s.target_position_ratio = 0.5;
    suggestions = TradeSuggestionList{s};
    af->_toTargets(date, tm, suggestions, weights, KQuery(), "A");
    REQUIRE_EQ(suggestions.size(), 1);
    CHECK_EQ(suggestions[0].number, doctest::Approx(5000.0));
}

/** @par Check point: L2 fixed_amount target below the current position reduces it (SELL delta) */
TEST_CASE("test_AllocateFunds_L2_fixed_amount_delta") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto af = AF_EqualWeight();
    af->setParam<double>("fixed_amount", 5000.0);
    Datetime date(200001010000LL);
    Stock stk = getStock("sz000001");
    REQUIRE(!stk.isNull());
    auto sys = create_test_sys(3, 5);
    std::unordered_map<SYSPtr, double> weights{{sys, 1.0}};
    tm->buy(date, stk, 10.0, 1000.0);  // total assets: 10000 market value + 90000 cash = 100000

    TradeSuggestion s;
    s.stock = stk;
    s.sys = sys;
    s.plan_price = 10.0;
    s.target_position_ratio = 0.9;  // positive => the fixed_amount target (5000)
    TradeSuggestionList suggestions{s};
    af->_toTargets(date, tm, suggestions, weights, KQuery(), "A");

    /** @arg the fixed target(500) < current(1000): a SELL(500) carrying the parent-account cash */
    REQUIRE_EQ(suggestions.size(), 1);
    CHECK(suggestions[0].type == SuggestionType::SELL);
    CHECK_EQ(suggestions[0].number, doctest::Approx(500.0));
    CHECK_EQ(suggestions[0].plan_cash, doctest::Approx(5000.0));
}

/** @par Check point: L2 aggregates the signed net target of the same instrument across
 * sub-systems */
TEST_CASE("test_AllocateFunds_L2_multi_contrib") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto af = AF_EqualWeight();
    af->setParam<double>("max_total_position", 0.0);  // isolate from the budget cap
    Datetime date(200001010000LL);
    Stock stk = getStock("sz000001");
    REQUIRE(!stk.isNull());
    auto sys = create_test_sys(3, 5);
    std::unordered_map<SYSPtr, double> weights{{sys, 1.0}};

    TradeSuggestion s;
    s.stock = stk;
    s.sys = sys;
    s.plan_price = 10.0;
    s.target_position_ratio = 0.25;
    TradeSuggestionList suggestions{s, s};

    /** @arg the same instrument sums the net target: 0.25 x 100000 x 2 = 50000 -> 5000 shares */
    af->_toTargets(date, tm, suggestions, weights, KQuery(), "A");
    REQUIRE_EQ(suggestions.size(), 1);
    CHECK_EQ(suggestions[0].number, doctest::Approx(5000.0));
}

/** @par Check point: L2 missing weight falls back to 1/N instead of the full position */
TEST_CASE("test_AllocateFunds_L2_missing_weight_fallback") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto af = AF_EqualWeight();
    Datetime date(200001010000LL);
    Stock stk = getStock("sz000001");
    REQUIRE(!stk.isNull());
    auto sysA = create_test_sys(3, 5);
    auto sysB = create_test_sys(5, 10);
    auto sysC = create_test_sys(7, 15);
    std::unordered_map<SYSPtr, double> weights{{sysA, 0.5}, {sysB, 0.5}};

    TradeSuggestion s;
    s.stock = stk;
    s.sys = sysC;  // not in the weight table
    s.plan_price = 10.0;
    s.target_position_ratio = 1.0;
    TradeSuggestionList suggestions{s};
    af->_toTargets(date, tm, suggestions, weights, KQuery(), "A");

    /** @arg a sub-system that got no weight is skipped: its intent never gets a fabricated
     *  equal weight, which would silently scale it beyond the allocation the AF decided */
    CHECK_UNARY(suggestions.empty());

    /** @arg an empty weight table keeps the plain conversion (weight 1.0): 100000 -> 10000 */
    s.sys = SystemPtr();
    TradeSuggestionList suggestions2{s};
    af->_toTargets(date, tm, suggestions2, AllocateFundsBase::Weights(), KQuery(), "A");
    REQUIRE_EQ(suggestions2.size(), 1);
    CHECK_EQ(suggestions2[0].number, doctest::Approx(10000.0));
}

/** @par Check point: L2 with a null tm does not crash (defensive skip) */
TEST_CASE("test_AllocateFunds_L2_null_tm") {
    auto af = AF_EqualWeight();
    auto sys = create_test_sys(3, 5);
    std::unordered_map<SYSPtr, double> weights{{sys, 1.0}};

    TradeSuggestion s;
    s.stock = getStock("sz000001");
    s.sys = sys;
    s.type = SuggestionType::BUY;
    s.plan_price = 10.0;
    s.number = 100.0;
    TradeSuggestionList suggestions{s};

    /** @arg a null tm returns without touching the suggestions */
    CHECK_NOTHROW(
      af->_toTargets(Datetime(200001010000LL), nullptr, suggestions, weights, KQuery(), "A"));
    CHECK_EQ(suggestions[0].number, doctest::Approx(100.0));
}

// ============================================================================
// adjust_mode internalization (rebalancing date expansion) and the PF/AF preset factories
// ============================================================================

/** @par Check point: the adjust_mode parameter is stored raw (normalization happens at the run-time
 * preparation) and the delay_to_trading_day switch */
TEST_CASE("test_MultiSystem_adjust_mode") {
    MultiSystem ms("ms");
    /** @arg the default adjust_mode is "query", delay_to_trading_day is true */
    CHECK_EQ(ms.getAdjustMode(), "query");
    CHECK_EQ(ms.getParam<string>("adjust_mode"), "query");
    CHECK_EQ(ms.getDelayToTradingDay(), true);

    /** @arg the raw value is kept until the run-time preparation normalizes it */
    ms.setAdjustMode("WEEK");
    CHECK_EQ(ms.getAdjustMode(), "WEEK");
    ms.setAdjustMode("Month");
    CHECK_EQ(ms.getAdjustMode(), "Month");
    ms.setAdjustMode("quarter");
    CHECK_EQ(ms.getAdjustMode(), "quarter");

    /** @arg an invalid value is rejected */
    CHECK_THROWS_AS(ms.setAdjustMode("monthly"), std::exception);
    CHECK_THROWS_AS(ms.setAdjustMode(""), std::exception);

    /** @arg delay_to_trading_day can be toggled independently */
    ms.setDelayToTradingDay(false);
    CHECK_EQ(ms.getDelayToTradingDay(), false);
    ms.setDelayToTradingDay(true);
    CHECK_EQ(ms.getDelayToTradingDay(), true);
}

/** @par Check point: the switch parameters (adjust_cycle / adjust_mode /
 * delay_to_trading_day / trade_on_close / sell_at_not_selected) keep the parameter, setter
 * and runtime state in sync */
TEST_CASE("test_MultiSystem_switch_params") {
    MultiSystem ms("ms");

    /** @arg all switches are registered as parameters with the member defaults */
    CHECK_EQ(ms.getParam<int>("adjust_cycle"), 1);
    CHECK_EQ(ms.getParam<string>("adjust_mode"), "query");
    CHECK_EQ(ms.getParam<bool>("delay_to_trading_day"), true);
    CHECK_EQ(ms.getParam<bool>("trade_on_close"), true);
    CHECK_EQ(ms.getParam<bool>("sell_at_not_selected"), false);

    /** @arg setting the parameters updates the runtime state */
    ms.setParam<int>("adjust_cycle", 5);
    CHECK_EQ(ms.getAdjustCycle(), 5);
    ms.setParam<string>("adjust_mode", "month");
    CHECK_EQ(ms.getAdjustMode(), "month");
    ms.setParam<bool>("delay_to_trading_day", false);
    CHECK_EQ(ms.getDelayToTradingDay(), false);
    ms.setParam<bool>("trade_on_close", false);
    CHECK_EQ(ms.getTradeOnClose(), false);
    ms.setParam<bool>("sell_at_not_selected", true);
    CHECK_EQ(ms.getSellAtNotSelected(), true);

    /** @arg the setters write back to the parameters */
    ms.setAdjustCycle(2);
    CHECK_EQ(ms.getParam<int>("adjust_cycle"), 2);
    ms.setAdjustMode("week");
    CHECK_EQ(ms.getParam<string>("adjust_mode"), "week");
    ms.setDelayToTradingDay(true);
    CHECK_EQ(ms.getParam<bool>("delay_to_trading_day"), true);
    ms.setTradeOnClose(true);
    CHECK_EQ(ms.getParam<bool>("trade_on_close"), true);
    ms.setSellAtNotSelected(false);
    CHECK_EQ(ms.getParam<bool>("sell_at_not_selected"), false);

    /** @arg a fully illegal cycle is rejected at set time */
    CHECK_THROWS_AS(ms.setParam<int>("adjust_cycle", -1), std::exception);

    /** @arg a non-positive cycle stays raw until the run-time preparation clamps it */
    ms.setParam<int>("adjust_cycle", 0);
    CHECK_EQ(ms.getAdjustCycle(), 0);
    CHECK_EQ(ms.getParam<int>("adjust_cycle"), 0);

    /** @arg the parameter path keeps the out-of-range policy (warned but not clamped) */
    ms.setParam<int>("adjust_cycle", 7);
    CHECK_EQ(ms.getAdjustCycle(), 7);
    ms.setParam<string>("adjust_mode", "week");
    CHECK_EQ(ms.getAdjustMode(), "week");
    CHECK_EQ(ms.getAdjustCycle(), 7);
}

/** @par Check point: the adjust_cycle validation (a negative value is rejected at set time; <=0 is
 * normalized to 1 by the run-time preparation; a value that can never reach a driven day of the
 * period is warned but kept, never clamped) */
TEST_CASE("test_MultiSystem_adjust_cycle_validation") {
    MultiSystem ms("ms");
    /** @arg the default cycle is 1 */
    CHECK_EQ(ms.getAdjustCycle(), 1);

    /** @arg a negative cycle is rejected at set time */
    CHECK_THROWS_AS(ms.setAdjustCycle(-5), std::exception);

    /** @arg 0 is accepted raw and normalized to 1 by the run-time preparation */
    ms.setAdjustCycle(0);
    CHECK_EQ(ms.getAdjustCycle(), 0);

    /** @arg a value that can never reach a driven day of the period is kept as is */
    ms.setAdjustMode("week");
    ms.setAdjustCycle(8);
    CHECK_EQ(ms.getAdjustCycle(), 8);
    CHECK_EQ(ms.getAdjustMode(), "week");
    ms.setAdjustCycle(7);
    CHECK_EQ(ms.getAdjustCycle(), 7);

    /** @arg switching to a period mode does not rewrite an out-of-range cycle */
    ms.setAdjustMode("query");
    ms.setAdjustCycle(60);
    ms.setAdjustMode("month");
    CHECK_EQ(ms.getAdjustCycle(), 60);
    CHECK_EQ(ms.getAdjustMode(), "month");

    /** @arg the factory path keeps the raw value too (normalization happens at run) */
    TMPtr tm = crtTM(Datetime(20100101), 100000.0);
    auto pf = PF_Simple(tm, SE_Fixed(), AF_EqualWeight(), 0, "month");
    CHECK_EQ(pf->getAdjustCycle(), 0);
    CHECK_EQ(pf->getAdjustMode(), "month");
    auto pf2 = PF_Simple(tm, SE_Fixed(), AF_EqualWeight(), 32, "month");
    CHECK_EQ(pf2->getAdjustCycle(), 32);
    CHECK_EQ(pf2->getAdjustMode(), "month");
}

/** @par Check point: the run-time preparation normalizes adjust_cycle <= 0 to 1 and lowercases
 * adjust_mode */
TEST_CASE("test_MultiSystem_switch_params_normalized_at_run") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 5);

    auto ms = std::make_shared<MultiSystem>("ms_norm");
    ms->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    ms->setAdjustCycle(0);
    ms->setAdjustMode("WEEK");
    auto sys = create_alway_buy_sys();
    sys->setTO(k);
    ms->add(sys);
    ms->run(k);

    /** @arg the clamped cycle and the lowercased mode are written back during readyForRun */
    CHECK_EQ(ms->getAdjustCycle(), 1);
    CHECK_EQ(ms->getAdjustMode(), "week");
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

    /** @arg quarter/1 without deferral on a cross-month axis: only the 1st day of the quarter
     * hits (Jan 1); Feb 1 is not a quarter day (Feb 2 is not the 2nd quarter day either) */
    {
        DatetimeList qaxis{Datetime(2025, 1, 1), Datetime(2025, 2, 1), Datetime(2025, 2, 2)};
        auto rq = MultiSystem::calcAdjustDates(qaxis, "quarter", 1, false);
        REQUIRE_EQ(rq.size(), 1);
        CHECK_EQ(rq[0], Datetime(2025, 1, 1));
        auto rq2 = MultiSystem::calcAdjustDates(qaxis, "quarter", 2, false);
        REQUIRE_EQ(rq2.size(), 0);
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

/** @par Check point: the PF preset factories (PF_Simple -> mode B / PF_WithoutAF -> mode C /
 * PF_SignalAggregate -> mode A, parameters mapped item by item) */
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

    /** @arg PF_Simple exposes the two switches instead of fixing them: trade_on_close and
     * sell_at_not_selected are passed through (false keeps the unselected sub-systems running) */
    auto pf_keep = PF_Simple(tm, SE_Fixed(), AF_EqualWeight(), 1, "query", true, false, false);
    CHECK_EQ(pf_keep->getMode(), "B");
    CHECK_EQ(pf_keep->getTradeOnClose(), false);
    CHECK_EQ(pf_keep->getSellAtNotSelected(), false);

    /** @arg PF_WithoutAF: mode C (the legacy shared-account Portfolio); parameters passed through
     * item by item; sys_use_self_tm is kept for the signature only and must not crash */
    auto pf2 = PF_WithoutAF(tm, SE_Fixed(), 3, "week", true, false, true, true);
    REQUIRE(pf2 != nullptr);
    CHECK_EQ(pf2->name(), "PF_WithoutAF");
    CHECK_EQ(pf2->getMode(), "C");
    CHECK_EQ(pf2->getAdjustCycle(), 3);
    CHECK_EQ(pf2->getAdjustMode(), "week");
    CHECK_EQ(pf2->getDelayToTradingDay(), true);
    CHECK_EQ(pf2->getTradeOnClose(), false);
    CHECK_EQ(pf2->getSellAtNotSelected(), true);
    REQUIRE(pf2->getAF() != nullptr);
    CHECK_EQ(pf2->getAF()->name(), "AF_EqualWeight");

    /** @arg PF_WithoutAF defaults: trade_on_close=true, sell_at_not_selected=false (the legacy
     * semantics: a sub-system that stops being admitted is not force cleared) */
    auto pf2_def = PF_WithoutAF(tm, SE_Fixed());
    CHECK_EQ(pf2_def->getMode(), "C");
    CHECK_EQ(pf2_def->getTradeOnClose(), true);
    CHECK_EQ(pf2_def->getSellAtNotSelected(), false);
    REQUIRE(pf2_def->getAF() != nullptr);
    CHECK_EQ(pf2_def->getAF()->name(), "AF_EqualWeight");

    /** @arg PF_SignalAggregate: mode A (the preset PF_WithoutAF carried before); the af and the
     * shadow signal cash are mapped item by item */
    auto pf3 = PF_SignalAggregate(tm, SE_Fixed(), AF_FixedWeight(0.3), 5, "month", false, true,
                                  true, 50000.0);
    REQUIRE(pf3 != nullptr);
    CHECK_EQ(pf3->name(), "PF_SignalAggregate");
    CHECK_EQ(pf3->getMode(), "A");
    CHECK_EQ(pf3->getAdjustCycle(), 5);
    CHECK_EQ(pf3->getAdjustMode(), "month");
    CHECK_EQ(pf3->getDelayToTradingDay(), false);
    CHECK_EQ(pf3->getTradeOnClose(), true);
    CHECK_EQ(pf3->getSellAtNotSelected(), true);
    CHECK_EQ(pf3->getSubInitCash(), 50000.0);
    REQUIRE(pf3->getAF() != nullptr);
    CHECK_EQ(pf3->getAF()->name(), "AF_FixedWeight");

    /** @arg PF_SignalAggregate defaults: equal weight af, cycle 1 / query mode / delay true /
     * trade on close / no force sell / the default signal cash */
    auto pf3_def = PF_SignalAggregate(tm, SE_Fixed());
    CHECK_EQ(pf3_def->getMode(), "A");
    CHECK_EQ(pf3_def->getAdjustCycle(), 1);
    CHECK_EQ(pf3_def->getAdjustMode(), "query");
    CHECK_EQ(pf3_def->getDelayToTradingDay(), true);
    CHECK_EQ(pf3_def->getTradeOnClose(), true);
    CHECK_EQ(pf3_def->getSellAtNotSelected(), false);
    REQUIRE(pf3_def->getAF() != nullptr);
    CHECK_EQ(pf3_def->getAF()->name(), "AF_EqualWeight");
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

    /** @arg AF_FixedAmount → fixed amount (the fixed_amount parameter passed through) */
    auto af4 = AF_FixedAmount(30000.0);
    REQUIRE(af4 != nullptr);
    CHECK_EQ(af4->name(), "AF_FixedAmount");
    CHECK_EQ(af4->getParam<double>("fixed_amount"), doctest::Approx(30000.0));

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

/** @par Check point (regression): the mode A shadow book is a pure signal source and must be reset
 * by bookkeeping, never accumulate (the old reset used a market sell that is rejected on a sub-lot
 * holding, so the cash piled up across rebalancing days and distorted the parent allocation). */
TEST_CASE("test_MultiSystem_shadow_book_not_accumulated") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData kdata = stk.getKData(query);
    REQUIRE(kdata.size() > 2);

    auto ms = std::make_shared<MultiSystem>("ms");
    ms->setTM(crtTM(Datetime(199001010000LL), 100000.0));
    auto sys = create_alway_buy_sys();
    sys->setTO(kdata);
    ms->add(sys);
    ms->setSE(SE_Fixed());
    ms->run(kdata);

    /** @arg the shadow book stays around the initial signal cash instead of piling up */
    price_t init_cash = ms->getSubInitCash();
    REQUIRE_GT(init_cash, 0.0);
    price_t sub_total = ms->getSystemList()[0]
                          ->getTM()
                          ->getFunds(kdata[kdata.size() - 1].datetime, query.kType())
                          .total_assets();
    CHECK_LT(sub_total, init_cash * 1.5);

    /** @arg an always-buy / hold sub-system produces no parent churn: INIT + the first-day buy only
     */
    CHECK_EQ(ms->getTM()->getTradeList().size(), 2);
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

/** @par Check point: mode C shares the real account of the parent (no shadow account) and
 * restores the delay mapping of the legacy Portfolio; mode A/B keep the isolated shadow account */
TEST_CASE("test_MultiSystem_mode_c_ready_for_run") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KData kdata = stk.getKData(KQuery(Datetime(19991110), Datetime(20000225)));
    REQUIRE(kdata.size() > 2);

    auto tm = crtTM(Datetime(199001010000LL), 100000.0);
    auto ms = std::make_shared<MultiSystem>("ms_c");
    ms->setTM(tm);
    ms->setMode("C");
    auto sys = create_alway_buy_sys();
    sys->setTO(kdata);
    ms->add(sys);
    ms->readyForRun();

    /** @arg the sub-system account is the real account of the parent itself (no TM_SUB) */
    CHECK(ms->getSystemList()[0]->getTM() == tm);
    CHECK(ms->getSystemList()[0]->getParam<bool>("shared_tm"));

    /** @arg trade_on_close=true (the default): the sub-system does not delay its orders */
    CHECK(!ms->getSystemList()[0]->getParam<bool>("buy_delay"));
    CHECK(!ms->getSystemList()[0]->getParam<bool>("sell_delay"));

    /** @arg trade_on_close=false: the delay mapping follows it */
    ms->setTradeOnClose(false);
    ms->readyForRun();
    CHECK(ms->getSystemList()[0]->getParam<bool>("buy_delay"));
    CHECK(ms->getSystemList()[0]->getParam<bool>("sell_delay"));

    /** @arg the running set is a runtime state: readyForRun does not build it */
    CHECK(ms->getRunningSet().empty());

    /** @arg mode A (the default) still gets an isolated shadow account */
    auto ms_a = std::make_shared<MultiSystem>("ms_a");
    auto tm_a = crtTM(Datetime(199001010000LL), 100000.0);
    ms_a->setTM(tm_a);
    auto sys_a = create_alway_buy_sys();
    sys_a->setTO(kdata);
    sys_a->setParam<bool>("buy_delay", true);
    sys_a->setParam<bool>("sell_delay", true);
    ms_a->add(sys_a);
    ms_a->readyForRun();
    CHECK(ms_a->getSystemList()[0]->getTM() != tm_a);
    CHECK(!ms_a->getSystemList()[0]->getParam<bool>("shared_tm"));
    /** @arg mode A leaves the delay parameters of the sub-systems alone */
    CHECK(ms_a->getSystemList()[0]->getParam<bool>("buy_delay"));
    CHECK(ms_a->getSystemList()[0]->getParam<bool>("sell_delay"));

    /** @arg readyForRun never fills the pools (they belong to the close stage of a run), so a
     * repeated preparation call stays idempotent; the clearing done by reset is asserted in
     * test_MultiSystem_mode_c_shared_account_and_pools, where a real run has filled the pools */
    ms->readyForRun();
    CHECK(ms->getRunningSet().empty());
    CHECK(ms->getForceSellList().empty());
}

/** @par Check point: readyForRun alone materializes the sub-systems from the SE prototypes (the
 * live trading entry calls readyForRun before any run, so an SE-only aggregate must not be rejected
 * as "no subsystem specified") */
TEST_CASE("test_MultiSystem_ready_for_run_adopt_se_protos") {
    Stock stk1 = getStock("sh600000");
    Stock stk2 = getStock("sz000001");
    REQUIRE(!stk1.isNull());
    REQUIRE(!stk2.isNull());

    auto se = SE_Fixed();
    se->addStockList({stk1, stk2}, create_alway_buy_sys());

    auto ms = std::make_shared<MultiSystem>("ms_adopt");
    ms->setTM(crtTM(Datetime(199001010000LL), 100000.0));
    ms->setSE(se);
    // No sub-system added on purpose: the SE holds the cloned prototypes per instrument
    CHECK_EQ(ms->getSystemList().size(), 0);
    ms->readyForRun();
    CHECK_EQ(ms->getSystemList().size(), 2);

    /** @arg an explicit sub-system list is left untouched (no duplicate adoption) */
    auto ms2 = std::make_shared<MultiSystem>("ms_no_adopt");
    ms2->setTM(crtTM(Datetime(199001010000LL), 100000.0));
    ms2->setSE(se);
    auto sys = create_alway_buy_sys();
    sys->setTO(stk1.getKData(KQuery(Datetime(19991110), Datetime(20000225))));
    ms2->add(sys);
    ms2->readyForRun();
    CHECK_EQ(ms2->getSystemList().size(), 1);
}

/** @par Check point: the prototypes adopted from an SE are bound to themselves, so an
 *  operator-combined selector can still resolve every running system to its operand side and
 *  select the intended instruments (SE-004) */
TEST_CASE("test_MultiSystem_combined_se_adopt") {
    Stock stk1 = getStock("sh600000");
    Stock stk2 = getStock("sz000001");
    REQUIRE(!stk1.isNull());
    REQUIRE(!stk2.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));

    auto se1 = SE_Fixed();
    se1->addStockList({stk1}, create_alway_buy_sys());
    auto se2 = SE_Fixed();
    se2->addStockList({stk2}, create_alway_buy_sys());
    auto se = se1 + se2;

    auto ms = std::make_shared<MultiSystem>("ms_combined_se");
    ms->setTM(crtTM(Datetime(199001010000LL), 100000.0));
    ms->setSE(se);
    ms->run(query);

    /** @arg the two operand prototypes are adopted as the running systems */
    CHECK_EQ(ms->getSystemList().size(), 2);

    /** @arg the combination resolved both running systems to their operand side: both are
     *  admitted instead of every one being dropped as "not bound to a prototype" */
    auto dates = StockManager::instance().getTradingCalendar(query);
    REQUIRE_UNARY(!dates.empty());
    CHECK_EQ(se->getSelected(dates.back()).size(), 2);
}

/** @par Check point: the clone of an aggregate configured with a combined SE re-adopts the
 *  prototypes of the cloned SE and keeps the identity real==proto binding, so the clone selects
 *  the same instruments as the source (guards the defect that the cloned SE carried a disjoint
 *  prototype instance set and silently selected nothing) */
TEST_CASE("test_MultiSystem_combined_se_clone") {
    Stock stk1 = getStock("sh600000");
    Stock stk2 = getStock("sz000001");
    REQUIRE(!stk1.isNull());
    REQUIRE(!stk2.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));

    auto se1 = SE_Fixed();
    se1->addStockList({stk1}, create_alway_buy_sys());
    auto se2 = SE_Fixed();
    se2->addStockList({stk2}, create_alway_buy_sys());
    auto se = se1 + se2;

    auto ms = std::make_shared<MultiSystem>("ms_combined_se");
    ms->setTM(crtTM(Datetime(199001010000LL), 100000.0));
    ms->setSE(se);
    ms->run(query);
    REQUIRE_EQ(ms->getSystemList().size(), 2);

    auto cloned = std::dynamic_pointer_cast<MultiSystem>(ms->clone());
    REQUIRE(cloned != nullptr);

    /** @arg the adopted prototypes are re-adopted from the cloned SE: the running list holds the
     *  very prototype instances of the cloned SE, no second disjoint set appears */
    REQUIRE_EQ(cloned->getSystemList().size(), 2);
    auto cloned_se_protos = cloned->getSE()->getProtoSystemList();
    REQUIRE_EQ(cloned_se_protos.size(), 2);
    CHECK(cloned->getSystemList()[0] == cloned_se_protos[0]);
    CHECK(cloned->getSystemList()[1] == cloned_se_protos[1]);

    /** @arg the clone selects the same instruments as the source */
    cloned->run(query);
    auto dates = StockManager::instance().getTradingCalendar(query);
    REQUIRE_UNARY(!dates.empty());
    CHECK_EQ(cloned->getSE()->getSelected(dates.back()).size(), 2);
}

/** @par Check point: an optimal selector returns the running systems, so every mode admits one */
TEST_CASE("test_MultiSystem_performance_optimal_se") {
    Stock stk1 = getStock("sh600000");
    Stock stk2 = getStock("sz000001");
    REQUIRE(!stk1.isNull());
    REQUIRE(!stk2.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    auto dates = StockManager::instance().getTradingCalendar(query);
    REQUIRE_GT(dates.size(), 20);

    for (const string& mode : {"A", "B", "C"}) {
        auto se = SE_PerformanceOptimal();
        se->setParam<int>("train_len", 10);
        se->setParam<int>("test_len", 5);
        se->addStockList({stk1, stk2}, create_alway_buy_sys());

        auto ms = std::make_shared<MultiSystem>("ms_optimal_" + mode);
        auto tm = crtTM(Datetime(199001010000LL), 100000.0);
        ms->setTM(tm);
        ms->setMode(mode);
        ms->setSE(se);
        if (mode != "C") {
            ms->setAF(AF_EqualWeight());
        }
        ms->run(query);

        auto sys_list = ms->getSystemList();
        REQUIRE_EQ(sys_list.size(), 2);

        /** @arg the selected instance is one of the running systems (identity contract) */
        size_t selected_count = 0;
        for (size_t i = dates.size() - 5; i < dates.size(); i++) {
            auto sw = se->getSelected(dates[i]);
            if (sw.empty()) {
                continue;
            }
            selected_count++;
            CHECK_UNARY(sw[0].sys == sys_list[0] || sw[0].sys == sys_list[1]);
        }
        CHECK_GT(selected_count, 0);

        /** @arg mode C shares the parent account, no training trade may land on it */
        if (mode == "C") {
            for (auto& sys : sys_list) {
                CHECK(sys->getTM() == tm);
                CHECK(sys->getParam<bool>("shared_tm"));
            }
            CHECK_GT(tm->getTradeList().size(), 1);

            auto* opt = dynamic_cast<OptimalSelectorBase*>(se.get());
            REQUIRE_UNARY(opt != nullptr);
            const auto& ranges = opt->getRunRanges();
            REQUIRE_UNARY(!ranges.empty());
            const Datetime init_datetime(199001010000LL);
            for (const auto& tr : tm->getTradeList()) {
                if (tr.datetime == init_datetime) {
                    continue;
                }
                bool in_test_window = false;
                for (const auto& range : ranges) {
                    if (tr.datetime >= range.run_start && tr.datetime < range.end) {
                        in_test_window = true;
                        break;
                    }
                }
                CHECK_UNARY(in_test_window);
            }
        }
    }
}

/** @par Check point: a configuration change of the outer aggregate retracts the cycle days of a
 *  nested aggregate as well, otherwise its cycle sub-signals keep buying on the stale days */
TEST_CASE("test_MultiSystem_nested_clear_cycle_dates") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));

    auto cycle_sg = SG_Cycle();
    auto cycle_sys = SYS_Simple(crtTM(Datetime(199001010000LL), 100000.0), MM_Nothing(),
                                EnvironmentPtr(), ConditionPtr(), cycle_sg);
    cycle_sys->setStock(stk);

    // The inner aggregate runs once on its own so that its cycle days are established
    auto inner = std::make_shared<MultiSystem>("inner");
    inner->setTM(crtTM(Datetime(199001010000LL), 100000.0));
    inner->setMode("A");
    inner->add(cycle_sys);
    inner->setSE(SE_Fixed());
    inner->run(query);
    REQUIRE_UNARY(!cycle_sg->getCycleDates().empty());

    auto outer = std::make_shared<MultiSystem>("outer");
    outer->setTM(crtTM(Datetime(199001010000LL), 100000.0));
    outer->setMode("A");
    outer->add(inner);
    outer->setSE(SE_Fixed());
    outer->run(query);

    /** @arg the outer reconfiguration reaches the nested cycle signal */
    outer->setAdjustDates(DatetimeList{Datetime(19991110)});
    CHECK_UNARY(cycle_sg->getCycleDates().empty());

    /** @arg the nested layer is rescheduled by the next run instead of staying silent */
    outer->run(query);
    CHECK_UNARY(!cycle_sg->getCycleDates().empty());
    CHECK_UNARY(!cycle_sg->getBuySignal().empty());
}

/** @par Check point: a nested aggregate that never runs on its own takes its cycle schedule from
 * the parent, so its snapshot-based selector still admits the sub-systems on the first run */
TEST_CASE("test_MultiSystem_nested_cycle_schedule_from_parent") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));

    auto cycle_sg = SG_Cycle();
    auto cycle_sys = SYS_Simple(crtTM(Datetime(199001010000LL), 100000.0), MM_Nothing(),
                                EnvironmentPtr(), ConditionPtr(), cycle_sg);
    cycle_sys->setStock(stk);

    // Never run on its own: the schedule can only come from the parent
    auto inner = std::make_shared<MultiSystem>("inner");
    inner->setTM(crtTM(Datetime(199001010000LL), 100000.0));
    inner->setMode("A");
    inner->add(cycle_sys);
    inner->setSE(SE_Signal());

    auto outer = std::make_shared<MultiSystem>("outer");
    outer->setTM(crtTM(Datetime(199001010000LL), 100000.0));
    outer->setMode("A");
    outer->add(inner);
    outer->setSE(SE_Fixed());
    outer->run(query);

    /** @arg the parent pushes its schedule down before the nested selector snapshots the signals */
    CHECK_UNARY(!cycle_sg->getCycleDates().empty());
    CHECK_GT(inner->getSE()->getSelected(Datetime(19991110)).size(), 0);
}

/** @par Check point: mode C converges sys_use_self_tm onto the shared account of the parent
 * (C-O3=A): the parameter is kept for the signature compatibility only, every sub-system still ends
 * up trading on the account of the parent */
TEST_CASE("test_MultiSystem_mode_c_sys_use_self_tm") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));

    auto se = SE_Fixed();
    se->addStockList({stk}, create_alway_buy_sys());

    auto tm = crtTM(Datetime(199001010000LL), 100000.0);
    auto pf = PF_WithoutAF(tm, se, 1, "query", true, true, /*sys_use_self_tm=*/true, false);
    REQUIRE(pf->getMode() == "C");
    pf->run(query);

    /** @arg the "own tm" branch is not exposed as a per-layer capability: the adopted sub-systems
     * share the real account of the parent anyway */
    CHECK_GT(pf->getSystemList().size(), 0);
    for (auto& sys : pf->getSystemList()) {
        CHECK(sys->getTM() == tm);
        CHECK(sys->getParam<bool>("shared_tm"));
    }
}

/** @par Check point: a mode C aggregate nested under a mode A aggregate keeps working - the shadow
 * account of the outer layer becomes the shared account inside, the inner trades are booked there
 * and the translated suggestions stay available for the upper layer (the nesting translation of the
 * design 9.3, which must never come out empty) */
TEST_CASE("test_MultiSystem_mode_c_nested_under_mode_a") {
    Stock stk1 = getStock("sh600000");
    Stock stk2 = getStock("sz000001");
    REQUIRE(!stk1.isNull());
    REQUIRE(!stk2.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));

    // the inner aggregate: mode C over two instruments (the sub-systems come from the SE
    // prototypes)
    auto se = SE_Fixed();
    se->addStockList({stk1, stk2}, create_alway_buy_sys());
    auto inner = PF_WithoutAF(crtTM(Datetime(199001010000LL), 100000.0), se, 1, "query", true, true,
                              false, false);
    REQUIRE(inner->getMode() == "C");

    // the outer aggregate: mode A without an SE, the inner aggregate is its only sub-system
    auto outer_tm = crtTM(Datetime(199001010000LL), 100000.0);
    auto outer = std::make_shared<MultiSystem>("outer_mode_a");
    outer->setTM(outer_tm);
    outer->setMode("A");
    outer->setAdjustCycle(1);
    outer->setTradeOnClose(true);
    outer->add(inner);
    outer->run(query);

    /** @arg the outer layer created a shadow account for the inner aggregate and the inner mode C
     * shared that very account with its own sub-systems (no third account appears in between) */
    auto inner_tm = inner->getTM();
    CHECK(inner_tm != outer_tm);
    for (auto& sys : inner->getSystemList()) {
        CHECK(sys->getTM() == inner_tm);
        CHECK(sys->getParam<bool>("shared_tm"));
    }

    /** @arg the inner aggregate traded on the shadow account and kept its running pool; the outer
     * mode A layer reads the inner's position snapshot as its aggregation source (the translated
     * suggestions are not used any more) */
    CHECK_GT(inner_tm->getTradeList().size(), 1);
    CHECK_GT(inner->getRunningSet().size(), 0);

    /** @arg the outer layer converted them into its own orders on the real account */
    CHECK_GT(outer_tm->getTradeList().size(), 1);
}

/** @par Check point: mode C trades on the shared real account of the parent; with a rebalancing
 * cycle of one day every buy happens on a day the SE admits that sub-system, and the two pools
 * stay mutually exclusive (the force-sell pool only holds what left the running pool while still
 * held) */
TEST_CASE("test_MultiSystem_mode_c_shared_account_and_pools") {
    Stock stk1 = getStock("sh600000");
    Stock stk2 = getStock("sz000001");
    REQUIRE(!stk1.isNull());
    REQUIRE(!stk2.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k1 = stk1.getKData(query);
    REQUIRE(k1.size() > 20);

    auto proto = create_test_sys(3, 5);
    auto se = SE_Signal({stk1, stk2}, proto);
    REQUIRE(se != nullptr);

    auto tm = crtTM(Datetime(199001010000LL), 100000.0);
    auto ms = std::make_shared<MultiSystem>("ms_c_pools");
    ms->setTM(tm);
    ms->setMode("C");
    ms->setSE(se);
    ms->setAdjustCycle(1);
    ms->setTradeOnClose(true);
    ms->setSellAtNotSelected(false);
    auto sys1 = create_test_sys(3, 5);
    sys1->setTO(k1);
    auto sys2 = create_test_sys(3, 5);
    sys2->setTO(stk2.getKData(query));
    ms->add(sys1);
    ms->add(sys2);
    ms->run(query);

    /** @arg the sub-system account IS the real account of the parent (shared_tm) */
    CHECK(ms->getSystemList()[0]->getTM() == tm);

    /** @arg the shared account really traded: entries and exits both exist */
    size_t buys = 0;
    size_t sells = 0;
    for (auto& t : tm->getTradeList()) {
        if (t.business == BUSINESS_BUY) {
            buys++;
        } else if (t.business == BUSINESS_SELL) {
            sells++;
        }
    }
    CHECK_GT(buys, 0);
    CHECK_GT(sells, 0);

    /** @arg the admission invariant: with adjust_cycle=1 every buy day is a day the SE admitted
     * that sub-system (mode C never trades behind the SE gate) */
    size_t checked = 0;
    for (auto& t : tm->getTradeList()) {
        if (t.business != BUSINESS_BUY) {
            continue;
        }
        bool admitted = false;
        for (auto& sw : se->getSelected(t.datetime)) {
            if (sw.sys && (sw.sys.get() == sys1.get() || sw.sys.get() == sys2.get())) {
                admitted = true;
                break;
            }
        }
        CHECK(admitted);
        checked++;
    }
    CHECK_GT(checked, 0);

    /** @arg the dual pool invariant: a sub-system sits either in the running pool or in the
     * force-sell pool (never both), and a force-sell member still holds its instrument */
    for (auto& sys : ms->getSystemList()) {
        bool in_running = (ms->getRunningSet().count(sys.get()) > 0);
        bool in_force = false;
        for (auto& held : ms->getForceSellList()) {
            if (held.get() == sys.get()) {
                in_force = true;
                break;
            }
        }
        bool both = in_running && in_force;
        CHECK(!both);
        if (in_force) {
            CHECK(tm->have(sys->getStock()));
        }
    }

    /** @arg reset drops both pools (they are runtime states) */
    ms->reset();
    CHECK(ms->getRunningSet().empty());
    CHECK(ms->getForceSellList().empty());
}

/** @par Check point: mode C with sell_at_not_selected=true liquidates the systems that the SE
 * stops admitting, so nothing is left unfollowed and the force-sell pool stays empty; with the
 * switch off a removed system that still holds is handed over to the force-sell pool (the legacy
 * behavior of leaving it unfollowed) */
TEST_CASE("test_MultiSystem_mode_c_sell_at_not_selected") {
    Stock stk1 = getStock("sh600000");
    Stock stk2 = getStock("sz000001");
    REQUIRE(!stk1.isNull());
    REQUIRE(!stk2.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));

    auto run_once = [&](bool sell_at_not_selected) {
        auto se = SE_Signal({stk1, stk2}, create_test_sys(3, 5));
        auto tm = crtTM(Datetime(199001010000LL), 100000.0);
        auto ms = std::make_shared<MultiSystem>("ms_c_sans");
        ms->setTM(tm);
        ms->setMode("C");
        ms->setSE(se);
        ms->setAdjustCycle(1);
        ms->setTradeOnClose(true);
        ms->setSellAtNotSelected(sell_at_not_selected);
        auto sys1 = create_test_sys(3, 5);
        sys1->setTO(stk1.getKData(query));
        auto sys2 = create_test_sys(3, 5);
        sys2->setTO(stk2.getKData(query));
        ms->add(sys1);
        ms->add(sys2);
        ms->run(query);
        return std::make_pair(ms, tm);
    };

    /** @arg the forced liquidation is on: nothing may be left in the force-sell pool (a removed
     * system is sold at once, so there is nothing to hand over) */
    auto on_pair = run_once(true);
    CHECK(on_pair.first->getForceSellList().empty());
    CHECK_GT(on_pair.second->getTradeList().size(), 1);

    /** @arg the forced liquidation is off: whatever still holds after leaving the running pool is
     * handed to the force-sell pool, and every such member really still holds */
    auto off_pair = run_once(false);
    for (auto& held : off_pair.first->getForceSellList()) {
        CHECK(off_pair.second->have(held->getStock()));
        CHECK(off_pair.first->getRunningSet().count(held.get()) == 0);
    }
}

/** @par Check point: delisting removes the sub-system from the mode C pools (the design 9.3
 * delisting rule): the delisted holding is force liquidated at the last bar close of the
 * instrument, the sub-system leaves both pools and is not re-admitted (its SG emits no signal past
 * its data end, so SE_Signal stops selecting it), while the other sub-systems keep trading */
TEST_CASE("test_MultiSystem_mode_c_delisted") {
    Stock alive_stk = getStock("sh600000");   // data runs through 2011-12-06
    Stock delist_stk = getStock("sz000005");  // data ends 2011-04-29: it "delists" mid-window
    REQUIRE(!alive_stk.isNull());
    REQUIRE(!delist_stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20111207));
    KData kd_alive = alive_stk.getKData(query);
    KData kd_delist = delist_stk.getKData(query);
    REQUIRE(kd_alive.size() > 100);
    REQUIRE(kd_delist.size() > 100);
    Datetime delist_day = kd_delist.getKRecord(kd_delist.size() - 1).datetime;

    // The test db marks every stock valid through the market LastDate, so sz000005's lastDatetime
    // does not reflect its shorter data end; override it (restored on exit) to simulate delisting
    Datetime original_last = delist_stk.lastDatetime();
    REQUIRE(original_last > delist_day);
    delist_stk.lastDatetime(delist_day);

    auto se = SE_Signal();
    auto tm = crtTM(Datetime(199001010000LL), 1000000.0);
    auto ms = std::make_shared<MultiSystem>("ms_c_delist");
    ms->setTM(tm);
    ms->setMode("C");
    ms->setSE(se);
    ms->setAdjustCycle(1);
    ms->setTradeOnClose(true);
    ms->setSellAtNotSelected(false);
    auto sys_delist = create_alway_buy_sys();
    sys_delist->setTO(kd_delist);
    auto sys_alive = create_alway_buy_sys();
    sys_alive->setTO(kd_alive);
    ms->add(sys_delist);
    ms->add(sys_alive);
    ms->run(query);

    /** @arg the delisted holding was force liquidated after its last trading day */
    bool delist_sell = false;
    for (auto& t : tm->getTradeList()) {
        if (t.business == BUSINESS_SELL && t.stock == delist_stk && t.datetime > delist_day) {
            delist_sell = true;
            break;
        }
    }
    CHECK(delist_sell);
    CHECK(!tm->have(delist_stk));

    /** @arg the delisted sub-system left both pools */
    CHECK(ms->getRunningSet().count(sys_delist.get()) == 0);
    for (auto& held : ms->getForceSellList()) {
        CHECK(held.get() != sys_delist.get());
    }

    /** @arg the alive sub-system kept trading after the delisting freed the cash, and stays in a
     *  pool at the end */
    bool alive_buy_after = false;
    for (auto& t : tm->getTradeList()) {
        if (t.business == BUSINESS_BUY && t.stock == alive_stk && t.datetime > delist_day) {
            alive_buy_after = true;
            break;
        }
    }
    CHECK(alive_buy_after);
    bool alive_in_pool = (ms->getRunningSet().count(sys_alive.get()) > 0);
    for (auto& held : ms->getForceSellList()) {
        if (held.get() == sys_alive.get()) {
            alive_in_pool = true;
        }
    }
    CHECK(alive_in_pool);

    delist_stk.lastDatetime(original_last);
}

/** @par Check point: the live replay shape (run day by day with reset=false, readyForRun on every
 * call) keeps the close-day counter, so with adjust_cycle=2 the SE is consulted only every second
 * driven day; reset restarts the cycle. (readyForRun used to zero the counter on every call, which
 * turned every replayed day into a rebalancing day.) */
TEST_CASE("test_MultiSystem_adjust_cycle_live_replay") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData kd = stk.getKData(query);
    REQUIRE(kd.size() > 10);
    const DatetimeList dates = kd.getDatetimeList();

    auto run_replay = [&](const string& mode) {
        auto se = std::make_shared<CountingSE>();
        auto tm = crtTM(Datetime(199001010000LL), 1000000.0);
        auto ms = std::make_shared<MultiSystem>("ms_replay_" + mode);
        ms->setTM(tm);
        ms->setMode(mode);
        ms->setSE(se);
        ms->setAdjustCycle(2);
        ms->setTradeOnClose(true);
        auto sys = create_alway_buy_sys();
        sys->setTO(kd);
        ms->add(sys);
        ms->readyForRun();
        for (auto& d : dates) {
            KData day_kd = stk.getKData(KQuery(d, d + Days(1), query.kType()));
            REQUIRE(day_kd.size() == 1);
            ms->run(day_kd, false);
        }
        return std::make_pair(ms, se);
    };

    /** @arg mode C: the SE fires on every second driven day only (ceil(n/2) calls, strictly fewer
     *  than the driven days) */
    auto c_pair = run_replay("C");
    const int expected = static_cast<int>((dates.size() + 1) / 2);
    CHECK_EQ(c_pair.second->call_count, expected);
    CHECK_LT(c_pair.second->call_count, static_cast<int>(dates.size()));

    /** @arg the sub still traded under the replay driving */
    size_t buys = 0;
    for (auto& t : c_pair.first->getTM()->getTradeList()) {
        if (t.business == BUSINESS_BUY) {
            buys++;
        }
    }
    CHECK_GT(buys, 0);

    /** @arg reset restarts the cycle: the very next driven day is a rebalancing day again */
    int before = c_pair.second->call_count;
    c_pair.first->reset();
    KData last_kd = stk.getKData(KQuery(dates.back(), dates.back() + Days(1), query.kType()));
    c_pair.first->run(last_kd, false);
    CHECK_EQ(c_pair.second->call_count, before + 1);

    /** @arg mode A: the same gate holds (the SE is consulted on the rebalancing days only) */
    auto a_pair = run_replay("A");
    CHECK_EQ(a_pair.second->call_count, expected);
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
    auto ms = PF_SignalAggregate(tm, SE_Fixed(), AF_EqualWeight(), 1, "query", true,
                                 /*trade_on_close=*/false);
    REQUIRE(ms != nullptr);
    auto sys = create_alway_buy_sys();
    sys->setTO(kdata);
    ms->add(sys);
    ms->run(kdata);

    /** @arg the portfolio produces actual trades (rebalancing suggestions executed at the open
     * phase) */
    CHECK_GE(ms->getTM()->getTradeList().size(), 2);
}

/** @par Check point: with trade_on_close=false the aggregate surfaces its next-open target orders
 * in MomentResult.pendingOnNextOpen (previously the field was always empty for the aggregate) */
TEST_CASE("test_MultiSystem_pending_on_next_open") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData kdata = stk.getKData(query);
    REQUIRE(kdata.size() > 2);

    auto ms = std::make_shared<MultiSystem>("ms");
    ms->setTM(crtTM(Datetime(199001010000LL), 100000.0));
    ms->setTradeOnClose(false);
    auto sys = create_alway_buy_sys();
    sys->setTO(kdata);
    ms->add(sys);
    ms->setTO(kdata);
    ms->readyForRun();

    Datetime d0 = kdata.getDatetimeList()[0];
    ms->runMomentOnOpen(d0);
    /** @arg the rebalancing close queues the target orders for the next open */
    MomentResult r0 = ms->runMomentOnClose(d0);
    CHECK_UNARY(!r0.pendingOnNextOpen.empty());
    /** @arg the queued orders are executed at the next open */
    ms->runMomentOnOpen(kdata.getDatetimeList()[1]);
    CHECK_UNARY(ms->getTM()->have(stk));
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

/** @par Check point: the executor uses the margin buying power when the parent supports automatic
 * cash financing, and the optional auto_checkin tops up the parent cash */
TEST_CASE("test_MultiSystem_margin_and_auto_checkin") {
    Stock stk1 = getStock("sh600000");
    Stock stk2 = getStock("sz000001");
    REQUIRE(!stk1.isNull());
    REQUIRE(!stk2.isNull());
    KQuery query(Datetime(19991110), Datetime(19991118));
    KData kdata1 = stk1.getKData(query);
    KData kdata2 = stk2.getKData(query);
    REQUIRE(kdata1.size() > 1);
    REQUIRE(kdata2.size() > 1);

    auto run_case = [&](bool margin, bool auto_checkin, bool single) {
        auto ms = std::make_shared<MultiSystem>("ms");
        auto tm = crtTM(Datetime(199001010000LL), 20000.0, TC_FixedA2017(0.0003, 5.0));
        ms->setTM(tm);
        // readyForRun propagates the MultiSystem switch to the parent TM, a value set on the TM
        // directly would be overwritten
        ms->setParam<bool>("support_borrow_cash", margin);
        ms->getAF()->setParam<double>("max_total_position", 0.0);  // no AF budget cap
        // The single-instrument case checks buying power, so the L3 cap must be off as well
        ms->getAF()->setParam<double>("max_single_position", single ? 0.0 : 1.0);
        ms->getAF()->setParam<double>("fixed_amount", single ? 25000.0 : 15000.0);
        ms->setAutoCheckin(auto_checkin);
        auto sys1 = create_alway_buy_sys();
        sys1->setTO(kdata1);
        ms->add(sys1);
        if (!single) {
            auto sys2 = create_alway_buy_sys();
            sys2->setTO(kdata2);
            ms->add(sys2);
        }
        // Only the first bar rebalances, so the accumulated trades equal this single batch (a
        // multi-day run would hide a rejected tail leg behind the next day's orders)
        ms->setAdjustDates({kdata1.getDatetimeList()[0]});
        ms->run(kdata1);
        double bought = 0.0;
        int buys = 0;
        for (const auto& tr : tm->getTradeList()) {
            if (tr.business == BUSINESS_BUY) {
                bought += tr.realPrice * tr.number * tr.stock.unit();
                ++buys;
            }
        }
        return std::make_pair(bought, buys);
    };

    /** @arg without financing or the top-up the buys are scaled to the cash */
    CHECK_LE(run_case(false, false, false).first, 20000.0 * 1.02);

    /** @arg automatic cash financing lets a single order exceed the cash: the buying power is
     *  cash / margin rate (0.6), so the 25000 target is filled instead of being scaled */
    CHECK_GT(run_case(true, false, true).first, 24000.0);

    /** @arg a multi-leg financed batch is funded leg by leg (the tail must not be rejected by the
     *  trade manager): both instruments trade */
    auto multi = run_case(true, false, false);
    CHECK_EQ(multi.second, 2);
    CHECK_GT(multi.first, 21000.0);

    /** @arg auto_checkin deposits the gap including the commission, so both legs are filled (the
     *  filled value stays below 2 x 15000 because of the board-lot rounding) */
    CHECK_GT(run_case(false, true, false).first, 26000.0);

    /** @arg financing combined with auto_checkin: the FULL gap is deposited, so every leg is
     *  cash-covered and filled without borrowing */
    auto both = run_case(true, true, false);
    CHECK_EQ(both.second, 2);
    CHECK_GT(both.first, 26000.0);
}

/** @par Check point: fixed_amount targets exceeding the total budget are truncated to whole
 * quanta (the first instrument in suggestion order takes the affordable quota, the rest are driven
 * flat) */
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
    ms->getAF()->setParam<double>("fixed_amount", 15000.0);  // 2 x 15000 > net assets ~20000
    auto sys1 = create_alway_buy_sys();
    sys1->setTO(kdata1);
    ms->add(sys1);
    auto sys2 = create_alway_buy_sys();
    sys2->setTO(kdata2);
    ms->add(sys2);
    ms->run(kdata1);

    /** @arg only one whole quota fits the budget: the first instrument is funded, the second is
     *  driven flat (the fixed amount is an absolute amount and is never scaled down) */
    CHECK_UNARY(ms->getTM()->have(stk1));
    CHECK_UNARY(!ms->getTM()->have(stk2));
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
    ms->getAF()->setParam<double>("fixed_amount", 20000.0);
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

/** @par Check point: L2 aggregates the same instrument across sub-systems before conversion */
TEST_CASE("test_AllocateFunds_L2_same_stock_aggregate") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto af = AF_EqualWeight();
    af->setParam<double>("fixed_amount", 5000.0);
    Datetime date(200001010000LL);
    Stock stk = getStock("sz000001");
    REQUIRE(!stk.isNull());
    auto sys1 = create_test_sys(3, 5);
    auto sys2 = create_test_sys(5, 10);
    std::unordered_map<SYSPtr, double> weights{{sys1, 0.5}, {sys2, 0.5}};

    TradeSuggestion s1;
    s1.stock = stk;
    s1.sys = sys1;
    s1.plan_price = 10.0;
    s1.target_position_ratio = 0.9;  // positive => the fixed_amount direction
    TradeSuggestion s2 = s1;
    s2.sys = sys2;

    /** @arg no position: the aggregate signed direction is positive, so the +/-fixed_amount target
     * is applied ONCE per instrument (not summed per contribution): 5000 -> a BUY of 500 shares */
    TradeSuggestionList suggestions{s1, s2};
    af->_toTargets(date, tm, suggestions, weights, KQuery(), "A");
    REQUIRE_EQ(suggestions.size(), 1);
    CHECK(suggestions[0].type == SuggestionType::BUY);
    CHECK_EQ(suggestions[0].number, doctest::Approx(500.0));

    /** @arg already holding 500 shares (exactly the aggregated target): no more trading */
    tm->buy(date, stk, 10.0, 500.0);
    TradeSuggestionList suggestions2{s1, s2};
    af->_toTargets(date, tm, suggestions2, weights, KQuery(), "A");
    CHECK_UNARY(suggestions2.empty());
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
        // The MultiSystem declares the rebalancing days to the sub-system signals before the run,
        // so the cycle buy comes from the single full-range calculation; buy_delay=false keeps the
        // trade on the same close
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

/** @par Check point: the cycle signals are calculated once over the whole trading object -- the
 * rebalancing days are a run-level input, they never slice the KData into per-cycle windows */
TEST_CASE("test_MultiSystem_cycle_signal_full_range") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 30);

    DatetimeList dates = k.getDatetimeList();
    PriceList fast_vals(k.size(), 0.0);
    PriceList slow_vals(k.size(), 0.3);
    for (size_t i = 0, total = k.size(); i < total; ++i) {
        fast_vals[i] = (i % 5 < 2) ? 1.0 : 0.0;
    }

    auto make_sg = [&]() {
        return SG_Cycle() | SG_Cross(PRICELIST(fast_vals, dates), PRICELIST(slow_vals, dates));
    };

    auto ms = std::make_shared<MultiSystem>("ms_cycle_full_range");
    ms->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    ms->setAdjustCycle(10);
    auto sys = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), make_sg());
    sys->setStock(stk);
    ms->add(sys);
    ms->run(k);
    REQUIRE(!sys->getTO().empty());

    /** @arg the sub signal equals a standalone calculation with the same declared days: one
     * full-range calculation, no window slicing and no prefix loss */
    auto standalone = make_sg();
    standalone->setCycleDates(sys->getSG()->getCycleDates());
    standalone->setTO(sys->getTO());
    CHECK_UNARY(sys->getSG()->getBuySignal() == standalone->getBuySignal());
    CHECK_UNARY(sys->getSG()->getSellSignal() == standalone->getSellSignal());

    /** @arg the declared days are exactly the driven axis dates hitting the close-day cycle
     * (an independent formula, not a copy of the declared list) */
    DatetimeList driven;
    for (size_t i = 0, total = k.size(); i < total; ++i) {
        if (k[i].datetime >= Datetime(200001010000LL)) {
            driven.push_back(k[i].datetime);
        }
    }
    DatetimeList expected;
    for (size_t i = 0; i < driven.size(); i += 10) {
        expected.push_back(driven[i]);
    }
    const auto& cycle_dates = sys->getSG()->getCycleDates();
    CHECK_UNARY(cycle_dates == expected);
    CHECK_UNARY(cycle_dates.size() >= 2);
}

/** @par Check point: mode C declares the cycle start on admission (the legacy Portfolio behavior),
 * so the admitted sub-system emits its cycle buy on the admission day only */
TEST_CASE("test_MultiSystem_mode_c_cycle_start_on_admission") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 5);

    auto proto = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    proto->setParam<bool>("buy_delay", false);
    auto se = SE_Fixed();
    se->addStockList({stk}, proto);

    auto ms = std::make_shared<MultiSystem>("ms_c_cycle");
    ms->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    ms->setMode("C");
    ms->setSE(se);
    ms->run(k);
    REQUIRE_EQ(ms->getSystemList().size(), 1);

    /** @arg only the admission day is declared: the running sub-system is not re-declared on the
     * following rebalancing days (unlike modes A/B which declare the whole run) */
    auto sys = ms->getSystemList()[0];
    const auto& cycle_dates = sys->getSG()->getCycleDates();
    CHECK_UNARY(cycle_dates.size() == 1);
    CHECK_UNARY(cycle_dates[0] >= Datetime(200001010000LL));
    CHECK_EQ(sys->getSG()->getBuyValue(cycle_dates[0]), 1.0);
}

/** @par Check point: SE_Signal never admits a cycle sub-system under mode C, as it snapshots the
 * buy signals before any cycle day is declared (mode B declares the days first, so it does work) */
TEST_CASE("test_MultiSystem_mode_c_cycle_sg_with_se_signal") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 5);

    auto countBuy = [](const TradeRecordList& trades) {
        size_t ret = 0;
        for (const auto& tr : trades) {
            if (tr.business == BUSINESS_BUY) {
                ++ret;
            }
        }
        return ret;
    };

    /** @arg mode C: nothing is admitted -- no cycle day, no buy signal, no buy at all */
    auto proto_c = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    proto_c->setParam<bool>("buy_delay", false);
    auto ms = std::make_shared<MultiSystem>("ms_c_cycle_se_signal");
    ms->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    ms->setMode("C");
    ms->setSE(SE_Signal({stk}, proto_c));
    ms->run(k);
    REQUIRE_EQ(ms->getSystemList().size(), 1);
    auto csg = ms->getSystemList()[0]->getSG();
    CHECK_EQ(ms->getRunningSet().size(), 0);
    CHECK_EQ(csg->getCycleDates().size(), 0);
    CHECK_EQ(csg->getBuySignal().size(), 0);
    CHECK_EQ(countBuy(ms->getTM()->getTradeList()), 0);

    /** @arg mode B declares the days before the snapshot, so the cycle buy does get traded */
    auto proto_b = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    proto_b->setParam<bool>("buy_delay", false);
    auto ms_b = std::make_shared<MultiSystem>("ms_b_cycle_se_signal");
    ms_b->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    ms_b->setMode("B");
    ms_b->setSE(SE_Signal({stk}, proto_b));
    ms_b->run(k);
    REQUIRE_EQ(ms_b->getSystemList().size(), 1);
    auto bsg = ms_b->getSystemList()[0]->getSG();
    CHECK_UNARY(!bsg->getCycleDates().empty());
    CHECK_GT(countBuy(ms_b->getTM()->getTradeList()), 0);
}

/** @par Check point: the runMoment* entry (the intraday live driving, no bulk run) declares the
 * rebalancing day at the close stage, so the cycle signals are never silently lost */
TEST_CASE("test_MultiSystem_cycle_signal_run_moment") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 6);

    auto ms = std::make_shared<MultiSystem>("ms_cycle_moment");
    ms->setTM(crtTM(k[0].datetime, 100000.0));
    ms->setAdjustCycle(3);
    auto sys = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    sys->setParam<bool>("buy_delay", false);
    sys->setStock(stk);
    ms->add(sys);
    ms->setTO(k);
    ms->readyForRun();

    /** @arg the close drive of four bars declares bar 0 and bar 3 (the days hitting the cycle) */
    for (size_t i = 0; i < 4; ++i) {
        ms->runMomentOnOpen(k[i].datetime);
        ms->runMomentOnClose(k[i].datetime);
    }
    const auto& cycle_dates = sys->getSG()->getCycleDates();
    REQUIRE_UNARY(cycle_dates.size() == 2);
    CHECK_EQ(cycle_dates[0], k[0].datetime);
    CHECK_EQ(cycle_dates[1], k[3].datetime);
    CHECK_EQ(sys->getSG()->getBuyValue(k[0].datetime), 1.0);
    CHECK_EQ(sys->getSG()->getBuyValue(k[3].datetime), 1.0);
}

/** @par Check point: a rebalancing configuration change retracts the declared cycle days */
TEST_CASE("test_MultiSystem_cycle_dates_cleared_on_config") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);

    auto ms = std::make_shared<MultiSystem>("ms_cycle_config");
    ms->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    ms->setAdjustCycle(10);
    auto sys = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    sys->setStock(stk);
    ms->add(sys);
    ms->run(k);
    CHECK_UNARY(!sys->getSG()->getCycleDates().empty());

    /** @arg a new cycle configuration is applied at the next run, where the previously declared
     * days are retracted and rebuilt */
    ms->setAdjustCycle(3);
    CHECK_UNARY(!sys->getSG()->getCycleDates().empty());

    /** @arg the next run rebuilds them with the new configuration */
    ms->run(k);
    const auto& cycle_dates = sys->getSG()->getCycleDates();
    DatetimeList driven;
    for (size_t i = 0, total = k.size(); i < total; ++i) {
        if (k[i].datetime >= Datetime(200001010000LL)) {
            driven.push_back(k[i].datetime);
        }
    }
    DatetimeList expected;
    for (size_t i = 0; i < driven.size(); i += 3) {
        expected.push_back(driven[i]);
    }
    CHECK_UNARY(cycle_dates == expected);
}

/** @par Check point: only the cycle signals consume the rebalancing days -- the plain signals are
 * not recalculated or polluted by a declaration */
TEST_CASE("test_MultiSystem_cycle_dates_not_injected_into_plain_sg") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);

    auto ms = std::make_shared<MultiSystem>("ms_cycle_plain");
    ms->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    ms->setAdjustCycle(3);
    auto sys = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_AllwaysBuy());
    sys->setStock(stk);
    ms->add(sys);
    ms->run(k);

    /** @arg the plain signal keeps an empty cycle-day list */
    CHECK_UNARY(sys->getSG()->getCycleDates().empty());
}

/** @par Check point: the cycle date is the scheduled re-entry of the continuous shadow account --
 * after the SE drops the sub-system and its shadow is cleared, the next cycle buy brings it back */
class SelectDropSecondSE : public SelectorBase {
public:
    SelectDropSecondSE() : SelectorBase("SelectDropSecondSE") {}

    SelectorPtr _clone() override {
        return std::make_shared<SelectDropSecondSE>();
    }

    void _calculate() override {}

    SystemWeightList _getSelected(Datetime) override {
        SystemWeightList ret;
        if (call_count++ != 1) {
            for (auto& sys : getRealSystemList()) {
                ret.emplace_back(sys, 1.0);
            }
        }
        return ret;
    }

    int call_count{0};
};

TEST_CASE("test_MultiSystem_cycle_reentry_after_clear") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 6);

    auto proto = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    proto->setParam<bool>("buy_delay", false);
    auto se = std::make_shared<SelectDropSecondSE>();
    se->addStockList({stk}, proto);

    auto ms = std::make_shared<MultiSystem>("ms_cycle_reentry");
    ms->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    ms->setMode("B");
    ms->setSellAtNotSelected(true);
    ms->setSE(se);
    ms->run(k);
    REQUIRE_EQ(ms->getSystemList().size(), 1);

    /** @arg the quota account is cleared on the dropped day (sell_at_not_selected frees the parent
     * cash), and the sub-system re-enters on the next cycle buy (two entries in total) */
    auto sys = ms->getSystemList()[0];
    size_t buy_count = 0;
    for (const auto& tr : sys->getTM()->getTradeList()) {
        if (tr.business == BUSINESS_BUY) {
            buy_count++;
        }
    }
    CHECK_EQ(buy_count, 2);
    CHECK_GT(sys->getTM()->getPosition(k.back().datetime, stk).number, 0.0);
}

/** @par Check point: mode B with sell_at_not_selected=false -- the unselected sub-system keeps
 * running and the two books stay matched (no shadow clear, no parent liquidation) */
TEST_CASE("test_MultiSystem_mode_b_unselected_books_match") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 6);

    auto proto = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    proto->setParam<bool>("buy_delay", false);
    auto se = std::make_shared<SelectDropSecondSE>();
    se->addStockList({stk}, proto);

    auto ms = std::make_shared<MultiSystem>("ms_mode_b_keep");
    ms->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    ms->setMode("B");  // sell_at_not_selected defaults to false
    ms->setSE(se);
    ms->run(k);
    REQUIRE_EQ(ms->getSystemList().size(), 1);

    /** @arg the shadow and the parent keep the mirrored position (one entry, never liquidated) */
    auto sys = ms->getSystemList()[0];
    size_t sub_buys = 0, sub_sells = 0, parent_buys = 0, parent_sells = 0;
    for (const auto& tr : sys->getTM()->getTradeList()) {
        if (tr.business == BUSINESS_BUY) {
            sub_buys++;
        } else if (tr.business == BUSINESS_SELL) {
            sub_sells++;
        }
    }
    for (const auto& tr : ms->getTM()->getTradeList()) {
        if (tr.business == BUSINESS_BUY) {
            parent_buys++;
        } else if (tr.business == BUSINESS_SELL) {
            parent_sells++;
        }
    }
    CHECK_EQ(sub_buys, 1);
    CHECK_EQ(sub_sells, 0);
    CHECK_EQ(parent_buys, 1);
    CHECK_EQ(parent_sells, 0);

    /** @arg the two books stay matched: the parent mirrors the shadow position */
    double sub_pos = sys->getTM()->getPosition(k.back().datetime, stk).number;
    double parent_pos = ms->getTM()->getPosition(k.back().datetime, stk).number;
    CHECK_GT(sub_pos, 0.0);
    CHECK_EQ(sub_pos, doctest::Approx(parent_pos));
}

/** @par Check point: mode B without an SE treats every sub-system as selected (the A/C convention)
 * -- the quota is allocated and the zero-cash shadow account is funded, instead of an empty run */
TEST_CASE("test_MultiSystem_mode_b_without_se") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 6);

    auto ms = std::make_shared<MultiSystem>("ms_mode_b_no_se");
    ms->setTM(crtTM(Datetime(199001010000LL), 100000.0));
    ms->setMode("B");
    ms->setAF(AF_EqualWeight());
    auto sys = create_alway_buy_sys();
    sys->setTO(k);
    ms->add(sys);
    ms->run(k);

    /** @arg the shadow account has been funded and the parent mirrors the buy (no empty run) */
    CHECK_GT(sys->getTM()->getFunds(k.back().datetime, query.kType()).net_assets(), 0.0);
    size_t parent_buys = 0;
    for (const auto& tr : ms->getTM()->getTradeList()) {
        if (tr.business == BUSINESS_BUY) {
            parent_buys++;
        }
    }
    CHECK_GT(parent_buys, 0);
    CHECK_GT(ms->getTM()->getPosition(k.back().datetime, stk).number, 0.0);
}

/** @par Check point: mode B with the default deferred trading -- the fill of the open on the drop
 * day is still mirrored, so the two books stay matched */
TEST_CASE("test_MultiSystem_mode_b_unselected_deferred_fill") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 6);

    // buy_delay/sell_delay keep their defaults (true): the entry fills at the next open
    auto proto = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    auto se = std::make_shared<SelectDropSecondSE>();
    se->addStockList({stk}, proto);

    auto ms = std::make_shared<MultiSystem>("ms_mode_b_deferred");
    ms->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    ms->setMode("B");
    ms->setSE(se);
    ms->run(k);
    REQUIRE_EQ(ms->getSystemList().size(), 1);

    /** @arg the deferred fill of the drop day is mirrored; the two books match */
    auto sys = ms->getSystemList()[0];
    double sub_pos = sys->getTM()->getPosition(k.back().datetime, stk).number;
    double parent_pos = ms->getTM()->getPosition(k.back().datetime, stk).number;
    CHECK_GT(sub_pos, 0.0);
    CHECK_EQ(sub_pos, doctest::Approx(parent_pos));
}

/** @par Check point: the mode B quota injections are capped by the remaining parent cash, so a
 * fixed_amount over-allocation cannot make the shadow buy more than the parent can fund */
TEST_CASE("test_MultiSystem_mode_b_injection_capped_by_parent_cash") {
    Stock stk1 = getStock("sh600000");
    Stock stk2 = getStock("sz000001");
    REQUIRE(!stk1.isNull());
    REQUIRE(!stk2.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk1.getKData(query);
    REQUIRE(k.size() > 6);

    auto build_proto = []() {
        auto sys = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
        sys->setParam<bool>("buy_delay", false);
        return sys;
    };
    auto se = SE_Fixed();
    se->addStockList({stk1, stk2}, build_proto());

    auto ms = std::make_shared<MultiSystem>("ms_mode_b_cap");
    ms->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    ms->setMode("B");
    ms->setSE(se);
    ms->setAF(AF_FixedAmount(100000.0));  // two quotas of 100k against 100k of parent cash
    ms->run(k);
    REQUIRE_EQ(ms->getSystemList().size(), 2);

    /** @arg each instrument's two books match and the parent never over-commits */
    for (auto& sys : ms->getSystemList()) {
        Stock stk = sys->getStock();
        double sub_pos = sys->getTM()->getPosition(k.back().datetime, stk).number;
        double parent_pos = ms->getTM()->getPosition(k.back().datetime, stk).number;
        CHECK_EQ(sub_pos, doctest::Approx(parent_pos));
    }
    CHECK_GE(ms->getTM()->cash(k.back().datetime, KQuery::DAY), 0.0);
}

/** @par Check point: the declared cycle days match the adjust_mode table (month), not the plain
 * close-day counting */
TEST_CASE("test_MultiSystem_cycle_dates_match_adjust_mode") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 30);

    auto ms = std::make_shared<MultiSystem>("ms_cycle_month");
    ms->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    ms->setAdjustMode("month");
    auto sys = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    sys->setStock(stk);
    ms->add(sys);
    ms->run(k);

    /** @arg the declared days equal the month rebalancing days of the driven axis */
    DatetimeList driven;
    for (size_t i = 0; i < k.size(); ++i) {
        if (k[i].datetime >= Datetime(200001010000LL)) {
            driven.push_back(k[i].datetime);
        }
    }
    DatetimeList expected = MultiSystem::calcAdjustDates(driven, "month", 1, true);
    CHECK_UNARY(expected.size() >= 2);
    CHECK_UNARY(sys->getSG()->getCycleDates() == expected);
}

/** @par Check point: mode A never operates the shadow account -- the SE selection only gates the
 * parent sampling, the shadow keeps running and the parent sells/re-buys on its own book */
TEST_CASE("test_MultiSystem_mode_a_shadow_independent") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 6);

    auto proto = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    proto->setParam<bool>("buy_delay", false);
    auto se = std::make_shared<SelectDropSecondSE>();
    se->addStockList({stk}, proto);

    auto ms = std::make_shared<MultiSystem>("ms_mode_a_shadow");
    ms->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    ms->setMode("A");
    ms->setSE(se);
    ms->run(k);
    REQUIRE_EQ(ms->getSystemList().size(), 1);

    /** @arg the shadow buys once, is never sold by the selection drop and keeps its holding */
    auto sys = ms->getSystemList()[0];
    size_t sub_buys = 0, sub_sells = 0;
    for (const auto& tr : sys->getTM()->getTradeList()) {
        if (tr.business == BUSINESS_BUY) {
            sub_buys++;
        } else if (tr.business == BUSINESS_SELL) {
            sub_sells++;
        }
    }
    CHECK_EQ(sub_buys, 1);
    CHECK_EQ(sub_sells, 0);
    CHECK_GT(sys->getTM()->getPosition(k.back().datetime, stk).number, 0.0);

    /** @arg the parent owns its own book: it sells on the drop and re-buys on the re-selection */
    size_t parent_buys = 0, parent_sells = 0;
    for (const auto& tr : ms->getTM()->getTradeList()) {
        if (tr.business == BUSINESS_BUY) {
            parent_buys++;
        } else if (tr.business == BUSINESS_SELL) {
            parent_sells++;
        }
    }
    CHECK_GE(parent_buys, 2);
    CHECK_GE(parent_sells, 1);
}

/** Selector that selects everything on the first rebalancing call and only the first system
 * afterwards (the partial-selection shape of the mode A orphan flattening) */
class SelectFirstOnlySE : public SelectorBase {
public:
    SelectFirstOnlySE() : SelectorBase("SelectFirstOnlySE") {}

    SelectorPtr _clone() override {
        return std::make_shared<SelectFirstOnlySE>();
    }

    void _calculate() override {}

    SystemWeightList _getSelected(Datetime) override {
        SystemWeightList ret;
        if (call_count++ == 0) {
            for (auto& sys : getRealSystemList()) {
                ret.emplace_back(sys, 1.0);
            }
        } else if (!getRealSystemList().empty()) {
            ret.emplace_back(getRealSystemList().front(), 1.0);
        }
        return ret;
    }

    int call_count{0};
};

/** @par Check point: mode A partial selection -- the instrument of the dropped sub-system is
 * flattened in the parent account while its shadow keeps holding (the parent never touches the
 * shadow) */
TEST_CASE("test_MultiSystem_mode_a_partial_selection_orphan") {
    Stock stk1 = getStock("sh600000");
    Stock stk2 = getStock("sz000001");
    REQUIRE(!stk1.isNull());
    REQUIRE(!stk2.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk1.getKData(query);
    REQUIRE(k.size() > 6);

    auto build_proto = []() {
        auto sys = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
        sys->setParam<bool>("buy_delay", false);
        return sys;
    };
    auto se = std::make_shared<SelectFirstOnlySE>();
    se->addStockList({stk1, stk2}, build_proto());

    auto ms = std::make_shared<MultiSystem>("ms_mode_a_partial");
    ms->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    ms->setMode("A");
    ms->setSE(se);
    ms->run(k);
    REQUIRE_EQ(ms->getSystemList().size(), 2);

    SYSPtr sub1, sub2;
    for (auto& s : ms->getSystemList()) {
        if (s->getStock() == stk1) {
            sub1 = s;
        } else if (s->getStock() == stk2) {
            sub2 = s;
        }
    }
    REQUIRE(sub1 != nullptr);
    REQUIRE(sub2 != nullptr);

    /** @arg both shadows entered once and keep their holdings (the selection drop never touches
     * them) */
    auto count_buys_sells = [](const SYSPtr& sys) {
        size_t buys = 0, sells = 0;
        for (const auto& tr : sys->getTM()->getTradeList()) {
            if (tr.business == BUSINESS_BUY) {
                buys++;
            } else if (tr.business == BUSINESS_SELL) {
                sells++;
            }
        }
        return std::pair<size_t, size_t>{buys, sells};
    };
    auto [b1, s1] = count_buys_sells(sub1);
    auto [b2, s2] = count_buys_sells(sub2);
    CHECK_EQ(b1, 1);
    CHECK_EQ(s1, 0);
    CHECK_EQ(b2, 1);
    CHECK_EQ(s2, 0);
    CHECK_GT(sub1->getTM()->getPosition(k.back().datetime, stk1).number, 0.0);
    CHECK_GT(sub2->getTM()->getPosition(k.back().datetime, stk2).number, 0.0);

    /** @arg the parent flattens the dropped instrument and keeps the selected one */
    CHECK_EQ(ms->getTM()->getPosition(k.back().datetime, stk2).number, 0.0);
    CHECK_GT(ms->getTM()->getPosition(k.back().datetime, stk1).number, 0.0);
}

class SelectAllSE : public SelectorBase {
public:
    explicit SelectAllSE(const SystemList& list) : SelectorBase("SelectAllSE"), m_list(list) {}

    SelectorPtr _clone() override {
        return std::make_shared<SelectAllSE>(m_list);
    }

    void _calculate() override {}

    SystemWeightList _getSelected(Datetime) override {
        SystemWeightList ret;
        for (auto& sys : m_list) {
            ret.emplace_back(sys, 1.0);
        }
        return ret;
    }

    SystemList m_list;
};

/** @par Check point: the mode C children are mode-agnostic -- a nested mode A child applies its
 * aggregate signal to the shared account and never flattens the sibling positions */
TEST_CASE("test_MultiSystem_mode_c_composite_keeps_sibling_positions") {
    Stock stk1 = getStock("sh600000");
    Stock stk2 = getStock("sz000001");
    REQUIRE(!stk1.isNull());
    REQUIRE(!stk2.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk1.getKData(query);
    REQUIRE(k.size() > 6);

    // The leaf sibling buys and holds its own instrument on the shared account
    auto leaf =
      SYS_Simple(crtTM(), MM_FixedCount(1000), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf->setStock(stk1);
    leaf->setParam<bool>("buy_delay", false);

    // The nested mode A child aggregates a different instrument
    auto inner_leaf =
      SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    inner_leaf->setStock(stk2);
    inner_leaf->setParam<bool>("buy_delay", false);
    auto inner = std::make_shared<MultiSystem>("inner_a");
    inner->setMode("A");
    inner->setSubInitCash(10000.0);
    inner->add(inner_leaf);

    auto se = std::make_shared<SelectAllSE>(SystemList{leaf, inner});

    auto ms = std::make_shared<MultiSystem>("ms_mode_c_mixed");
    ms->setTM(crtTM(Datetime(200001010000LL), 200000.0));
    ms->setMode("C");
    ms->setSE(se);
    ms->add(leaf);
    ms->add(inner);
    ms->run(k);
    REQUIRE_EQ(ms->getSystemList().size(), 2);

    /** @arg the sibling instrument is never sold by the nested aggregate */
    double sibling_sells = 0.0;
    for (const auto& tr : ms->getTM()->getTradeList()) {
        if (tr.stock == stk1 && tr.business == BUSINESS_SELL) {
            sibling_sells += tr.number;
        }
    }
    CHECK_EQ(sibling_sells, 0.0);

    /** @arg both the sibling holding and the nested aggregate's target are on the shared account */
    CHECK_GT(ms->getTM()->getPosition(k.back().datetime, stk1).number, 0.0);
    CHECK_GT(ms->getTM()->getPosition(k.back().datetime, stk2).number, 0.0);
}

class SelectFirstCallOnlySE : public SelectorBase {
public:
    explicit SelectFirstCallOnlySE(const SystemList& list)
    : SelectorBase("SelectFirstCallOnlySE"), m_list(list) {}

    SelectorPtr _clone() override {
        return std::make_shared<SelectFirstCallOnlySE>(m_list);
    }

    void _calculate() override {}

    SystemWeightList _getSelected(Datetime) override {
        SystemWeightList ret;
        if (call_count++ == 0) {
            for (auto& sys : m_list) {
                ret.emplace_back(sys, 1.0);
            }
        }
        return ret;
    }

    SystemList m_list;
    int call_count{0};
};

/** @par Check point: a mode B composite sub-system is cleared together with the parent book when
 * it is dropped and sell_at_not_selected is on */
TEST_CASE("test_MultiSystem_mode_b_composite_unselected_clear") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 6);

    auto leaf = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", false);
    auto inner = std::make_shared<MultiSystem>("inner_a_clear");
    inner->setMode("A");
    inner->setSubInitCash(100000.0);
    inner->add(leaf);

    auto se = std::make_shared<SelectFirstCallOnlySE>(SystemList{inner});

    auto ms = std::make_shared<MultiSystem>("ms_mode_b_composite");
    ms->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    ms->setMode("B");
    ms->setSE(se);
    ms->setSellAtNotSelected(true);
    ms->add(inner);
    ms->run(k);
    REQUIRE_EQ(ms->getSystemList().size(), 1);

    /** @arg the parent mirrored the entry before the drop */
    size_t parent_buys = 0;
    for (const auto& tr : ms->getTM()->getTradeList()) {
        if (tr.business == BUSINESS_BUY && tr.stock == stk) {
            parent_buys++;
        }
    }
    CHECK_EQ(parent_buys, 1);

    /** @arg the composite shadow book and the parent book are cleared together */
    CHECK_EQ(inner->getTM()->getPosition(k.back().datetime, stk).number, 0.0);
    CHECK_EQ(ms->getTM()->getPosition(k.back().datetime, stk).number, 0.0);
}

/** @par Check point: the removal of a composite from the mode C running pool clears its internal
 * ledgers (a nested mode B child with deferred mirrors), leaving both the leaf book and the
 * shared account flat */
TEST_CASE("test_MultiSystem_mode_c_removed_composite_clears_internal_books") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 6);

    auto leaf = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", false);
    auto inner = std::make_shared<MultiSystem>("inner_b_removed");
    inner->setMode("B");
    inner->setTradeOnClose(false);
    inner->add(leaf);

    auto se = std::make_shared<SelectFirstCallOnlySE>(SystemList{inner});

    auto ms = std::make_shared<MultiSystem>("ms_mode_c_removed");
    ms->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    ms->setMode("C");
    ms->setSE(se);
    ms->setSellAtNotSelected(true);
    ms->add(inner);
    ms->run(k);
    REQUIRE_EQ(ms->getSystemList().size(), 1);

    /** @arg the internal leaf ledger and the shared account are both flat after the removal */
    CHECK_EQ(leaf->getTM()->getPosition(k.back().datetime, stk).number, 0.0);
    CHECK_EQ(ms->getTM()->getPosition(k.back().datetime, stk).number, 0.0);
}

/** @par Check point: a composite above its quota is reduced pro rata and the parent mirrors the
 * sells (the quota is shrunk between the two runs) */
TEST_CASE("test_MultiSystem_mode_b_composite_over_quota_reduction") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query1(Datetime(19991110), Datetime(20000120));
    KQuery query2(Datetime(20000121), Datetime(20000225));
    KData k1 = stk.getKData(query1);
    KData k2 = stk.getKData(query2);
    REQUIRE(k1.size() > 5);
    REQUIRE(k2.size() > 5);

    auto leaf = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", false);
    auto inner = std::make_shared<MultiSystem>("inner_a_reduce");
    inner->setMode("A");
    inner->setSubInitCash(100000.0);
    inner->add(leaf);

    auto se = std::make_shared<SelectAllSE>(SystemList{inner});

    auto ms = std::make_shared<MultiSystem>("ms_mode_b_reduce");
    ms->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    ms->setMode("B");
    ms->setSE(se);
    ms->add(inner);
    ms->run(k1);

    double pos1 = inner->getTM()->getPosition(k1.back().datetime, stk).number;
    REQUIRE_GT(pos1, 0.0);

    /** @arg a shrunk quota reduces the composite holding and the parent mirrors it */
    ms->setAF(AF_FixedAmount(1000.0));
    ms->run(k2, false);
    double pos2 = inner->getTM()->getPosition(k2.back().datetime, stk).number;
    double parent2 = ms->getTM()->getPosition(k2.back().datetime, stk).number;
    CHECK_LT(pos2, pos1);
    CHECK_EQ(pos2, doctest::Approx(parent2));
}

class SelectFirstOfGivenSE : public SelectorBase {
public:
    explicit SelectFirstOfGivenSE(const SystemList& list)
    : SelectorBase("SelectFirstOfGivenSE"), m_list(list) {}

    SelectorPtr _clone() override {
        return std::make_shared<SelectFirstOfGivenSE>(m_list);
    }

    void _calculate() override {}

    SystemWeightList _getSelected(Datetime) override {
        SystemWeightList ret;
        const bool first_call = (call_count++ == 0);
        for (auto& sys : m_list) {
            ret.emplace_back(sys, 1.0);
            if (!first_call) {
                break;
            }
        }
        return ret;
    }

    SystemList m_list;
    int call_count{0};
};

/** @par Check point: with several mode B children holding the same instrument, dropping one sells
 * only its own share, the sibling book and the parent mirror stay matched */
TEST_CASE("test_MultiSystem_mode_b_overlapping_instruments_keep_sibling") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 6);

    auto build_inner = [&](const string& name) {
        auto leaf = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
        leaf->setStock(stk);
        leaf->setParam<bool>("buy_delay", false);
        auto inner = std::make_shared<MultiSystem>(name);
        inner->setMode("A");
        inner->setSubInitCash(50000.0);
        inner->add(leaf);
        return inner;
    };
    auto inner1 = build_inner("inner1");
    auto inner2 = build_inner("inner2");

    auto se = std::make_shared<SelectFirstOfGivenSE>(SystemList{inner1, inner2});

    auto ms = std::make_shared<MultiSystem>("ms_mode_b_overlap");
    ms->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    ms->setMode("B");
    ms->setSE(se);
    ms->setSellAtNotSelected(true);
    ms->add(inner1);
    ms->add(inner2);
    ms->run(k);
    REQUIRE_EQ(ms->getSystemList().size(), 2);

    /** @arg the dropped child is flat, the surviving child and the parent hold the same quantity */
    double p1 = inner1->getTM()->getPosition(k.back().datetime, stk).number;
    double p2 = inner2->getTM()->getPosition(k.back().datetime, stk).number;
    double pp = ms->getTM()->getPosition(k.back().datetime, stk).number;
    CHECK_GT(p1, 0.0);
    CHECK_EQ(p2, 0.0);
    CHECK_EQ(pp, doctest::Approx(p1));
}

/** @par Check point: the runMoment* entry (live) honours adjust_mode without a precomputed table */
TEST_CASE("test_MultiSystem_adjust_mode_run_moment") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 30);

    auto ms = std::make_shared<MultiSystem>("ms_adjust_mode_moment");
    ms->setTM(crtTM(k[0].datetime, 100000.0));
    ms->setAdjustMode("month");
    auto sys = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    sys->setParam<bool>("buy_delay", false);
    sys->setStock(stk);
    ms->add(sys);
    ms->setTO(k);
    ms->readyForRun();

    for (size_t i = 0; i < k.size(); ++i) {
        ms->runMomentOnOpen(k[i].datetime);
        ms->runMomentOnClose(k[i].datetime);
    }

    /** @arg only the first driven day of each month is declared, not every driven day */
    const auto& dates = sys->getSG()->getCycleDates();
    CHECK_GT(dates.size(), 0);
    CHECK_LT(dates.size(), 5);
}

/** @par Check point: the live single-bar replay honours adjust_mode (one rebalancing day per
 * period) */
TEST_CASE("test_MultiSystem_adjust_mode_live_replay") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData kd = stk.getKData(query);
    REQUIRE(kd.size() > 10);
    const DatetimeList dates = kd.getDatetimeList();

    auto ms = std::make_shared<MultiSystem>("ms_adjust_mode_live");
    ms->setTM(crtTM(Datetime(199001010000LL), 1000000.0));
    ms->setAdjustMode("month");
    ms->setTradeOnClose(true);
    auto sys = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    sys->setParam<bool>("buy_delay", false);
    sys->setStock(stk);
    sys->setTO(kd);
    ms->add(sys);
    ms->readyForRun();

    for (auto& d : dates) {
        KData day_kd = stk.getKData(KQuery(d, d + Days(1), query.kType()));
        REQUIRE(day_kd.size() == 1);
        ms->run(day_kd, false);
    }

    /** @arg one rebalancing day per month, not the whole single-bar replay */
    const auto& cycle_dates = sys->getSG()->getCycleDates();
    CHECK_GT(cycle_dates.size(), 0);
    CHECK_LT(cycle_dates.size(), 5);
}

class SelectKeepFirstTwoSE : public SelectorBase {
public:
    explicit SelectKeepFirstTwoSE(const SystemList& list)
    : SelectorBase("SelectKeepFirstTwoSE"), m_list(list) {}

    SelectorPtr _clone() override {
        return std::make_shared<SelectKeepFirstTwoSE>(m_list);
    }

    void _calculate() override {}

    SystemWeightList _getSelected(Datetime) override {
        SystemWeightList ret;
        if (call_count++ < 2) {
            for (auto& sys : m_list) {
                ret.emplace_back(sys, 1.0);
            }
        }
        return ret;
    }

    SystemList m_list;
    int call_count{0};
};

/** @par Check point: mode B with trade_on_close=false defers the quota calibration legs (sub and
 * parent side) to the next open, so both books share one price basis and the turnover is recorded
 */
TEST_CASE("test_MultiSystem_mode_b_trade_on_close_false_defers_calibration") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 6);

    auto leaf = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", false);
    auto se = std::make_shared<SelectKeepFirstTwoSE>(SystemList{leaf});

    auto ms = std::make_shared<MultiSystem>("ms_mode_b_defer_calib");
    ms->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    ms->setMode("B");
    ms->setSE(se);
    ms->setSellAtNotSelected(true);
    ms->setTradeOnClose(false);
    ms->add(leaf);
    ms->run(k);

    Datetime sub_sell_date, parent_sell_date;
    for (const auto& tr : leaf->getTM()->getTradeList()) {
        if (tr.business == BUSINESS_SELL && tr.stock == stk) {
            sub_sell_date = tr.datetime;
        }
    }
    for (const auto& tr : ms->getTM()->getTradeList()) {
        if (tr.business == BUSINESS_SELL && tr.stock == stk) {
            parent_sell_date = tr.datetime;
        }
    }

    /** @arg the sub and parent sells execute at the same open after the drop day */
    CHECK_UNARY(!sub_sell_date.isNull());
    CHECK_EQ(sub_sell_date, parent_sell_date);
    CHECK_GT(sub_sell_date, k[2].datetime);

    /** @arg both books end flat and the rebalancing turnover is recorded */
    CHECK_EQ(leaf->getTM()->getPosition(k.back().datetime, stk).number, 0.0);
    CHECK_EQ(ms->getTM()->getPosition(k.back().datetime, stk).number, 0.0);
    CHECK_UNARY(!ms->getAdjustTurnover().empty());
}

/** @par Check point: the mode A sampling sees the delayed intent of a nested mode C sub-layer
 * (the composite MomentResult does not carry its leaves' queued requests) */
TEST_CASE("test_MultiSystem_mode_a_sees_nested_c_pending") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 10);

    auto leaf = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", true);  // the entry queues to the next open
    auto inner = std::make_shared<MultiSystem>("inner_c_pending");
    inner->setMode("C");
    inner->setSE(std::make_shared<SelectAllSE>(SystemList{leaf}));
    inner->add(leaf);

    auto se = std::make_shared<SelectAllSE>(SystemList{inner});
    auto ms = std::make_shared<MultiSystem>("outer_a_pending");
    ms->setTM(crtTM(k[0].datetime, 100000.0));
    ms->setMode("A");
    ms->setAdjustCycle(5);
    ms->setSE(se);
    ms->add(inner);
    ms->run(k);

    Datetime first_buy;
    for (const auto& tr : ms->getTM()->getTradeList()) {
        if (tr.business == BUSINESS_BUY) {
            first_buy = tr.datetime;
            break;
        }
    }

    /** @arg the parent buys on the first rebalancing day from the pending intent */
    CHECK_UNARY(!first_buy.isNull());
    CHECK_EQ(first_buy, k[0].datetime);
    CHECK_GT(ms->getTM()->getPosition(k.back().datetime, stk).number, 0.0);
}

/** @par Check point: a nested aggregate / runMoment* entry expands the adjust_mode table, so the
 * inner rebalancing is not degraded to every driven day */
TEST_CASE("test_MultiSystem_nested_adjust_mode") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 30);

    auto leaf = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf->setStock(stk);

    auto inner = std::make_shared<MultiSystem>("inner_b");
    inner->setMode("B");
    inner->setAdjustMode("month");
    inner->add(leaf);

    auto ms = std::make_shared<MultiSystem>("outer_a");
    ms->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    ms->setMode("A");
    ms->add(inner);
    ms->run(k);

    /** @arg only the month rebalancing days are declared, not every driven day */
    const auto& dates = leaf->getSG()->getCycleDates();
    CHECK_GT(dates.size(), 0);
    CHECK_LT(dates.size(), 5);
}

/** @par Check point: mode C declares the cycle days on admission -- a rebalancing configuration
 * change must not retract them (a running sub-system never re-declares) */
TEST_CASE("test_MultiSystem_mode_c_config_keeps_cycle_dates") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 6);

    auto proto = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    proto->setParam<bool>("buy_delay", false);
    auto se = SE_Fixed();
    se->addStockList({stk}, proto);

    auto ms = std::make_shared<MultiSystem>("ms_mode_c_cfg");
    ms->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    ms->setMode("C");
    ms->setSE(se);
    ms->run(k);
    REQUIRE_EQ(ms->getSystemList().size(), 1);

    auto sys = ms->getSystemList()[0];
    DatetimeList dates = sys->getSG()->getCycleDates();
    REQUIRE_UNARY(dates.size() == 1);

    /** @arg the admission-declared day survives a rebalancing configuration change */
    ms->setAdjustCycle(5);
    CHECK_UNARY(sys->getSG()->getCycleDates() == dates);
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

/** @par Check point: mode B adjust_running_sys -- true force-reduces the over-quota running
 * sub-system back to its quota on the later rebalancing days; false leaves the running
 * sub-systems untouched (the master _adjust_without_running semantics).
 *  @note A constant-weight AF recomputes the quota from the current parent total assets, so the
 * sub value and the quota grow in the same proportion and never diverge (the reduction only fires
 * on real weight/fund shrink). The test AF shifts the weights across periods to force a
 * deterministic over-quota state on the second rebalancing day. */
namespace {
// The test AF: mode B, the first allocation uses the weights {0.9, 0.1}, every later allocation
// uses {0.4, 0.6}, so the first sub-system becomes over quota on the second rebalancing day
class TestShiftWeightAF : public AllocateFundsBase {
    ALLOCATE_FUNDS_IMP(TestShiftWeightAF)
    ALLOCATE_FUNDS_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    TestShiftWeightAF() : AllocateFundsBase("TestShiftWeightAF") {}

    Weights _allocate(const Datetime& date, const TradeManagerPtr& tm,
                      SubSystemContextList& contexts, const KQuery& query,
                      const string& mode) override {
        std::vector<double> weights(contexts.size(), 0.0);
        for (size_t i = 0; i < contexts.size(); ++i) {
            if (m_calls == 0) {
                weights[i] = (i == 0) ? 0.9 : 0.1;
            } else {
                weights[i] = (i == 0) ? 0.4 : 0.6;
            }
        }
        m_calls++;
        return _applyWeights(date, tm, contexts, query, weights, mode);
    }

    void _reset() override {
        m_calls = 0;
    }

private:
    int m_calls{0};
};
}  // namespace

TEST_CASE("test_MultiSystem_mode_b_adjust_running_sys") {
    Stock stk1 = getStock("sh600000");
    Stock stk2 = getStock("sz000001");
    REQUIRE(!stk1.isNull());
    REQUIRE(!stk2.isNull());
    KQuery query(Datetime(19991208), Datetime(20000324));
    KData kd1 = stk1.getKData(query);
    KData kd2 = stk2.getKData(query);
    REQUIRE(kd1.size() > 6);
    REQUIRE(kd2.size() > 6);
    const DatetimeList dates = kd1.getDatetimeList();

    auto count_sells = [](const TMPtr& tm) {
        size_t n = 0;
        for (const auto& tr : tm->getTradeList()) {
            if (tr.business == BUSINESS_SELL) {
                n++;
            }
        }
        return n;
    };

    // The SELL records executed on the very first trade day (the initial allocation day)
    auto first_day_sells = [](const TMPtr& tm) {
        Datetime first;
        for (const auto& tr : tm->getTradeList()) {
            if (first.isNull() || tr.datetime < first) {
                first = tr.datetime;
            }
        }
        size_t n = 0;
        for (const auto& tr : tm->getTradeList()) {
            if (tr.business == BUSINESS_SELL && tr.datetime == first) {
                n++;
            }
        }
        return n;
    };

    /** @arg adjust_running_sys=true: the first (initial allocation) day has no SELL (the subs are
     * only injected and buy); a later rebalancing day force-reduces the over-quota running
     * sub-system, so SELL records exist beyond the first day */
    auto run_true = [&]() {
        auto se = SE_Fixed();
        se->addStockList({stk1, stk2}, create_alway_buy_sys());
        auto tm = crtTM(Datetime(200001010000LL), 100000.0);
        auto af = std::make_shared<TestShiftWeightAF>();
        af->setParam<bool>("adjust_running_sys", true);
        auto pf = PF_Simple(tm, se, af, 2, "query", true);
        pf->run(query);
        return std::make_pair(pf, tm);
    };
    auto true_pair = run_true();
    CHECK_EQ(first_day_sells(true_pair.second), 0);
    CHECK_GT(count_sells(true_pair.second), 0);

    /** @arg the over-quota running sub-system (index 0, funded with the {0.9} first-period weight)
     * is force-reduced back towards its later {0.4} quota */
    auto subs = true_pair.first->getSystemList();
    REQUIRE_EQ(subs.size(), 2);
    Datetime last_date = true_pair.second->lastDatetime();
    price_t parent_net = true_pair.second->getFunds(last_date, query.kType()).net_assets();
    REQUIRE_GT(parent_net, 0.0);
    price_t sub0 = subs[0]->getTM()->getFunds(last_date, query.kType()).net_assets();
    CHECK_LT(sub0, 0.5 * parent_net);

    /** @arg adjust_running_sys=false: sysA runs first (funded in a previous period) while sysB is
     * added later on a non-rebalancing day; the running sub-system keeps its cash and position and
     * the new sub-system (whose net assets were zero at admission = the boundary of the "already
     * running" predicate) receives the remaining parent cash */
    auto run_false = [&]() {
        auto se = std::make_shared<CountingSE>();
        auto tm = crtTM(Datetime(199001010000LL), 100000.0);
        auto ms = std::make_shared<MultiSystem>("ms_no_adjust_running");
        ms->setTM(tm);
        ms->setMode("B");
        ms->setSE(se);
        ms->getAF()->setParam<double>("fixed_amount", 30000.0);
        ms->getAF()->setParam<bool>("adjust_running_sys", false);
        ms->setAdjustCycle(2);
        ms->setTradeOnClose(true);
        auto sysA = create_alway_buy_sys();
        sysA->setTO(kd1);
        ms->add(sysA);
        auto sysB = create_alway_buy_sys();
        sysB->setTO(kd2);

        price_t a_cash_before = 0.0;
        double a_num_before = 0.0;
        const size_t add_index =
          3;  // a non-rebalancing day (adjust_cycle=2): the next reb day is 4
        for (size_t i = 0; i < dates.size(); ++i) {
            if (i == add_index) {
                a_cash_before = sysA->getTM()->currentCash();
                a_num_before = sysA->getTM()->getPosition(dates[i], stk1).number;
                ms->add(sysB);
            }
            KData day_kd = stk1.getKData(KQuery(dates[i], dates[i] + Days(1), query.kType()));
            ms->run(day_kd, false);
        }
        CHECK_EQ(sysA->getTM()->currentCash(), doctest::Approx(a_cash_before));
        CHECK_EQ(sysA->getTM()->getPosition(tm->lastDatetime(), stk1).number,
                 doctest::Approx(a_num_before));
        CHECK_GT(sysB->getTM()->getFunds(tm->lastDatetime(), query.kType()).net_assets(), 0.0);
    };
    run_false();
}

/** @par Check point: mode B mirrors the sub-system trades on every bar (continuous mirror), so the
 * parent trades on the non-rebalancing days as well instead of only on the rebalancing days */
TEST_CASE("test_MultiSystem_mode_b_continuous_mirror") {
    Stock stk1 = getStock("sh600000");
    Stock stk2 = getStock("sz000001");
    REQUIRE(!stk1.isNull());
    REQUIRE(!stk2.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData kd1 = stk1.getKData(query);
    KData kd2 = stk2.getKData(query);
    REQUIRE(kd1.size() > 6);
    const DatetimeList dates = kd1.getDatetimeList();

    // The subs buy a fixed lot on every bar (as long as cash allows), so they keep trading on the
    // non-rebalancing days and the mirrored parent book does too
    auto mk_sys = [&](const KData& kd) {
        auto sys = SYS_Simple(crtTM(), MM_FixedCount(100), EnvironmentPtr(), ConditionPtr(),
                              SG_AllwaysBuy());
        sys->setParam<bool>("buy_delay", false);
        sys->setParam<bool>("sell_delay", false);
        sys->setTO(kd);
        return sys;
    };

    // Drive day by day with adjust_cycle=2: the close-day index equals the driven-day index, so the
    // even indices are the rebalancing days and the odd ones are not
    auto se = std::make_shared<CountingSE>();
    auto tm = crtTM(Datetime(199001010000LL), 100000.0);
    auto ms = std::make_shared<MultiSystem>("ms_continuous_mirror");
    ms->setTM(tm);
    ms->setMode("B");
    ms->setSE(se);
    ms->setAdjustCycle(2);
    ms->setTradeOnClose(true);
    ms->add(mk_sys(kd1));
    ms->add(mk_sys(kd2));
    for (const auto& d : dates) {
        KData day_kd = stk1.getKData(KQuery(d, d + Days(1), query.kType()));
        ms->run(day_kd, false);
    }

    size_t matched = 0;
    bool trade_on_non_reb = false;
    for (const auto& tr : tm->getTradeList()) {
        for (size_t i = 0; i < dates.size(); ++i) {
            if (dates[i] == tr.datetime) {
                matched++;
                if (i % 2 != 0) {
                    trade_on_non_reb = true;
                }
                break;
            }
        }
    }

    /** @arg there is at least one trade on a NON-rebalancing day (an odd index with
     * adjust_cycle=2); a reb-day-only (batched) engine would never produce one */
    CHECK_GT(matched, 0);
    CHECK_UNARY(trade_on_non_reb);
}

// Scripted short signals: one sell signal (open a short) on open_date, one buy signal (cover the
// short) on cover_date
class ScriptedShortSG : public SignalBase {
public:
    ScriptedShortSG(Datetime open_date, Datetime cover_date)
    : SignalBase("ScriptedShortSG"), m_open_date(open_date), m_cover_date(cover_date) {}

    SignalPtr _clone() override {
        return make_shared<ScriptedShortSG>(m_open_date, m_cover_date);
    }

    void _calculate(const KData& kdata) override {
        if (kdata.getPos(m_open_date) != Null<size_t>()) {
            _addSellSignal(m_open_date);
        }
        if (kdata.getPos(m_cover_date) != Null<size_t>()) {
            _addBuySignal(m_cover_date);
        }
    }

private:
    Datetime m_open_date;
    Datetime m_cover_date;
};

// Scripted long signals: one buy signal on buy_date, one sell signal on sell_date
class ScriptedBuySellSG : public SignalBase {
public:
    ScriptedBuySellSG(Datetime buy_date, Datetime sell_date)
    : SignalBase("ScriptedBuySellSG"), m_buy_date(buy_date), m_sell_date(sell_date) {}

    SignalPtr _clone() override {
        return make_shared<ScriptedBuySellSG>(m_buy_date, m_sell_date);
    }

    void _calculate(const KData& kdata) override {
        if (kdata.getPos(m_buy_date) != Null<size_t>()) {
            _addBuySignal(m_buy_date);
        }
        if (kdata.getPos(m_sell_date) != Null<size_t>()) {
            _addSellSignal(m_sell_date);
        }
    }

private:
    Datetime m_buy_date;
    Datetime m_sell_date;
};

/** @par Check point: a CLEAR leg only clears the attributable part -- the sibling child's
 * same-batch buy (and holding) must survive the netting */
TEST_CASE("test_MultiSystem_mode_b_netting_clear_keeps_sibling") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 6);

    auto make_inner = [&](const string& name, const SGPtr& sg) {
        auto leaf = SYS_Simple(crtTM(), MM_FixedCount(100), EnvironmentPtr(), ConditionPtr(), sg);
        leaf->setStock(stk);
        leaf->setParam<bool>("buy_delay", false);
        leaf->setParam<bool>("sell_delay", false);
        auto inner = std::make_shared<MultiSystem>(name);
        inner->setMode("A");
        inner->setSubInitCash(100000.0);
        inner->add(leaf);
        return inner;
    };
    auto inner1 = make_inner("inner1_clear", make_shared<ScriptedBuySellSG>(
                                               k[k.size() - 3].datetime, k[k.size() - 2].datetime));
    auto inner2 = make_inner("inner2_buy", SG_Cycle());

    auto se = std::make_shared<SelectAllSE>(SystemList{inner1, inner2});
    auto ms = std::make_shared<MultiSystem>("ms_b_net_clear");
    ms->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    ms->setMode("B");
    ms->setSE(se);
    ms->add(inner1);
    ms->add(inner2);
    ms->run(k);

    double p1 = inner1->getTM()->getPosition(k.back().datetime, stk).number;
    double p2 = inner2->getTM()->getPosition(k.back().datetime, stk).number;
    double pp = ms->getTM()->getPosition(k.back().datetime, stk).number;

    /** @arg the cleared child is flat, the buying child and the parent keep the same quantity */
    CHECK_EQ(p1, 0.0);
    CHECK_GT(p2, 0.0);
    CHECK_EQ(pp, doctest::Approx(p1 + p2));
}

/** @par Check point: the outer A sampling does not count a nested A's leaf intent twice (the inner
 * sampling already folded it into its target) */
TEST_CASE("test_MultiSystem_nested_a_no_double_pending") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 10);

    auto sg = make_shared<ScriptedBuySellSG>(k[k.size() - 3].datetime, Datetime(203001010000LL));
    auto leaf = SYS_Simple(crtTM(), MM_FixedCount(1000), EnvironmentPtr(), ConditionPtr(), sg);
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", true);
    auto inner = std::make_shared<MultiSystem>("inner_a_double");
    inner->setMode("A");
    inner->setSubInitCash(100000.0);
    inner->add(leaf);

    auto se = std::make_shared<SelectAllSE>(SystemList{inner});
    auto ms = std::make_shared<MultiSystem>("outer_a_double");
    ms->setTM(crtTM(k[0].datetime, 100000.0));
    ms->setMode("A");
    ms->setSE(se);
    ms->add(inner);
    ms->run(k);

    double inner_pos = inner->getTM()->getPosition(k.back().datetime, stk).number;
    double parent_pos = ms->getTM()->getPosition(k.back().datetime, stk).number;
    REQUIRE_GT(inner_pos, 0.0);

    /** @arg the parent follows the inner target, not twice the leaf intent */
    CHECK_LT(parent_pos, inner_pos * 1.5);
}

/** @par Check point: a deep nested A (outer A -> middle B -> inner A, trade_on_close=false) is
 * still visible to the outer sampling through its queued batch */
TEST_CASE("test_MultiSystem_cascade_a_b_a_deferred_intent") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 10);

    Datetime buy_date = k[k.size() - 3].datetime;
    auto sg = make_shared<ScriptedBuySellSG>(buy_date, Datetime(203001010000LL));
    auto leaf = SYS_Simple(crtTM(), MM_FixedCount(1000), EnvironmentPtr(), ConditionPtr(), sg);
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", true);

    auto inner = std::make_shared<MultiSystem>("inner_a_deferred");
    inner->setMode("A");
    inner->setSubInitCash(100000.0);
    inner->setTradeOnClose(false);
    inner->add(leaf);

    auto middle = std::make_shared<MultiSystem>("middle_b");
    middle->setMode("B");
    middle->setSE(std::make_shared<SelectAllSE>(SystemList{inner}));
    middle->add(inner);

    auto outer = std::make_shared<MultiSystem>("outer_a_cascade");
    outer->setTM(crtTM(Datetime(200001010000LL), 1000000.0));
    outer->setMode("A");
    outer->setSE(std::make_shared<SelectAllSE>(SystemList{middle}));
    outer->add(middle);
    outer->run(k);

    Datetime first_buy;
    for (const auto& tr : outer->getTM()->getTradeList()) {
        if (tr.business == BUSINESS_BUY) {
            first_buy = tr.datetime;
            break;
        }
    }

    /** @arg the outer buys on the intent day, not one period later */
    CHECK_UNARY(!first_buy.isNull());
    CHECK_EQ(first_buy, buy_date);
}

/** @par Check point: an outer mode B reducing a nested mode B composite delegates the budget, so
 * the leaf ledger, the composite account and the outer parent stay matched */
TEST_CASE("test_MultiSystem_mode_b_nested_b_quota_reduction") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query1(Datetime(19991110), Datetime(20000120));
    KQuery query2(Datetime(20000121), Datetime(20000225));
    KData k1 = stk.getKData(query1);
    KData k2 = stk.getKData(query2);
    REQUIRE(k1.size() > 5);
    REQUIRE(k2.size() > 5);

    auto leaf = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", false);
    leaf->setTO(stk.getKData(KQuery(Datetime(19991110), Datetime(20000225))));
    auto inner = std::make_shared<MultiSystem>("inner_b_nested");
    inner->setMode("B");
    inner->setSE(std::make_shared<SelectAllSE>(SystemList{leaf}));
    inner->add(leaf);

    auto ms = std::make_shared<MultiSystem>("outer_b_nested");
    ms->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    ms->setMode("B");
    ms->setSE(std::make_shared<SelectAllSE>(SystemList{inner}));
    ms->add(inner);
    ms->run(k1);

    double leaf1 = leaf->getTM()->getPosition(k1.back().datetime, stk).number;
    double inner1 = inner->getTM()->getPosition(k1.back().datetime, stk).number;
    double parent1 = ms->getTM()->getPosition(k1.back().datetime, stk).number;
    REQUIRE_GT(leaf1, 0.0);
    CHECK_EQ(inner1, doctest::Approx(parent1));

    /** @arg a shrunk outer budget reaches the leaf ledger through the inner calibration */
    ms->setAF(AF_FixedAmount(30000.0));
    ms->run(k2, false);
    double leaf2 = leaf->getTM()->getPosition(k2.back().datetime, stk).number;
    double inner2 = inner->getTM()->getPosition(k2.back().datetime, stk).number;
    double parent2 = ms->getTM()->getPosition(k2.back().datetime, stk).number;
    CHECK_LT(inner2, inner1);
    CHECK_EQ(leaf2, doctest::Approx(inner2));
    CHECK_EQ(inner2, doctest::Approx(parent2));
}

/** @par Check point: mode C with trade_on_close=false records the rebalancing turnover from the
 * next open fills */
TEST_CASE("test_MultiSystem_mode_c_turnover_trade_on_close_false") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 10);

    Datetime buy_date = k[k.size() - 3].datetime;
    auto sg = make_shared<ScriptedBuySellSG>(buy_date, Datetime(203001010000LL));
    auto leaf = SYS_Simple(crtTM(), MM_FixedCount(1000), EnvironmentPtr(), ConditionPtr(), sg);
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", true);

    auto ms = std::make_shared<MultiSystem>("ms_c_turnover_false");
    ms->setTM(crtTM(k[0].datetime, 100000.0));
    ms->setMode("C");
    ms->setSE(std::make_shared<SelectAllSE>(SystemList{leaf}));
    ms->setTradeOnClose(false);
    ms->add(leaf);
    ms->run(k);

    /** @arg the open fill is attributed to the rebalancing close */
    CHECK_UNARY(!ms->getAdjustTurnover().empty());
    CHECK_EQ(ms->getAdjustTurnover().back().first, buy_date);
}

/** @par Check point: a same-bar open fill followed by a close reversal is netted in one batch, so
 * the parent does not keep a phantom position the sub-system already closed */
TEST_CASE("test_MultiSystem_mode_b_same_bar_netting") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData kdata = stk.getKData(query);
    REQUIRE(kdata.size() > 3);

    auto sg = make_shared<ScriptedBuySellSG>(kdata[0].datetime, kdata[1].datetime);
    auto sys = SYS_Simple(crtTM(), MM_FixedCount(100), EnvironmentPtr(), ConditionPtr(), sg);
    sys->setStock(stk);
    sys->setParam<bool>("buy_delay", true);
    sys->setParam<bool>("sell_delay", false);

    auto se = std::make_shared<SelectAllSE>(SystemList{sys});
    auto ms = std::make_shared<MultiSystem>("ms_mode_b_netting");
    ms->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    ms->setMode("B");
    ms->setSE(se);
    ms->add(sys);
    ms->run(kdata);

    /** @arg both books are flat: the open fill and the close reversal cancel out */
    CHECK_EQ(sys->getTM()->getPosition(kdata.back().datetime, stk).number, 0.0);
    CHECK_EQ(ms->getTM()->getPosition(kdata.back().datetime, stk).number, 0.0);
}

/** @par Check point: mode B mirror of a short book -- the full cover of the sub-system short must
 * reach the parent as a short-side CLEAR (buyShort), not as opening a long. The old code read the
 * POST-trade net position (a flat book) and misclassified the full cover as a long BUY. */
TEST_CASE("test_MultiSystem_mode_b_mirror_cover_short_side") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData kdata = stk.getKData(query);
    REQUIRE(kdata.size() > 3);

    // The first close (close-day index 0) is a rebalancing day with adjust_cycle=2, the second
    // close is not: the sub opens the short on the rebalancing day and covers it on the mirror day
    Datetime open_date = kdata[0].datetime;
    Datetime cover_date = kdata[1].datetime;

    auto sg = make_shared<ScriptedShortSG>(open_date, cover_date);
    sg->setParam<bool>("support_borrow_stock", true);
    auto sys = SYS_Simple(crtTM(), MM_FixedCount(100), EnvironmentPtr(), ConditionPtr(), sg);
    sys->setParam<bool>("buy_delay", false);
    sys->setParam<bool>("sell_delay", false);
    sys->setParam<bool>("support_borrow_stock", true);
    sys->setStock(stk);

    auto se = SE_Fixed();
    se->addStockList({stk}, sys);
    auto tm = crtTM(Datetime(199001010000LL), 100000.0);
    auto pf = PF_Simple(tm, se, AF_EqualWeight(), 2, "query", true);  // mode B
    REQUIRE_EQ(pf->getMode(), "B");
    // readyForRun syncs this switch to the parent TM; without it the parent's sellShort is
    // rejected ("Non borrowed, can't sell short")
    pf->setParam<bool>("support_borrow_stock", true);
    pf->run(query);

    size_t sell_short_count = 0;
    size_t buy_short_count = 0;
    size_t buy_count = 0;
    for (const auto& tr : tm->getTradeList()) {
        if (tr.stock != stk) {
            continue;
        }
        if (tr.business == BUSINESS_SELL_SHORT) {
            sell_short_count++;
        } else if (tr.business == BUSINESS_BUY_SHORT) {
            buy_short_count++;
        } else if (tr.business == BUSINESS_BUY) {
            buy_count++;
        }
    }

    /** @arg the parent mirrored the short opening (sellShort) and the full cover (buyShort) */
    CHECK_EQ(sell_short_count, 1);
    CHECK_EQ(buy_short_count, 1);

    /** @arg the full cover did NOT open a long (the regression discriminator of the side misjudge)
     */
    CHECK_EQ(buy_count, 0);

    /** @arg the parent short book is flat after the cover */
    CHECK_EQ(tm->getShortPosition(stk).number, 0.0);
}

/** @par Check point: mode B mirror of a zero-crossing sub book -- a nested aggregate that sells its
 * long leg and opens a short leg on the same instrument in one bar must reach the parent as TWO
 * legs (a long-side CLEAR + a short-side SELL). The old net-delta folding reconstructed a single
 * one-leg order (sell ~300 with only 100 held) that the TM rejected, dropping the whole mirror. */
TEST_CASE("test_MultiSystem_mode_b_mirror_zero_crossing") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData kdata = stk.getKData(query);
    REQUIRE(kdata.size() > 4);

    // d0: the mode B quota is injected into the aggregate only AFTER the first close, so the
    // aggregate can trade from d1 on
    Datetime d1 = kdata[1].datetime;
    Datetime d2 = kdata[2].datetime;

    // child1 (long leg): buy 200 on d1, sell all on d2 -- reuses ScriptedShortSG with the dates
    // swapped (sell signal on d2, buy signal on d1). The short child must OUTWEIGH the long child
    // at the aggregate (the L2 target is sum(weight x ratio), equal weights 0.5), otherwise the
    // net target stays positive and no zero-crossing happens: the short 600 maps to the ratio
    // -0.168 while the long 200 maps to +0.056, so the aggregate net target goes negative
    auto sg1 = make_shared<ScriptedShortSG>(d2, d1);
    auto sys1 = SYS_Simple(crtTM(), MM_FixedCount(200), EnvironmentPtr(), ConditionPtr(), sg1);
    sys1->setParam<bool>("buy_delay", false);
    sys1->setParam<bool>("sell_delay", false);
    sys1->setStock(stk);

    // child2 (short leg): open a short of 600 on d2 only (the cover date is out of the window)
    auto sg2 = make_shared<ScriptedShortSG>(d2, Datetime(299901010000LL));
    sg2->setParam<bool>("support_borrow_stock", true);
    auto sys2 = SYS_Simple(crtTM(), MM_FixedCount(600), EnvironmentPtr(), ConditionPtr(), sg2);
    sys2->setParam<bool>("buy_delay", false);
    sys2->setParam<bool>("sell_delay", false);
    sys2->setParam<bool>("support_borrow_stock", true);
    sys2->setStock(stk);

    // The nested mode A aggregate: its own account buys 100 shares on d1 and flips to a short book
    // on d2 (sell the 100 long + open ~200 short in the same bar)
    auto agg = PF_SignalAggregate(crtTM(), SE_Fixed(), AF_EqualWeight(), 1, "query", true);
    agg->add(sys1);
    agg->add(sys2);
    agg->setParam<bool>("support_borrow_stock", true);  // the aggregate account opens the short

    // The grandparent mirrors the aggregate account (mode B, no SE: an aggregate proto carries no
    // MM/SG and cannot be bound as an SE prototype, so it is added directly)
    auto tm = crtTM(Datetime(199001010000LL), 100000.0);
    auto pf = PF_Simple(tm, SelectorPtr(), AF_EqualWeight(), 1, "query", true);  // mode B
    REQUIRE_EQ(pf->getMode(), "B");
    pf->add(agg);
    pf->setParam<bool>("support_borrow_stock", true);
    pf->run(query);

    size_t buy_count = 0;
    size_t sell_count = 0;
    size_t sell_short_count = 0;
    size_t buy_short_count = 0;
    for (const auto& tr : tm->getTradeList()) {
        if (tr.stock != stk) {
            continue;
        }
        if (tr.business == BUSINESS_BUY) {
            buy_count++;
        } else if (tr.business == BUSINESS_SELL) {
            sell_count++;
        } else if (tr.business == BUSINESS_SELL_SHORT) {
            sell_short_count++;
        } else if (tr.business == BUSINESS_BUY_SHORT) {
            buy_short_count++;
        }
    }

    /** @arg d1: the long leg is mirrored as one buy */
    CHECK_EQ(buy_count, 1);

    /** @arg d2: the crossing reaches the parent as both legs -- the long-side exit (the CLEAR sell)
     * AND the short-side opening; the folded one-leg order would have been rejected (sell 300 with
     * only 200 held) and left both counts at 0 */
    CHECK_EQ(sell_count, 1);
    CHECK_EQ(sell_short_count, 1);
    CHECK_EQ(buy_short_count, 0);

    /** @arg the parent ends with the mirrored short book */
    CHECK_GT(tm->getShortPosition(stk).number, 0.0);
}

/** @par Check point: mode A (signal aggregation) -- the sub shadow signal cash stays bounded.
 * The shadows run continuously and are never reset (the old per-rebalancing-day reset was
 * removed): the sub book keeps submitting intents around the initial signal cash instead of
 * piling up, and the parent keeps rebalancing (before the fix the shadow ran out of cash and the
 * portfolio stopped rebalancing). */
TEST_CASE("test_MultiSystem_mode_a_shadow_cash_bounded") {
    Stock stk1 = getStock("sh600000");
    Stock stk2 = getStock("sz000001");
    REQUIRE(!stk1.isNull());
    REQUIRE(!stk2.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));

    auto se = SE_Fixed();
    se->addStockList({stk1, stk2}, create_alway_buy_sys());
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto pf = PF_SignalAggregate(tm, se, AF_EqualWeight(), 1, "query", true);  // mode A
    REQUIRE_EQ(pf->getMode(), "A");
    pf->run(query);
    auto subs = pf->getSystemList();
    REQUIRE_EQ(subs.size(), 2);

    // The upper bound is the real invariant (the shadow must never accumulate -- a buggy book
    // used to inflate one leg to millions)
    Datetime last_date(20000224);
    for (auto& sub : subs) {
        price_t sub_total = sub->getTM()->getFunds(last_date, query.kType()).total_assets();
        CHECK_GT(sub_total, 90000.0);
        CHECK_LT(sub_total, 150000.0);
    }

    // The portfolio runs (nearly) fully invested instead of leaving the parent cash idle
    auto pf_funds = tm->getFunds(last_date, query.kType());
    REQUIRE_GT(pf_funds.total_assets(), 0.0);
    CHECK_LT(pf_funds.cash, pf_funds.total_assets() * 0.1);
}

/** @par Check point (regression): the forced sell of the aggregate system takes MAX_DOUBLE as the
 * "sell everything" sentinel (the mode B _clearSubSystem passes it). It has to be resolved before
 * the board-lot rounding: the double -> int64 conversion of an out-of-range value is undefined
 * behavior, and it used to leave the position unsold on MSVC while closing it on clang/arm64, so
 * the sub shadow book never released its position and the results diverged across the platforms. */
TEST_CASE("test_MultiSystem_force_sell_all_sentinel") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData kdata = stk.getKData(query);
    REQUIRE(kdata.size() > 2);

    auto tm = crtTM(Datetime(199911100000LL), 100000.0);
    auto sys = create_alway_buy_sys();
    sys->setTM(tm);
    sys->setTO(kdata);
    sys->run(kdata);

    Datetime last_date = kdata[kdata.size() - 1].datetime;
    double hold = tm->getHoldNumber(last_date, stk);
    REQUIRE_GT(hold, 0.0);

    /** @arg MAX_DOUBLE closes the whole position whatever the board lot is */
    TradeRecord tr = sys->sellForceOnClose(last_date, MAX_DOUBLE, PART_PORTFOLIO);
    CHECK(!tr.isNull());
    CHECK_EQ(tr.number, hold);
    CHECK_EQ(tm->getHoldNumber(last_date, stk), 0.0);
}

/** @par Check point: MultiSystem uses the inherited System::m_path (no shadow member); setPath on
 * a nested aggregate propagates through the base member and getPath reads it back */
TEST_CASE("test_MultiSystem_path_no_shadow") {
    auto inner = std::make_shared<MultiSystem>("inner");
    inner->setPath("outer/inner");
    CHECK_EQ(inner->System::getPath(), std::string("outer/inner"));
    CHECK_EQ(inner->getPath(), std::string("outer/inner"));
}

/** @par Check point: MultiSystem::readyForRun propagates support_borrow_cash / support_borrow_stock
 * to its own TM (the base System::readyForRun does that for leaf systems; the aggregate overrides
 * without delegating) */
TEST_CASE("test_MultiSystem_ready_for_run_syncs_margin_params") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KData kdata = stk.getKData(KQueryByDate(Datetime(19991110), Datetime(20000225)));
    REQUIRE(!kdata.empty());

    auto ms = std::make_shared<MultiSystem>("ms");
    auto tm = crtTM(Datetime(199001010000LL), 100000.0);
    ms->setTM(tm);
    ms->setParam<bool>("support_borrow_cash", true);
    ms->setParam<bool>("support_borrow_stock", true);
    auto sys = SYS_Simple(crtTM(Datetime(199001010000LL), 100000.0), MM_Nothing(), EnvironmentPtr(),
                          ConditionPtr(), SG_AllwaysBuy());
    sys->setTO(kdata);
    ms->add(sys);
    ms->setTO(kdata);
    ms->readyForRun();

    /** @arg the two switches reach the aggregate's own TM */
    CHECK_UNARY(ms->getTM()->getParam<bool>("support_borrow_cash"));
    CHECK_UNARY(ms->getTM()->getParam<bool>("support_borrow_stock"));
}

/** @par Check point: mode A clamps the negative net target to zero when the parent account cannot
 * borrow securities (otherwise the short order is silently rejected by the TM); with borrowing
 * supported the short target is kept as a short-side SELL. */
TEST_CASE("test_AllocateFunds_L2_modeA_short_clamp_without_borrow") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);  // support_borrow_stock defaults to false
    REQUIRE_UNARY(!tm->getParam<bool>("support_borrow_stock"));
    auto af = AF_EqualWeight();
    Datetime date(200001010000LL);
    Stock stk = getStock("sz000001");
    REQUIRE(!stk.isNull());
    auto sys = create_test_sys(3, 5);
    std::unordered_map<SYSPtr, double> weights{{sys, 1.0}};

    TradeSuggestion s;
    s.stock = stk;
    s.sys = sys;
    s.plan_price = 10.0;
    s.target_position_ratio = -0.5;  // a net short target

    /** @arg without borrow the negative target is clamped to zero: no order is emitted */
    TradeSuggestionList suggestions{s};
    af->_toTargets(date, tm, suggestions, weights, KQuery(), "A");
    CHECK_UNARY(suggestions.empty());

    /** @arg with borrow the short target is kept: a short-side SELL opens the short */
    tm->setParam<bool>("support_borrow_stock", true);
    suggestions.assign(1, s);
    af->_toTargets(date, tm, suggestions, weights, KQuery(), "A");
    REQUIRE_EQ(suggestions.size(), 1);
    CHECK(suggestions[0].type == SuggestionType::SELL);
    CHECK_UNARY(suggestions[0].short_side);
    CHECK_EQ(suggestions[0].number, doctest::Approx(5000.0));
}

/** Recording AF: keeps the last L1 context list so the test can inspect what MultiSystem filled */
class RecordingContextAF : public AllocateFundsBase {
public:
    RecordingContextAF() : AllocateFundsBase("RecordingContextAF") {}

    AllocateFundsPtr _clone() override {
        return std::make_shared<RecordingContextAF>();
    }

    Weights _allocate(const Datetime& date, const TradeManagerPtr& tm,
                      SubSystemContextList& contexts, const KQuery& query,
                      const string& mode) override {
        last_contexts = contexts;
        return AllocateFundsBase::_allocate(date, tm, contexts, query, mode);
    }

    SubSystemContextList last_contexts;
};

/** @par Check point: mode A fills the L1 context fields -- current_weight (the sub account scale
 * over the parent real account) and total_return (the shadow return over the initial signal cash).
 * They used to be filled only by the mode B path, so a mode A AF always saw zeros. */
TEST_CASE("test_MultiSystem_mode_a_context_fields") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));

    auto se = SE_Fixed();
    se->addStockList({stk}, create_alway_buy_sys());
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto af = make_shared<RecordingContextAF>();
    price_t sub_init_cash = 100000.0;
    auto pf =
      PF_SignalAggregate(tm, se, af, 1, "query", true, true, false, sub_init_cash);  // mode A
    REQUIRE_EQ(pf->getMode(), "A");
    pf->run(query);

    REQUIRE_EQ(af->last_contexts.size(), 1);
    const auto& ctx = af->last_contexts[0];

    /** @arg the sub account scale over the parent real account is filled (previously always 0) */
    CHECK_UNARY(ctx.current_weight > 0.0);

    /** @arg the shadow return over the initial signal cash is filled with the documented formula */
    CHECK_UNARY(std::isfinite(ctx.total_return));
    CHECK_EQ(ctx.total_return,
             doctest::Approx((ctx.funds.total_assets() - sub_init_cash) / sub_init_cash));
}

/** @par Check point: the alphabetical alias factories map to the same modes and
 * presets as the semantic-name factories */
TEST_CASE("test_MultiSystem_pf_mode_aliases") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);

    /** @arg PF_ModeA behaves as PF_SignalAggregate (mode A) */
    auto se_a = SE_Fixed();
    se_a->addStockList({stk}, create_alway_buy_sys());
    auto a = PF_ModeA(tm, se_a, AF_EqualWeight(), 2, "query", true, true, false, 100000.0);
    REQUIRE(a != nullptr);
    CHECK_EQ(a->getMode(), "A");
    CHECK_EQ(a->getSubInitCash(), doctest::Approx(100000.0));

    /** @arg PF_ModeB behaves as PF_Simple (mode B, trade_on_close=true, sell_at_not_selected=true)
     */
    auto se_b = SE_Fixed();
    se_b->addStockList({stk}, create_alway_buy_sys());
    auto b = PF_ModeB(tm, se_b, AF_EqualWeight(), 2);
    REQUIRE(b != nullptr);
    CHECK_EQ(b->getMode(), "B");
    CHECK_UNARY(b->getTradeOnClose());
    CHECK_UNARY(b->getSellAtNotSelected());

    /** @arg PF_ModeB forwards the two switches exactly as PF_Simple does */
    auto b2 = PF_ModeB(tm, se_b, AF_EqualWeight(), 2, "query", true, false, false);
    CHECK_EQ(b2->getTradeOnClose(), false);
    CHECK_EQ(b2->getSellAtNotSelected(), false);

    /** @arg PF_ModeC behaves as PF_WithoutAF (mode C, sell_at_not_selected=false by default) */
    auto c = PF_ModeC(tm, se_b);
    REQUIRE(c != nullptr);
    CHECK_EQ(c->getMode(), "C");
    CHECK_UNARY(!c->getSellAtNotSelected());
}

/** @par Check point: the intent state of a nested aggregate child includes its already-converted
 * but not yet executed open pending suggestions (the intent state = the net exposure + the
 * unfilled requests; the parent snapshot must not miss the target the child has queued for its
 * own next open) */
TEST_CASE("test_MultiSystem_mode_a_nested_pending_intent") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));

    // The leaf keeps the default buy_delay=true, so its first signal is a delayed request only
    auto se_inner = SE_Fixed();
    se_inner->addStockList(
      {stk}, SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_AllwaysBuy()));
    auto inner = PF_SignalAggregate(crtTM(Datetime(199001010000LL), 100000.0), se_inner,
                                    AF_EqualWeight(), 1, "query", true, false, false, 100000.0);
    REQUIRE_EQ(inner->getMode(), "A");
    CHECK_UNARY(!inner->getTradeOnClose());

    auto outer_tm = crtTM(Datetime(199001010000LL), 100000.0);
    auto outer = std::make_shared<MultiSystem>("outer_mode_a");
    outer->setTM(outer_tm);
    outer->setMode("A");
    outer->setAdjustCycle(1);
    outer->setTradeOnClose(true);
    outer->add(inner);
    outer->run(query);

    const auto& outer_trades = outer_tm->getTradeList();
    REQUIRE_UNARY(outer_trades.size() > 0);

    // The trade list starts with the BUSINESS_INIT record; take the first real trade
    TradeRecord first;
    for (auto& tr : outer_trades) {
        if (tr.business != BUSINESS_INIT) {
            first = tr;
            break;
        }
    }
    REQUIRE_UNARY(first.business != BUSINESS_INIT);
    KData k = stk.getKData(query);
    REQUIRE_UNARY(k.size() > 0);

    /** @arg the outer layer buys on the very first close day: the queued intent of the inner
     * aggregate is folded into the snapshot (without the folding the parent would see a zero
     * exposure and would only start buying one day later, after the inner queue is executed) */
    CHECK_EQ(first.business, BUSINESS_BUY);
    CHECK_EQ(first.datetime, k.getKRecord(0).datetime);

    /** @arg the inner aggregate executes its own queued target later than the outer order (its
     * pending runs at the next open of the inner layer, while the outer layer already bought at
     * the first close from the folded intent) */
    TradeRecord inner_first;
    for (auto& tr : inner->getTM()->getTradeList()) {
        if (tr.business != BUSINESS_INIT) {
            inner_first = tr;
            break;
        }
    }
    if (inner_first.business != BUSINESS_INIT) {
        CHECK_GT(inner_first.datetime, first.datetime);
    }
}

/** Selector test double: admits every running system on the first consultation only, then nothing
 *  (used to drive the "no longer selected" path of mode C) */
class SelectOnceSE : public SelectorBase {
public:
    SelectOnceSE() : SelectorBase("SelectOnceSE") {}

    SelectorPtr _clone() override {
        return std::make_shared<SelectOnceSE>();
    }

    void _calculate() override {}

    SystemWeightList _getSelected(Datetime) override {
        SystemWeightList ret;
        if (call_count++ == 0) {
            for (auto& sys : getRealSystemList()) {
                ret.emplace_back(sys, 1.0);
            }
        }
        return ret;
    }

    int call_count{0};
};

/** @par Check point: a nested aggregate under mode C shares the real account of the parent and
 * its synthesized orders land on it directly (the parent neither converts nor orders again) */
TEST_CASE("test_MultiSystem_mode_c_nested_aggregate_child") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));

    // The inner aggregate: mode A over one always-buy leaf
    auto inner_se = SE_Fixed();
    inner_se->addStockList({stk}, create_alway_buy_sys());
    auto inner = PF_SignalAggregate(crtTM(Datetime(199001010000LL), 100000.0), inner_se,
                                    AF_EqualWeight(), 1, "query", true, true, false, 100000.0);
    REQUIRE_EQ(inner->getMode(), "A");

    // The outer aggregate: mode C, the inner one is its only sub-system
    auto parent_tm = crtTM(Datetime(199001010000LL), 100000.0);
    auto outer = std::make_shared<MultiSystem>("outer_mode_c_nested");
    outer->setTM(parent_tm);
    outer->setMode("C");
    outer->setSellAtNotSelected(false);
    outer->add(inner);
    outer->run(query);

    /** @arg the inner aggregate shares the real account of the parent */
    CHECK(inner->getTM() == parent_tm);
    CHECK_UNARY(inner->getParam<bool>("shared_tm"));

    /** @arg the orders synthesized by the inner mode A layer land on the shared account */
    REQUIRE_UNARY(parent_tm->getTradeList().size() > 1);
    double hold = parent_tm->getHoldNumber(Datetime(200002250000LL), stk);
    CHECK_GT(hold, 0.0);

    /** @arg the parent does not order again: the holding stays within a single full position of
     *  the account (a duplicated parent order would roughly double it) */
    auto k = stk.getKData(query);
    REQUIRE_UNARY(k.size() > 0);
    double price = k.getKRecord(k.size() - 1).closePrice;
    CHECK_LE(hold * price, 100000.0 * 1.05);
}

/** @par Check point: mode C force-liquidates a nested aggregate that left the running pool while
 * still holding on the shared account (the holding judgement must look through the sub-tree: a
 * nested aggregate carries no stock of its own) */
TEST_CASE("test_MultiSystem_mode_c_nested_aggregate_force_sell") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));

    auto inner_se = SE_Fixed();
    inner_se->addStockList({stk}, create_alway_buy_sys());
    auto inner = PF_SignalAggregate(crtTM(Datetime(199001010000LL), 100000.0), inner_se,
                                    AF_EqualWeight(), 1, "query", true, true, false, 100000.0);

    auto parent_tm = crtTM(Datetime(199001010000LL), 100000.0);
    auto outer = std::make_shared<MultiSystem>("outer_mode_c_nested_drop");
    outer->setTM(parent_tm);
    outer->setMode("C");
    outer->setSellAtNotSelected(true);               // force liquidation of the unselected
    outer->setSE(std::make_shared<SelectOnceSE>());  // admitted on the first day only
    outer->add(inner);
    outer->run(query);

    /** @arg the sub-system was admitted on the first day and holds on the shared account */
    CHECK_GT(parent_tm->getTradeList().size(), 1);

    /** @arg once it is no longer selected, the parent force-liquidates the holding of the whole
     *  sub-tree (the inner aggregate has no stock of its own, so the check must reach the leaves)
     */
    CHECK_EQ(parent_tm->getHoldNumber(Datetime(200002250000LL), stk), doctest::Approx(0.0));
    bool has_sell = false;
    for (auto& tr : parent_tm->getTradeList()) {
        if (tr.business == BUSINESS_SELL) {
            has_sell = true;
            break;
        }
    }
    CHECK_UNARY(has_sell);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
/** @par Check point: a run with resetAll keeps the instruments of the sub-systems (the full reset
 * wipes them, but they are the topology handed over by the parent / the SE and must survive, or
 * the aggregate would silently degrade into "no trades" on the second run) */
TEST_CASE("test_MultiSystem_reset_all_keeps_instruments") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));

    auto se = SE_Fixed();
    se->addStockList({stk}, create_alway_buy_sys());
    auto tm = crtTM(Datetime(199001010000LL), 100000.0);
    auto pf = PF_SignalAggregate(tm, se, AF_EqualWeight(), 1, "query", true, true, false, 100000.0);

    pf->run(query, true, false);
    const size_t first_trades = tm->getTradeList().size();
    REQUIRE_UNARY(first_trades > 1);

    /** @arg the second run with resetAll still trades (the instruments survived the reset) */
    pf->run(query, true, true);
    CHECK_UNARY(tm->getTradeList().size() > 1);

    /** @arg the sub-systems still carry their instruments after the resetAll run */
    for (auto& sys : pf->getSystemList()) {
        CHECK_UNARY(!sys->getStock().isNull());
        CHECK_UNARY(!sys->getTO().empty());
    }
}

/** @par Check point: a mode change rebuilds the account form of the sub-systems (switching back
 * to A after C must give them shadow accounts again instead of the shared real account) */
TEST_CASE("test_MultiSystem_mode_switch_rebuilds_accounts") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));

    auto se = SE_Fixed();
    se->addStockList({stk}, create_alway_buy_sys());
    auto parent_tm = crtTM(Datetime(199001010000LL), 100000.0);
    auto ms = std::make_shared<MultiSystem>("ms_mode_switch");
    ms->setTM(parent_tm);
    ms->setSE(se);
    ms->setSellAtNotSelected(false);

    ms->setMode("A");
    ms->run(query);
    SystemPtr sub = ms->getSystemList()[0];
    REQUIRE_UNARY(sub->getTM() != parent_tm);
    CHECK_UNARY(!sub->getParam<bool>("shared_tm"));

    /** @arg in mode C the sub-system trades on the shared real account */
    ms->setMode("C");
    ms->run(query);
    CHECK_UNARY(sub->getTM() == parent_tm);
    CHECK_UNARY(sub->getParam<bool>("shared_tm"));

    /** @arg switching back to A gives the sub-system a fresh shadow account again */
    ms->setMode("A");
    ms->run(query);
    CHECK_UNARY(sub->getTM() != parent_tm);
    CHECK_UNARY(!sub->getParam<bool>("shared_tm"));
}

/** @par Check point: the SE refuses a prototype that carries shared_tm (the shared account is an
 * aggregate-side decision, a prototype must stay an isolated account) */
TEST_CASE("test_PF_WithoutAF_rejects_shared_tm_proto") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    auto proto = create_alway_buy_sys();
    proto->setParam<bool>("shared_tm", true);
    auto se = SE_Fixed();

    /** @arg adding a shared_tm prototype throws */
    CHECK_THROWS_AS(se->addStockList({stk}, proto), hku::exception);
}

#if HKU_SUPPORT_SERIALIZATION
/** @par Check point: the current-schema (straight-line) serialization round-trip of MultiSystem
 * keeps the mode and the preset configuration */
TEST_CASE("test_MultiSystem_serialize_round_trip") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    auto se = SE_Fixed();
    se->addStockList({stk}, create_alway_buy_sys());
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto pf1 =
      PF_SignalAggregate(tm, se, AF_FixedWeight(0.6), 3, "week", false, true, false, 50000.0);
    pf1->name("pf_roundtrip");

    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/MultiSystem_roundtrip.xml";
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(pf1);
    }

    MultiSystemPtr pf2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(pf2);
    }
    REQUIRE(pf2 != nullptr);

    /** @arg the running mode and the preset fields survive the round-trip */
    CHECK_EQ(pf2->getMode(), pf1->getMode());
    CHECK_EQ(pf2->getSubInitCash(), pf1->getSubInitCash());
    CHECK_EQ(pf2->getAdjustCycle(), pf1->getAdjustCycle());
    CHECK_EQ(pf2->getTradeOnClose(), pf1->getTradeOnClose());
    CHECK_EQ(pf2->getSellAtNotSelected(), pf1->getSellAtNotSelected());

    /** @arg the sub-system list and the AF parameters survive the round-trip */
    CHECK_EQ(pf2->getSystemList().size(), pf1->getSystemList().size());
    REQUIRE(pf2->getAF() != nullptr);
    CHECK_EQ(pf2->getAF()->getParam<double>("weight"), doctest::Approx(0.6));
    CHECK_UNARY(pf2->getSE() != nullptr);
}
#endif /* HKU_SUPPORT_SERIALIZATION */

/** @par Check point: mode B with trade_on_close=false reduces by the holding value, not the net
 * assets that still include the un-recycled idle cash */
TEST_CASE("test_MultiSystem_mode_b_deferred_reduction_uses_holding_value") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery full(Datetime(19991110), Datetime(20000225));
    KQuery q1(Datetime(19991110), Datetime(20000120));
    KQuery q2(Datetime(20000121), Datetime(20000225));
    KData k1 = stk.getKData(q1);
    KData k2 = stk.getKData(q2);
    REQUIRE(k1.size() > 5);
    REQUIRE(k2.size() > 5);

    auto sg = make_shared<ScriptedBuySellSG>(k1[k1.size() - 2].datetime, Datetime(203001010000LL));
    auto leaf = SYS_Simple(crtTM(), MM_FixedCount(100), EnvironmentPtr(), ConditionPtr(), sg);
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", false);
    leaf->setTO(stk.getKData(full));

    auto ms = std::make_shared<MultiSystem>("ms_b_holding_value");
    ms->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    ms->setMode("B");
    ms->setSE(std::make_shared<SelectAllSE>(SystemList{leaf}));
    ms->add(leaf);
    ms->run(k1);

    double pos1 = leaf->getTM()->getPosition(k1.back().datetime, stk).number;
    REQUIRE_EQ(pos1, 100.0);

    /** @arg the idle cash (most of the quota) is not sold as if it were a holding */
    ms->setAF(AF_FixedAmount(50000.0));
    ms->setTradeOnClose(false);
    ms->run(k2, false);
    CHECK_EQ(leaf->getTM()->getPosition(k2.back().datetime, stk).number, pos1);
}

/** @par Check point: a trade_on_close=false rebalancing with a zero parent cash still queues the
 * deferred injections and funds them from the next open sells */
TEST_CASE("test_MultiSystem_mode_b_deferred_injection_after_reduction") {
    Stock stk1 = getStock("sh600000");
    Stock stk2 = getStock("sz000001");
    REQUIRE(!stk1.isNull());
    REQUIRE(!stk2.isNull());
    KQuery full1(Datetime(19991110), Datetime(20000225));
    KQuery q1(Datetime(19991110), Datetime(20000120));
    KQuery q2(Datetime(20000121), Datetime(20000225));
    KData k1 = stk1.getKData(q1);
    KData k2 = stk1.getKData(q2);
    REQUIRE(k1.size() > 5);
    REQUIRE(k2.size() > 5);

    auto make_leaf = [&](const Stock& stk) {
        auto sys = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
        sys->setStock(stk);
        sys->setParam<bool>("buy_delay", false);
        sys->setTO(stk.getKData(full1));
        return sys;
    };
    auto leaf1 = make_leaf(stk1);
    auto leaf2 = make_leaf(stk2);

    auto ms = std::make_shared<MultiSystem>("ms_b_deferred_inject");
    ms->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    ms->setMode("B");
    ms->setSE(std::make_shared<SelectAllSE>(SystemList{leaf1, leaf2}));
    ms->add(leaf1);
    ms->add(leaf2);
    ms->run(k1);

    double a1 = leaf1->getTM()->getPosition(k1.back().datetime, stk1).number;
    double b1 = leaf2->getTM()->getPosition(k1.back().datetime, stk2).number;
    REQUIRE_GT(a1, 0.0);
    REQUIRE_GT(b1, 0.0);

    /** @arg the under-quota child receives the deferred injection, the over-quota child is reduced
     */
    ms->setAF(AF_FixedWeightList({0.8, 0.2}));
    ms->setTradeOnClose(false);
    ms->run(k2, false);
    double a2 = leaf1->getTM()->getPosition(k2.back().datetime, stk1).number;
    double b2 = leaf2->getTM()->getPosition(k2.back().datetime, stk2).number;
    CHECK_GT(a2, a1);
    CHECK_LT(b2, b1);
}

/** @par Check point: the C -> B -> A cascade keeps the shared account in step with the inner mode
 * A account (the middle mode B mirrors every inner trade) */
TEST_CASE("test_MultiSystem_cascade_c_b_a") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 6);

    auto leaf = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", false);
    auto inner = std::make_shared<MultiSystem>("cba_inner_a");
    inner->setMode("A");
    inner->setSubInitCash(100000.0);
    inner->add(leaf);

    auto middle = std::make_shared<MultiSystem>("cba_middle_b");
    middle->setMode("B");
    middle->setSE(std::make_shared<SelectAllSE>(SystemList{inner}));
    middle->add(inner);

    auto outer = std::make_shared<MultiSystem>("cba_outer_c");
    outer->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    outer->setMode("C");
    outer->setSE(std::make_shared<SelectAllSE>(SystemList{middle}));
    outer->add(middle);
    outer->run(k);

    double parent = outer->getTM()->getPosition(k.back().datetime, stk).number;
    double apos = inner->getTM()->getPosition(k.back().datetime, stk).number;

    /** @arg the shared account surfaces the inner trades exactly once */
    CHECK_GT(parent, 0.0);
    CHECK_EQ(parent, doctest::Approx(apos));
}

/** @par Check point: the A -> C -> B cascade mirrors the inner mode B trades onto the outer mode A
 * shadow account */
TEST_CASE("test_MultiSystem_cascade_a_c_b") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 6);

    auto leaf = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", false);
    auto inner = std::make_shared<MultiSystem>("acb_inner_b");
    inner->setMode("B");
    inner->setSE(std::make_shared<SelectAllSE>(SystemList{leaf}));
    inner->add(leaf);

    auto middle = std::make_shared<MultiSystem>("acb_middle_c");
    middle->setMode("C");
    middle->add(inner);

    auto outer = std::make_shared<MultiSystem>("acb_outer_a");
    outer->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    outer->setMode("A");
    outer->setSE(std::make_shared<SelectAllSE>(SystemList{middle}));
    outer->add(middle);
    outer->run(k);

    double parent = outer->getTM()->getPosition(k.back().datetime, stk).number;
    double shadow = middle->getTM()->getPosition(k.back().datetime, stk).number;

    /** @arg the outer follows the shadow account the inner trades land on */
    CHECK_GT(parent, 0.0);
    CHECK_EQ(parent, doctest::Approx(shadow));
}

/** @par Check point: the C -> C -> A cascade surfaces the inner A trade exactly once on the shared
 * account */
TEST_CASE("test_MultiSystem_cascade_c_c_a") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 10);

    Datetime buy_date = k[k.size() - 3].datetime;
    auto sg = make_shared<ScriptedBuySellSG>(buy_date, Datetime(203001010000LL));
    auto leaf = SYS_Simple(crtTM(), MM_FixedCount(100), EnvironmentPtr(), ConditionPtr(), sg);
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", false);
    auto inner = std::make_shared<MultiSystem>("cca_inner_a");
    inner->setMode("A");
    inner->setSubInitCash(100000.0);
    inner->add(leaf);

    auto middle = std::make_shared<MultiSystem>("cca_middle_c");
    middle->setMode("C");
    middle->add(inner);

    auto outer = std::make_shared<MultiSystem>("cca_outer_c");
    outer->setTM(crtTM(k[0].datetime, 100000.0));
    outer->setMode("C");
    outer->add(middle);
    outer->run(k);

    size_t parent_buys = 0;
    for (const auto& tr : outer->getTM()->getTradeList()) {
        if (tr.business == BUSINESS_BUY) {
            parent_buys++;
        }
    }

    /** @arg one inner buy is surfaced once (no duplicate ordering through the cascade) */
    CHECK_EQ(parent_buys, 1);
    CHECK_GT(outer->getTM()->getPosition(k.back().datetime, stk).number, 0.0);
}

/** @par Check point: a mode C child removed and re-admitted gets a fresh cycle start and trades
 * again */
TEST_CASE("test_MultiSystem_mode_c_reentry_after_force_sell") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 6);

    auto proto =
      SYS_Simple(crtTM(), MM_FixedCount(100), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    proto->setStock(stk);
    proto->setParam<bool>("buy_delay", false);
    auto se = std::make_shared<SelectDropSecondSE>();
    se->addStockList({stk}, proto);

    auto ms = std::make_shared<MultiSystem>("ms_c_reentry");
    ms->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    ms->setMode("C");
    ms->setSE(se);
    ms->setSellAtNotSelected(true);
    ms->run(k);
    REQUIRE_EQ(ms->getSystemList().size(), 1);

    auto sys = ms->getSystemList()[0];
    bool bought_after_drop = false;
    for (const auto& tr : sys->getTM()->getTradeList()) {
        if (tr.business == BUSINESS_BUY && tr.datetime > k[1].datetime) {
            bought_after_drop = true;
        }
    }

    /** @arg the re-admitted child trades again after the forced liquidation */
    CHECK_UNARY(bought_after_drop);
    CHECK_GT(ms->getTM()->getPosition(k.back().datetime, stk).number, 0.0);
}

/** @par Check point: mode C without an SE never declares a cycle day (the direct cycle signals
 * stay silent) */
TEST_CASE("test_MultiSystem_mode_c_no_se_cycle_silent") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 6);

    auto leaf = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", false);

    auto ms = std::make_shared<MultiSystem>("ms_c_no_se");
    ms->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    ms->setMode("C");
    ms->add(leaf);
    ms->run(k);

    /** @arg no cycle day is declared and no trade is made */
    CHECK_UNARY(leaf->getSG()->getCycleDates().empty());
    CHECK_EQ(ms->getTM()->getPosition(k.back().datetime, stk).number, 0.0);
}

/** @par Check point: a close-only live session (runMomentOnClose without runMomentOnOpen) does not
 * drive the delayed requests and does not crash */
TEST_CASE("test_MultiSystem_close_only_live_buffer_fallback") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 6);

    auto leaf = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", true);

    auto ms = std::make_shared<MultiSystem>("ms_close_only");
    ms->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    ms->setMode("B");
    ms->setSE(std::make_shared<SelectAllSE>(SystemList{leaf}));
    ms->add(leaf);
    ms->setTO(k);
    ms->readyForRun();

    for (size_t i = 0; i < 5; ++i) {
        ms->runMomentOnClose(k[i].datetime);
    }

    /** @arg the parent never trades: the delayed requests need an open drive */
    CHECK_EQ(ms->getTM()->getTradeList().size(), 1);
}

class SelectExceptDateSE : public SelectorBase {
public:
    SelectExceptDateSE(const SystemList& list, Datetime except_date)
    : SelectorBase("SelectExceptDateSE"), m_list(list), m_except_date(except_date) {}

    SelectorPtr _clone() override {
        return std::make_shared<SelectExceptDateSE>(m_list, m_except_date);
    }

    void _calculate() override {}

    SystemWeightList _getSelected(Datetime date) override {
        SystemWeightList ret;
        if (date.startOfDay() != m_except_date.startOfDay()) {
            for (auto& sys : m_list) {
                ret.emplace_back(sys, 1.0);
            }
        }
        return ret;
    }

    SystemList m_list;
    Datetime m_except_date;
};

class SelectExceptStockDateSE : public SelectorBase {
public:
    SelectExceptStockDateSE(const SystemList& list, const Stock& except_stock, Datetime except_date)
    : SelectorBase("SelectExceptStockDateSE"),
      m_list(list),
      m_except_stock(except_stock),
      m_except_date(except_date) {}

    SelectorPtr _clone() override {
        return std::make_shared<SelectExceptStockDateSE>(m_list, m_except_stock, m_except_date);
    }

    void _calculate() override {}

    SystemWeightList _getSelected(Datetime date) override {
        SystemWeightList ret;
        const bool drop = (date.startOfDay() == m_except_date.startOfDay());
        for (auto& sys : m_list) {
            if (drop && sys->getStock() == m_except_stock) {
                continue;
            }
            ret.emplace_back(sys, 1.0);
        }
        return ret;
    }

    SystemList m_list;
    Stock m_except_stock;
    Datetime m_except_date;
};

/** @par Check point: a deferred calibration sell with no quote at the next open is retried on the
 * following open (SH600005 has no bar on 2000-06-30) */
TEST_CASE("test_MultiSystem_pending_calibration_retry_suspension") {
    Stock stk = getStock("sh600005");
    Stock market = getStock("sh600000");
    REQUIRE(!stk.isNull());
    REQUIRE(!market.isNull());
    KQuery query(Datetime(20000501), Datetime(20000720));
    KQuery full(Datetime(20000401), Datetime(20000731));
    // The run axis comes from a stock that trades on 2000-06-30: the sub's instrument is
    // suspended that day, so its deferred leg has no quote and must be retried
    KData k = market.getKData(query);
    REQUIRE(k.size() > 10);

    Datetime buy_date(200005100000LL);
    Datetime drop_date(200006290000LL);
    Datetime retry_date(200007030000LL);
    auto sg = make_shared<ScriptedBuySellSG>(buy_date, Datetime(203001010000LL));
    auto leaf = SYS_Simple(crtTM(), MM_FixedCount(100), EnvironmentPtr(), ConditionPtr(), sg);
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", false);
    leaf->setTO(stk.getKData(full));

    // A second, always selected child keeps the quota calibration active on the drop day
    auto other = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    other->setStock(getStock("sz000001"));
    other->setParam<bool>("buy_delay", false);

    auto se = std::make_shared<SelectExceptStockDateSE>(SystemList{leaf, other}, stk, drop_date);
    auto ms = std::make_shared<MultiSystem>("ms_retry_suspension");
    ms->setTM(crtTM(Datetime(200005010000LL), 200000.0));
    ms->setMode("B");
    ms->setSE(se);
    ms->setSellAtNotSelected(true);
    ms->setTradeOnClose(false);
    ms->add(leaf);
    ms->add(other);
    ms->run(k);

    Datetime sub_sell_date, parent_sell_date;
    for (const auto& tr : leaf->getTM()->getTradeList()) {
        if (tr.business == BUSINESS_SELL) {
            sub_sell_date = tr.datetime;
        }
    }
    for (const auto& tr : ms->getTM()->getTradeList()) {
        if (tr.business == BUSINESS_SELL) {
            parent_sell_date = tr.datetime;
        }
    }

    /** @arg the leg skipped on the suspended day executes on the next quoted open */
    CHECK_UNARY(!sub_sell_date.isNull());
    CHECK_EQ(sub_sell_date, retry_date);
    CHECK_UNARY(!parent_sell_date.isNull());
    CHECK_EQ(leaf->getTM()->getPosition(k.back().datetime, stk).number, 0.0);
    CHECK_EQ(ms->getTM()->getPosition(k.back().datetime, stk).number, 0.0);
}

/** @par Check point: a queued suggestion with no quote at the next open is retried on the
 * following open instead of filling at the stale close price (SH600005 has no bar on 2000-06-30) */
TEST_CASE("test_MultiSystem_open_pending_retry_suspension") {
    Stock stk = getStock("sh600005");
    Stock market = getStock("sh600000");
    REQUIRE(!stk.isNull());
    REQUIRE(!market.isNull());
    KQuery query(Datetime(20000501), Datetime(20000720));
    KQuery full(Datetime(20000401), Datetime(20000731));
    KData k = market.getKData(query);
    REQUIRE(k.size() > 10);

    Datetime buy_date(200006290000LL);
    Datetime retry_date(200007030000LL);
    auto sg = make_shared<ScriptedBuySellSG>(buy_date, Datetime(203001010000LL));
    auto leaf = SYS_Simple(crtTM(), MM_FixedCount(100), EnvironmentPtr(), ConditionPtr(), sg);
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", false);
    leaf->setTO(stk.getKData(full));

    auto ms = std::make_shared<MultiSystem>("ms_open_pending_retry");
    ms->setTM(crtTM(Datetime(200005010000LL), 100000.0));
    ms->setMode("B");
    ms->setSE(std::make_shared<SelectAllSE>(SystemList{leaf}));
    ms->setTradeOnClose(false);
    ms->add(leaf);
    ms->run(k);

    Datetime parent_buy_date;
    for (const auto& tr : ms->getTM()->getTradeList()) {
        if (tr.business == BUSINESS_BUY && tr.stock == stk) {
            parent_buy_date = tr.datetime;
        }
    }

    /** @arg the fill happens on the next quoted open, never on the suspended day */
    CHECK_EQ(parent_buy_date, retry_date);
    CHECK_GT(ms->getTM()->getPosition(k.back().datetime, stk).number, 0.0);
}

/** @par Check point: a retry batch survives the next rebalancing close (merged with the new batch,
 * so the parent mirror of an already filled sub trade is never lost) */
TEST_CASE("test_MultiSystem_open_pending_retry_merged_with_new_batch") {
    Stock stk1 = getStock("sh600005");
    Stock stk2 = getStock("sh600000");
    REQUIRE(!stk1.isNull());
    REQUIRE(!stk2.isNull());
    KQuery query(Datetime(20000501), Datetime(20000720));
    KQuery full(Datetime(20000401), Datetime(20000731));
    KData k = stk2.getKData(query);
    REQUIRE(k.size() > 10);

    // stk1 is suspended on 2000-06-30: its queued buy retrains, while stk2 trades and produces a
    // new batch on that very rebalancing close
    auto sg1 = make_shared<ScriptedBuySellSG>(Datetime(200006290000LL), Datetime(203001010000LL));
    auto leaf1 = SYS_Simple(crtTM(), MM_FixedCount(100), EnvironmentPtr(), ConditionPtr(), sg1);
    leaf1->setStock(stk1);
    leaf1->setParam<bool>("buy_delay", false);
    leaf1->setTO(stk1.getKData(full));

    auto sg2 = make_shared<ScriptedBuySellSG>(Datetime(200006300000LL), Datetime(203001010000LL));
    auto leaf2 = SYS_Simple(crtTM(), MM_FixedCount(100), EnvironmentPtr(), ConditionPtr(), sg2);
    leaf2->setStock(stk2);
    leaf2->setParam<bool>("buy_delay", false);
    leaf2->setTO(stk2.getKData(full));

    auto ms = std::make_shared<MultiSystem>("ms_open_pending_merge");
    ms->setTM(crtTM(Datetime(200005010000LL), 200000.0));
    ms->setMode("B");
    ms->setSE(std::make_shared<SelectAllSE>(SystemList{leaf1, leaf2}));
    ms->setTradeOnClose(false);
    ms->add(leaf1);
    ms->add(leaf2);
    ms->run(k);

    Datetime parent1_buy_date;
    for (const auto& tr : ms->getTM()->getTradeList()) {
        if (tr.business == BUSINESS_BUY && tr.stock == stk1) {
            parent1_buy_date = tr.datetime;
        }
    }
    double parent1 = ms->getTM()->getPosition(k.back().datetime, stk1).number;
    double leaf1_pos = leaf1->getTM()->getPosition(k.back().datetime, stk1).number;
    double parent2 = ms->getTM()->getPosition(k.back().datetime, stk2).number;
    double leaf2_pos = leaf2->getTM()->getPosition(k.back().datetime, stk2).number;

    /** @arg both the retried leg and the new leg fill on the next quoted open, two books matched */
    CHECK_EQ(parent1_buy_date, Datetime(200007030000LL));
    CHECK_GT(leaf1_pos, 0.0);
    CHECK_EQ(parent1, doctest::Approx(leaf1_pos));
    CHECK_GT(leaf2_pos, 0.0);
    CHECK_EQ(parent2, doctest::Approx(leaf2_pos));
    CHECK_UNARY(!ms->getAdjustTurnover().empty());
    CHECK_EQ(ms->getAdjustTurnover().back().first, Datetime(200006300000LL));
}

/** @par Check point: a nested mode B with trade_on_close=false still matches the three ledgers
 * when the outer mode B delegates a shrunk budget */
TEST_CASE("test_MultiSystem_mode_b_nested_b_trade_on_close_false") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery full(Datetime(19991110), Datetime(20000225));
    KQuery q1(Datetime(19991110), Datetime(20000120));
    KQuery q2(Datetime(20000121), Datetime(20000225));
    KData k1 = stk.getKData(q1);
    KData k2 = stk.getKData(q2);
    REQUIRE(k1.size() > 5);
    REQUIRE(k2.size() > 5);

    // A single scripted buy in the first run: no fresh signals in the second run, so the mirrors
    // settle and the three ledgers are comparable one for one at the end
    auto sg = make_shared<ScriptedBuySellSG>(k1[k1.size() - 2].datetime, Datetime(203001010000LL));
    auto leaf = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), sg);
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", false);
    leaf->setTO(stk.getKData(full));
    auto inner = std::make_shared<MultiSystem>("inner_b_nested_deferred");
    inner->setMode("B");
    inner->setSE(std::make_shared<SelectAllSE>(SystemList{leaf}));
    inner->setTradeOnClose(false);
    inner->add(leaf);

    auto ms = std::make_shared<MultiSystem>("outer_b_nested_deferred");
    ms->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    ms->setMode("B");
    ms->setSE(std::make_shared<SelectAllSE>(SystemList{inner}));
    ms->add(inner);
    ms->run(k1);

    double leaf1 = leaf->getTM()->getPosition(k1.back().datetime, stk).number;
    double inner1 = inner->getTM()->getPosition(k1.back().datetime, stk).number;
    REQUIRE_GT(leaf1, 0.0);

    /** @arg the delegated budget reaches the leaf through the inner deferred calibration */
    ms->setAF(AF_FixedAmount(30000.0));
    ms->run(k2, false);
    double leaf2 = leaf->getTM()->getPosition(k2.back().datetime, stk).number;
    double inner2 = inner->getTM()->getPosition(k2.back().datetime, stk).number;
    double parent2 = ms->getTM()->getPosition(k2.back().datetime, stk).number;
    CHECK_LT(inner2, inner1);
    CHECK_EQ(leaf2, doctest::Approx(inner2));
    CHECK_EQ(inner2, doctest::Approx(parent2));
}

/** @par Check point: an outer mode B reducing a nested mode C composite falls back to the direct
 * pro rata reduction (a mode C inner cannot consume a delegated budget) */
TEST_CASE("test_MultiSystem_mode_b_nested_c_pro_rata_reduction") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery full(Datetime(19991110), Datetime(20000225));
    KQuery q1(Datetime(19991110), Datetime(20000120));
    KQuery q2(Datetime(20000121), Datetime(20000225));
    KData k1 = stk.getKData(q1);
    KData k2 = stk.getKData(q2);
    REQUIRE(k1.size() > 5);
    REQUIRE(k2.size() > 5);

    auto leaf = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", false);
    leaf->setTO(stk.getKData(full));
    auto inner = std::make_shared<MultiSystem>("inner_c_under_b");
    inner->setMode("C");
    inner->setSE(std::make_shared<SelectAllSE>(SystemList{leaf}));
    inner->add(leaf);

    auto ms = std::make_shared<MultiSystem>("outer_b_over_c");
    ms->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    ms->setMode("B");
    ms->setSE(std::make_shared<SelectAllSE>(SystemList{inner}));
    ms->add(inner);
    ms->run(k1);

    double inner1 = inner->getTM()->getPosition(k1.back().datetime, stk).number;
    REQUIRE_GT(inner1, 0.0);

    /** @arg the shrunk budget reduces the shared ledger pro rata and mirrors to the parent */
    ms->setAF(AF_FixedAmount(30000.0));
    ms->run(k2, false);
    double inner2 = inner->getTM()->getPosition(k2.back().datetime, stk).number;
    double parent2 = ms->getTM()->getPosition(k2.back().datetime, stk).number;
    CHECK_LT(inner2, inner1);
    CHECK_EQ(inner2, doctest::Approx(parent2));
}

/** @par Check point: an outer mode A follows a nested mode B child through its shadow account */
TEST_CASE("test_MultiSystem_mode_a_nested_b_follows_shadow") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 6);

    auto leaf = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", false);
    auto inner = std::make_shared<MultiSystem>("ab_inner_b");
    inner->setMode("B");
    inner->setSE(std::make_shared<SelectAllSE>(SystemList{leaf}));
    inner->add(leaf);

    auto outer = std::make_shared<MultiSystem>("ab_outer_a");
    outer->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    outer->setMode("A");
    outer->setSE(std::make_shared<SelectAllSE>(SystemList{inner}));
    outer->add(inner);
    outer->run(k);

    double parent = outer->getTM()->getPosition(k.back().datetime, stk).number;
    double shadow = inner->getTM()->getPosition(k.back().datetime, stk).number;

    /** @arg the parent tracks the shadow account the inner trades on */
    CHECK_GT(parent, 0.0);
    CHECK_EQ(parent, doctest::Approx(shadow));
}

/** @par Check point: with adjust_running_sys=false the nested mode B keeps its running leaf, the
 * delegated budget does not force-sell it */
TEST_CASE("test_MultiSystem_mode_b_nested_b_adjust_running_sys_false") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery full(Datetime(19991110), Datetime(20000225));
    KQuery q1(Datetime(19991110), Datetime(20000120));
    KQuery q2(Datetime(20000121), Datetime(20000225));
    KData k1 = stk.getKData(q1);
    KData k2 = stk.getKData(q2);
    REQUIRE(k1.size() > 5);
    REQUIRE(k2.size() > 5);

    auto leaf = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", false);
    leaf->setTO(stk.getKData(full));
    auto inner = std::make_shared<MultiSystem>("inner_b_no_force");
    inner->setMode("B");
    inner->setSE(std::make_shared<SelectAllSE>(SystemList{leaf}));
    inner->getAF()->setParam<bool>("adjust_running_sys", false);
    inner->add(leaf);

    auto ms = std::make_shared<MultiSystem>("outer_b_no_force");
    ms->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    ms->setMode("B");
    ms->setSE(std::make_shared<SelectAllSE>(SystemList{inner}));
    ms->add(inner);
    ms->run(k1);

    double leaf1 = leaf->getTM()->getPosition(k1.back().datetime, stk).number;
    double inner1 = inner->getTM()->getPosition(k1.back().datetime, stk).number;
    REQUIRE_GT(leaf1, 0.0);

    ms->setAF(AF_FixedAmount(30000.0));
    ms->run(k2, false);

    /** @arg the running leaf is not force-reduced by the delegated budget */
    CHECK_EQ(leaf->getTM()->getPosition(k2.back().datetime, stk).number, leaf1);
    CHECK_EQ(inner->getTM()->getPosition(k2.back().datetime, stk).number, inner1);
}

/** @par Check point: the outer mode A sampling folds the raw delayed requests of an inner mode A
 * leaf on a day the inner itself does not sample (adjust cycles 2 vs 3) */
TEST_CASE("test_MultiSystem_cascade_a_a_unsampled_fold") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 10);

    // The 3rd close: the outer (cycle 2) samples, the inner (cycle 3) does not, and the leaf
    // request of this very close is still pending (its delayed fill only happens the next open)
    Datetime buy_date = k[2].datetime;
    auto sg = make_shared<ScriptedBuySellSG>(buy_date, Datetime(203001010000LL));
    auto leaf = SYS_Simple(crtTM(), MM_FixedCount(1000), EnvironmentPtr(), ConditionPtr(), sg);
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", true);

    auto inner = std::make_shared<MultiSystem>("aa_inner");
    inner->setMode("A");
    inner->setSubInitCash(100000.0);
    inner->setAdjustCycle(3);
    inner->add(leaf);

    auto outer = std::make_shared<MultiSystem>("aa_outer");
    outer->setTM(crtTM(Datetime(199911010000LL), 1000000.0));
    outer->setMode("A");
    outer->setSE(std::make_shared<SelectAllSE>(SystemList{inner}));
    outer->setAdjustCycle(2);
    outer->add(inner);
    outer->run(k);

    Datetime first_buy;
    for (const auto& tr : outer->getTM()->getTradeList()) {
        if (tr.business == BUSINESS_BUY) {
            first_buy = tr.datetime;
            break;
        }
    }

    /** @arg the intent is visible to the outer on the very day it samples */
    CHECK_UNARY(!first_buy.isNull());
    CHECK_EQ(first_buy, buy_date);
}

/** @par Check point: in mode A the retried suggestion leg merges with the new rebalancing batch
 * (the suspended stock is excluded from the new batch by its missing quote, no double counting) */
TEST_CASE("test_MultiSystem_mode_a_retry_merge") {
    Stock stk1 = getStock("sh600005");
    Stock stk2 = getStock("sh600000");
    REQUIRE(!stk1.isNull());
    REQUIRE(!stk2.isNull());
    KQuery full(Datetime(20000401), Datetime(20000731));
    KQuery query(Datetime(20000501), Datetime(20000720));
    KData k = stk2.getKData(query);
    REQUIRE(k.size() > 10);

    auto sg1 = make_shared<ScriptedBuySellSG>(Datetime(200006290000LL), Datetime(203001010000LL));
    auto leaf1 = SYS_Simple(crtTM(), MM_FixedCount(100), EnvironmentPtr(), ConditionPtr(), sg1);
    leaf1->setStock(stk1);
    leaf1->setParam<bool>("buy_delay", true);
    leaf1->setTO(stk1.getKData(full));

    auto sg2 = make_shared<ScriptedBuySellSG>(Datetime(200006300000LL), Datetime(203001010000LL));
    auto leaf2 = SYS_Simple(crtTM(), MM_FixedCount(100), EnvironmentPtr(), ConditionPtr(), sg2);
    leaf2->setStock(stk2);
    leaf2->setParam<bool>("buy_delay", true);
    leaf2->setTO(stk2.getKData(full));

    auto ms = std::make_shared<MultiSystem>("ms_a_retry_merge");
    ms->setTM(crtTM(Datetime(200005010000LL), 200000.0));
    ms->setMode("A");
    ms->setSE(std::make_shared<SelectAllSE>(SystemList{leaf1, leaf2}));
    ms->setTradeOnClose(false);
    ms->add(leaf1);
    ms->add(leaf2);
    ms->run(k);

    size_t buys1 = 0;
    Datetime buy1_date;
    for (const auto& tr : ms->getTM()->getTradeList()) {
        if (tr.business == BUSINESS_BUY && tr.stock == stk1) {
            buys1++;
            buy1_date = tr.datetime;
        }
    }

    /** @arg the suspended intent fills once on the next quoted open, together with the new leg */
    CHECK_EQ(buys1, 1);
    CHECK_EQ(buy1_date, Datetime(200007030000LL));
    CHECK_GT(ms->getTM()->getPosition(k.back().datetime, stk1).number, 0.0);
    CHECK_GT(ms->getTM()->getPosition(k.back().datetime, stk2).number, 0.0);
}

/** @par Check point: a deferred injection waits for the deferred sells of the same rebalancing
 * when the sells are retried for a suspension (the freed cash is part of the injection funding) */
TEST_CASE("test_MultiSystem_mode_b_deferred_inject_waits_for_retry") {
    Stock stk1 = getStock("sh600005");
    Stock stk2 = getStock("sz000001");
    Stock market = getStock("sh600000");
    REQUIRE(!stk1.isNull());
    REQUIRE(!stk2.isNull());
    KQuery full1(Datetime(20000401), Datetime(20000731));
    KQuery full2(Datetime(20000401), Datetime(20000731));
    KQuery q1(Datetime(20000501), Datetime(20000628));
    KQuery q2(Datetime(20000629), Datetime(20000720));
    KData k1 = market.getKData(q1);
    KData k2 = market.getKData(q2);
    REQUIRE(k1.size() > 5);
    REQUIRE(k2.size() > 3);

    auto leaf1 = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf1->setStock(stk1);
    leaf1->setParam<bool>("buy_delay", false);
    leaf1->setTO(stk1.getKData(full1));
    auto leaf2 = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf2->setStock(stk2);
    leaf2->setParam<bool>("buy_delay", false);
    leaf2->setTO(stk2.getKData(full2));

    auto ms = std::make_shared<MultiSystem>("ms_inject_after_retry");
    ms->setTM(crtTM(Datetime(200005010000LL), 100000.0));
    ms->setMode("B");
    ms->setSE(std::make_shared<SelectAllSE>(SystemList{leaf1, leaf2}));
    ms->setTradeOnClose(false);
    ms->add(leaf1);
    ms->add(leaf2);
    ms->run(k1);

    double leaf2_before = leaf2->getTM()->getPosition(k2[0].datetime, stk2).number;
    REQUIRE_GT(leaf2_before, 0.0);

    /** @arg the injection lands on the day the suspended sell finally fills (2000-07-03) */
    ms->setAF(AF_FixedWeightList({0.1, 0.9}));
    ms->run(k2, false);
    double leaf2_after = leaf2->getTM()->getPosition(k2[2].datetime, stk2).number;
    double leaf1_after = leaf1->getTM()->getPosition(k2[2].datetime, stk1).number;
    double leaf1_before = leaf1->getTM()->getPosition(k2[0].datetime, stk1).number;
    CHECK_GT(leaf2_after, leaf2_before);
    CHECK_LT(leaf1_after, leaf1_before);
}

/** @par Check point: a calendar adjust_mode with trade_on_close=false still records the monthly
 * rebalancing turnover (the adjust decision comes from the close stage, never re-derived) */
TEST_CASE("test_MultiSystem_mode_a_calendar_turnover_deferred") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 10);

    auto leaf =
      SYS_Simple(crtTM(), MM_FixedCount(100), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", false);
    auto ms = std::make_shared<MultiSystem>("ms_a_calendar_turnover");
    ms->setTM(crtTM(Datetime(200001010000LL), 1000000.0));
    ms->setMode("A");
    ms->setSE(std::make_shared<SelectAllSE>(SystemList{leaf}));
    ms->setAdjustMode("month");
    ms->setTradeOnClose(false);
    ms->add(leaf);
    ms->run(k);

    size_t i0 = k.size();
    for (size_t i = 0; i < k.size(); ++i) {
        if (k[i].datetime >= Datetime(200001010000LL)) {
            i0 = i;
            break;
        }
    }
    REQUIRE(i0 < k.size() - 2);

    /** @arg the first monthly rebalance fills at the next open and is recorded */
    REQUIRE(!ms->getAdjustTurnover().empty());
    CHECK_EQ(ms->getAdjustTurnover().front().first, k[i0].datetime);
    CHECK_GE(ms->getAdjustTurnover().size(), 2);
}

/** @par Check point: the continuous-mirror fills of the non-rebalancing days are not counted into
 * the turnover (only the batches queued on the explicit rebalancing closes are) */
TEST_CASE("test_MultiSystem_mode_b_mirror_not_in_turnover") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery full(Datetime(19991110), Datetime(20000225));
    KQuery q1(Datetime(19991110), Datetime(20000121));
    KQuery q2(Datetime(20000124), Datetime(20000225));
    KData k1 = stk.getKData(q1);
    KData k2 = stk.getKData(q2);
    REQUIRE(k1.size() > 8);
    REQUIRE(k2.size() > 8);

    auto leaf =
      SYS_Simple(crtTM(), MM_FixedCount(100), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", false);
    leaf->setTO(stk.getKData(full));
    auto ms = std::make_shared<MultiSystem>("ms_b_mirror_turnover");
    ms->setTM(crtTM(Datetime(199911010000LL), 1000000.0));
    ms->setMode("B");
    ms->setSE(std::make_shared<SelectAllSE>(SystemList{leaf}));
    ms->setTradeOnClose(false);
    ms->add(leaf);
    ms->run(k1);

    size_t base = ms->getAdjustTurnover().size();

    /** @arg only the fills of the batches queued on the two rebalancing closes are recorded */
    ms->setAdjustDates(DatetimeList{k2[3].datetime, k2[7].datetime});
    ms->run(k2, false);
    const auto& turnover = ms->getAdjustTurnover();
    REQUIRE_GE(turnover.size() - base, 2);
    // The last two records are the two rebalancing batches (an earlier one may carry the last
    // batch of the first run, executed at the first open of the second run)
    CHECK_EQ(turnover[turnover.size() - 2].first, k2[3].datetime);
    CHECK_EQ(turnover[turnover.size() - 1].first, k2[7].datetime);
}

/** @par Check point: the trade_on_close=true rebalancing records its turnover at the close stage
 * (the master path of the turnover attribution) */
TEST_CASE("test_MultiSystem_mode_b_close_turnover") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery full(Datetime(19991110), Datetime(20000225));
    KQuery q1(Datetime(19991110), Datetime(20000121));
    KQuery q2(Datetime(20000124), Datetime(20000225));
    KData k1 = stk.getKData(q1);
    KData k2 = stk.getKData(q2);
    REQUIRE(k1.size() > 8);
    REQUIRE(k2.size() > 8);

    auto leaf = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", false);
    leaf->setTO(stk.getKData(full));
    auto ms = std::make_shared<MultiSystem>("ms_b_close_turnover");
    ms->setTM(crtTM(Datetime(199911010000LL), 100000.0));
    ms->setMode("B");
    ms->setSE(std::make_shared<SelectAllSE>(SystemList{leaf}));
    ms->add(leaf);
    ms->run(k1);

    size_t base = ms->getAdjustTurnover().size();

    /** @arg the close-stage reduction of the shrunk budget is recorded on its own close day */
    ms->setAF(AF_FixedAmount(30000.0));
    ms->run(k2, false);
    REQUIRE_GT(ms->getAdjustTurnover().size(), base);
    CHECK_EQ(ms->getAdjustTurnover()[base].first, k2.front().datetime);
    CHECK_GT(ms->getAdjustTurnover()[base].second, 0.0);
}

/** @par Check point: a stale injection of an earlier round does not re-inflate a child whose quota
 * was lowered while a sell leg was still waiting for a quote */
TEST_CASE("test_MultiSystem_mode_b_stale_inject_purged") {
    Stock stk1 = getStock("sh600005");
    Stock stk2 = getStock("sz000001");
    Stock market = getStock("sh600000");
    REQUIRE(!stk1.isNull());
    REQUIRE(!stk2.isNull());
    KQuery full1(Datetime(20000401), Datetime(20000731));
    KQuery full2(Datetime(20000401), Datetime(20000731));
    KQuery q1(Datetime(20000501), Datetime(20000626));
    KQuery q2(Datetime(20000629), Datetime(20000630));
    KQuery q3(Datetime(20000630), Datetime(20000720));
    KData k1 = market.getKData(q1);
    KData k2 = market.getKData(q2);
    KData k3 = market.getKData(q3);
    REQUIRE(k1.size() > 5);
    REQUIRE_GE(k2.size(), 1);
    REQUIRE_GT(k3.size(), 5);
    // The second run intentionally re-drives the 06-30 close: the next rebalancing round must
    // happen while the retry of the suspended leg is still pending
    REQUIRE_EQ(k3.front().datetime, Datetime(200006300000LL));

    auto leaf1 = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf1->setStock(stk1);
    leaf1->setParam<bool>("buy_delay", false);
    leaf1->setTO(stk1.getKData(full1));
    auto leaf2 = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf2->setStock(stk2);
    leaf2->setParam<bool>("buy_delay", false);
    leaf2->setTO(stk2.getKData(full2));

    auto ms = std::make_shared<MultiSystem>("ms_stale_inject");
    ms->setTM(crtTM(Datetime(200005010000LL), 200000.0));
    ms->setMode("B");
    ms->setSE(std::make_shared<SelectAllSE>(SystemList{leaf1, leaf2}));
    ms->setTradeOnClose(false);
    ms->add(leaf1);
    ms->add(leaf2);
    ms->run(k1);

    // The 06-29 close: leaf1 (suspended on 06-30) is over quota -> a stuck sell leg; leaf2 is
    // under quota -> an injection with the high quota
    ms->setAF(AF_FixedWeightList({0.1, 0.9}));
    ms->run(k2, false);

    double leaf2_before = leaf2->getTM()->getPosition(k3[0].datetime, stk2).number;
    REQUIRE_GT(leaf2_before, 0.0);

    // The 06-30 close lowers leaf2's quota while leaf1's sell leg still waits: the stale high
    // injection of leaf2 must be purged, otherwise it re-inflates leaf2 after its own sells.
    // The weight list is used as given (no normalization): 45% / 5% of the parent assets
    ms->setAF(AF_FixedWeightList({0.45, 0.05}));
    ms->run(k3, false);

    // Measured on the day the stuck sell finally fills (07-03): the stale injection would show
    // up exactly here, re-inflating leaf2 right after its own sells
    Datetime retry_day(200007030000LL);
    double leaf2_after = leaf2->getTM()->getPosition(retry_day, stk2).number;
    double leaf2_nav = leaf2->getTM()->getFunds(retry_day, KQuery::DAY).net_assets();

    /** @arg leaf2 is near its lowered quota on the retry day, not re-inflated by the stale target
     */
    CHECK_LT(leaf2_after, leaf2_before);
    CHECK_LT(leaf2_nav, 100000.0);
}

/** @par Check point: a selected sub-system with a zero quota is cleared like an unselected one
 * (the calibrated flow keeps the same liquidation semantics as the legacy flow) */
TEST_CASE("test_MultiSystem_mode_b_zero_quota_clears") {
    Stock stk1 = getStock("sh600000");
    Stock stk2 = getStock("sz000001");
    REQUIRE(!stk1.isNull());
    REQUIRE(!stk2.isNull());
    KQuery full1(Datetime(19991110), Datetime(20000225));
    KQuery full2(Datetime(19991110), Datetime(20000225));
    KQuery q1(Datetime(19991110), Datetime(20000120));
    KQuery q2(Datetime(20000121), Datetime(20000225));
    KData k1 = stk1.getKData(q1);
    KData k2 = stk1.getKData(q2);
    REQUIRE(k1.size() > 5);
    REQUIRE(k2.size() > 5);

    auto leaf1 = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf1->setStock(stk1);
    leaf1->setParam<bool>("buy_delay", false);
    leaf1->setTO(stk1.getKData(full1));
    auto leaf2 = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf2->setStock(stk2);
    leaf2->setParam<bool>("buy_delay", false);
    leaf2->setTO(stk2.getKData(full2));

    auto ms = std::make_shared<MultiSystem>("ms_b_zero_quota");
    ms->setTM(crtTM(Datetime(199911010000LL), 200000.0));
    ms->setMode("B");
    ms->setSE(std::make_shared<SelectAllSE>(SystemList{leaf1, leaf2}));
    ms->setSellAtNotSelected(true);
    ms->add(leaf1);
    ms->add(leaf2);
    ms->run(k1);

    REQUIRE_GT(leaf2->getTM()->getPosition(k1.back().datetime, stk2).number, 0.0);

    /** @arg a zero quota clears the sub ledger and the parent holdings */
    ms->setAF(AF_FixedWeightList({1.0, 0.0}));
    ms->run(k2, false);
    CHECK_EQ(leaf2->getTM()->getPosition(k2.back().datetime, stk2).number, 0.0);
    CHECK_EQ(ms->getTM()->getPosition(k2.back().datetime, stk2).number, 0.0);
    CHECK_GT(leaf1->getTM()->getPosition(k2.back().datetime, stk1).number, 0.0);
}

/** @par Check point: a pending sell leg whose holding was force-cleared while it waited (a
 * delisting) is dropped instead of blocking the injections forever */
TEST_CASE("test_MultiSystem_dead_leg_dropped_after_force_clear") {
    Stock stk1 = getStock("sh600005");
    Stock stk2 = getStock("sz000001");
    Stock market = getStock("sh600000");
    REQUIRE(!stk1.isNull());
    REQUIRE(!stk2.isNull());
    KQuery full1(Datetime(20000401), Datetime(20000731));
    KQuery full2(Datetime(20000401), Datetime(20000731));
    KQuery q1(Datetime(20000501), Datetime(20000626));
    KQuery q2(Datetime(20000629), Datetime(20000630));
    KQuery q3(Datetime(20000630), Datetime(20000701));
    KData k1 = market.getKData(q1);
    KData k2 = market.getKData(q2);
    KData k3 = market.getKData(q3);
    REQUIRE(k1.size() > 5);
    REQUIRE_GE(k2.size(), 1);
    REQUIRE_GE(k3.size(), 1);

    auto leaf1 = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf1->setStock(stk1);
    leaf1->setParam<bool>("buy_delay", false);
    leaf1->setTO(stk1.getKData(full1));
    auto leaf2 = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf2->setStock(stk2);
    leaf2->setParam<bool>("buy_delay", false);
    leaf2->setTO(stk2.getKData(full2));

    auto ms = std::make_shared<MultiSystem>("ms_dead_leg");
    ms->setTM(crtTM(Datetime(200005010000LL), 200000.0));
    ms->setMode("B");
    ms->setSE(std::make_shared<SelectAllSE>(SystemList{leaf1, leaf2}));
    ms->setSellAtNotSelected(true);
    ms->setTradeOnClose(false);
    ms->add(leaf1);
    ms->add(leaf2);
    ms->run(k1);

    // The 06-29 close: leaf1 gets a zero quota (a deferred clear), leaf2 stays under quota
    // (a deferred injection waiting for the sells)
    ms->setAF(AF_FixedWeightList({0.0, 1.0}));
    ms->run(k2, false);

    // Simulate the force-clear of a delisting arriving before the suspended open
    Datetime d = k2.back().datetime;
    double sub_num = leaf1->getTM()->getPosition(d, stk1).number;
    double parent_num = ms->getTM()->getPosition(d, stk1).number;
    REQUIRE_GT(sub_num, 0.0);
    leaf1->getTM()->sell(d, stk1, 1.0, sub_num, 0.0, 0.0, 1.0, PART_SYSTEM, "force clear");
    if (parent_num > 0.0) {
        ms->getTM()->sell(d, stk1, 1.0, parent_num, 0.0, 0.0, 1.0, PART_SYSTEM, "force clear");
    }

    double pos2_before = leaf2->getTM()->getPosition(d, stk2).number;

    // The 06-30 open: the instrument is suspended and its holding is gone -> the legs are dropped,
    // the deferred injection is no longer blocked
    ms->run(k3, false);

    size_t dead_sells = 0;
    for (const auto& tr : ms->getTM()->getTradeList()) {
        if (tr.business == BUSINESS_SELL && tr.stock == stk1 &&
            tr.datetime == Datetime(200006300000LL)) {
            dead_sells++;
        }
    }
    CHECK_EQ(dead_sells, 0);
    // The freed injection reached leaf2: it bought with the injected cash on the same close
    CHECK_GT(leaf2->getTM()->getPosition(Datetime(200006300000LL), stk2).number, pos2_before);
}

/** @par Check point: a zero-quota liquidation in trade_on_close=true does not mirror the cleared
 * sub open-buffer fills (the parent would buy back what the liquidation just sold) */
TEST_CASE("test_MultiSystem_mode_b_zero_quota_no_rebuy") {
    Stock stk1 = getStock("sh600000");
    Stock stk2 = getStock("sz000001");
    REQUIRE(!stk1.isNull());
    REQUIRE(!stk2.isNull());
    KQuery full1(Datetime(19991110), Datetime(20000225));
    KQuery full2(Datetime(19991110), Datetime(20000225));
    KQuery q1(Datetime(19991110), Datetime(20000121));
    KQuery q2(Datetime(20000124), Datetime(20000225));
    KData k1 = stk1.getKData(q1);
    KData k2 = stk1.getKData(q2);
    REQUIRE(k1.size() > 5);
    REQUIRE(k2.size() > 5);

    auto leaf1 = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf1->setStock(stk1);
    leaf1->setParam<bool>("buy_delay", false);
    leaf1->setTO(stk1.getKData(full1));
    auto leaf2 =
      SYS_Simple(crtTM(), MM_FixedCount(100), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf2->setStock(stk2);
    leaf2->setParam<bool>("buy_delay", true);
    leaf2->setTO(stk2.getKData(full2));

    auto ms = std::make_shared<MultiSystem>("ms_b_zero_quota_no_rebuy");
    ms->setTM(crtTM(Datetime(199911010000LL), 200000.0));
    ms->setMode("B");
    ms->setSE(std::make_shared<SelectAllSE>(SystemList{leaf1, leaf2}));
    ms->setSellAtNotSelected(true);
    ms->add(leaf1);
    ms->add(leaf2);
    ms->run(k1);

    /** @arg the morning fill of the cleared sub is not mirrored after the liquidation sells */
    ms->setAF(AF_FixedWeightList({1.0, 0.0}));
    ms->run(k2, false);
    Datetime clear_day = k2.front().datetime;
    bool morning_fill = false;
    for (const auto& tr : leaf2->getTM()->getTradeList()) {
        if (tr.business == BUSINESS_BUY && tr.datetime == clear_day) {
            morning_fill = true;
        }
    }
    size_t parent_buys = 0;
    for (const auto& tr : ms->getTM()->getTradeList()) {
        if (tr.business == BUSINESS_BUY && tr.stock == stk2 && tr.datetime == clear_day) {
            parent_buys++;
        }
    }
    /** @arg the cleared sub did have an open-buffer fill, and it was not mirrored */
    CHECK_UNARY(morning_fill);
    CHECK_EQ(parent_buys, 0);
    CHECK_EQ(leaf2->getTM()->getPosition(k2.back().datetime, stk2).number, 0.0);
    CHECK_EQ(ms->getTM()->getPosition(k2.back().datetime, stk2).number, 0.0);
    CHECK_GT(leaf1->getTM()->getPosition(k2.back().datetime, stk1).number, 0.0);
}

/** @par Check point: a parent-side residual that can never fill (both books flat) is dropped, so
 * the deferred injections of the later rounds are not blocked forever */
TEST_CASE("test_MultiSystem_mode_b_residual_terminated") {
    Stock stk1 = getStock("sh600000");
    Stock stk2 = getStock("sz000001");
    REQUIRE(!stk1.isNull());
    REQUIRE(!stk2.isNull());
    KQuery q1(Datetime(20000501), Datetime(20000627));
    KQuery q2(Datetime(20000629), Datetime(20000707));
    KData k1 = stk1.getKData(q1);
    KData k2 = stk1.getKData(q2);
    REQUIRE(k1.size() > 5);
    REQUIRE(k2.size() > 3);

    auto leaf1 = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf1->setStock(stk1);
    leaf1->setParam<bool>("buy_delay", false);
    auto leaf2 = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf2->setStock(stk2);
    leaf2->setParam<bool>("buy_delay", false);

    auto ms = std::make_shared<MultiSystem>("ms_residual");
    ms->setTM(crtTM(Datetime(200005010000LL), 100000.0));
    ms->setMode("B");
    ms->setSE(std::make_shared<SelectAllSE>(SystemList{leaf1, leaf2}));
    ms->setTradeOnClose(false);
    ms->add(leaf1);
    ms->add(leaf2);
    ms->run(k1);

    // Keep only one lot on the parent book: the next reduction leg can only partially fill and
    // leaves a residual that can never fill on a flat parent book
    Datetime drain_date = k1.back().datetime;
    double parent_num = ms->getTM()->getPosition(drain_date, stk1).number;
    REQUIRE_GT(parent_num, 100.0);
    price_t drain_price = k1.back().closePrice;
    ms->getTM()->sell(drain_date, stk1, drain_price, parent_num - 100.0, 0.0, 0.0, drain_price,
                      PART_SYSTEM, "drain");

    double leaf2_cash_before = leaf2->getTM()->getFunds(drain_date, KQuery::DAY).cash;
    double leaf1_before = leaf1->getTM()->getPosition(drain_date, stk1).number;

    /** @arg the residual dies on the flat parent book and the injection is not blocked */
    ms->setAF(AF_FixedWeightList({0.1, 0.7}));
    ms->run(k2, false);

    size_t late_sells = 0;
    for (const auto& tr : ms->getTM()->getTradeList()) {
        if (tr.business == BUSINESS_SELL && tr.stock == stk1 &&
            tr.datetime > Datetime(200006300000LL)) {
            late_sells++;
        }
    }
    CHECK_LT(leaf1->getTM()->getPosition(k2.back().datetime, stk1).number, leaf1_before);
    CHECK_EQ(ms->getTM()->getPosition(k2.back().datetime, stk1).number, 0.0);
    CHECK_EQ(late_sells, 0);
    CHECK_GT(leaf2->getTM()->getFunds(Datetime(200006300000LL), KQuery::DAY).cash,
             leaf2_cash_before);
}

TEST_CASE("test_MultiSystem_mode_b_legacy_zero_quota") {
    Stock stk1 = getStock("sh600000");
    Stock stk2 = getStock("sz000001");
    REQUIRE(!stk1.isNull());
    REQUIRE(!stk2.isNull());
    KQuery full1(Datetime(19991110), Datetime(20000225));
    KQuery full2(Datetime(19991110), Datetime(20000225));
    KQuery q1(Datetime(19991110), Datetime(20000120));
    KQuery q2(Datetime(20000121), Datetime(20000225));
    KData k1 = stk1.getKData(q1);
    KData k2 = stk1.getKData(q2);
    REQUIRE(k1.size() > 5);
    REQUIRE(k2.size() > 5);

    auto leaf1 = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf1->setStock(stk1);
    leaf1->setParam<bool>("buy_delay", false);
    leaf1->setTO(stk1.getKData(full1));
    auto leaf2 =
      SYS_Simple(crtTM(), MM_FixedCount(100), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf2->setStock(stk2);
    leaf2->setParam<bool>("buy_delay", false);
    leaf2->setTO(stk2.getKData(full2));

    auto ms = std::make_shared<MultiSystem>("ms_b_legacy_zero_quota");
    ms->setTM(crtTM(Datetime(199911010000LL), 200000.0));
    ms->setMode("B");
    ms->setSellAtNotSelected(true);
    ms->add(leaf1);
    ms->add(leaf2);
    ms->run(k1);

    REQUIRE_GT(leaf2->getTM()->getPosition(k1.back().datetime, stk2).number, 0.0);

    /** @arg the zero quota clears both books, without replicating the sub same-close fills */
    ms->setAF(AF_FixedWeightList({1.0, 0.0}));
    ms->run(k2, false);
    CHECK_EQ(leaf2->getTM()->getPosition(k2.back().datetime, stk2).number, 0.0);
    CHECK_EQ(ms->getTM()->getPosition(k2.back().datetime, stk2).number, 0.0);
    CHECK_GT(leaf1->getTM()->getPosition(k2.back().datetime, stk1).number, 0.0);
}

/** @par Check point: a nested composite selected with a zero quota is liquidated recursively
 * (the leaf ledger, the inner account and the outer parent all flat) */
TEST_CASE("test_MultiSystem_mode_b_zero_quota_nested_composite") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery full(Datetime(19991110), Datetime(20000225));
    KQuery q1(Datetime(19991110), Datetime(20000120));
    KQuery q2(Datetime(20000121), Datetime(20000225));
    KData k1 = stk.getKData(q1);
    KData k2 = stk.getKData(q2);
    REQUIRE(k1.size() > 5);
    REQUIRE(k2.size() > 5);

    auto leaf = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", false);
    leaf->setTO(stk.getKData(full));
    auto other = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    other->setStock(getStock("sz000001"));
    other->setParam<bool>("buy_delay", false);
    auto inner = std::make_shared<MultiSystem>("inner_b_zero_quota");
    inner->setMode("B");
    inner->setSE(std::make_shared<SelectAllSE>(SystemList{leaf}));
    inner->add(leaf);

    auto ms = std::make_shared<MultiSystem>("outer_b_zero_quota");
    ms->setTM(crtTM(Datetime(199911010000LL), 100000.0));
    ms->setMode("B");
    ms->setSE(std::make_shared<SelectAllSE>(SystemList{inner, other}));
    ms->setSellAtNotSelected(true);
    ms->add(inner);
    ms->add(other);
    ms->run(k1);

    REQUIRE_GT(leaf->getTM()->getPosition(k1.back().datetime, stk).number, 0.0);

    /** @arg the whole nested subtree is cleared together with the outer parent holding */
    ms->setAF(AF_FixedWeightList({0.0, 1.0}));
    ms->run(k2, false);
    CHECK_EQ(leaf->getTM()->getPosition(k2.back().datetime, stk).number, 0.0);
    CHECK_EQ(inner->getTM()->getPosition(k2.back().datetime, stk).number, 0.0);
    CHECK_EQ(ms->getTM()->getPosition(k2.back().datetime, stk).number, 0.0);
}

/** @par Check point: adjust_running_sys=false keeps a running sub with a zero quota untouched */
TEST_CASE("test_MultiSystem_mode_b_zero_quota_keeps_running") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery full(Datetime(19991110), Datetime(20000225));
    KQuery q1(Datetime(19991110), Datetime(20000120));
    KQuery q2(Datetime(20000121), Datetime(20000225));
    KData k1 = stk.getKData(q1);
    KData k2 = stk.getKData(q2);
    REQUIRE(k1.size() > 5);
    REQUIRE(k2.size() > 5);

    // A fixed-count MM keeps cash, so the sub keeps trading while running: the test can tell
    // "still driven and mirrored" apart from "skipped"
    auto leaf =
      SYS_Simple(crtTM(), MM_FixedCount(100), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", false);
    leaf->setTO(stk.getKData(full));
    auto other = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    other->setStock(getStock("sz000001"));
    other->setParam<bool>("buy_delay", false);
    auto ms = std::make_shared<MultiSystem>("ms_b_zero_quota_running");
    // A large parent budget keeps the fixed-count sub funded through both runs
    ms->setTM(crtTM(Datetime(199911010000LL), 1000000.0));
    ms->setMode("B");
    ms->setSE(std::make_shared<SelectAllSE>(SystemList{leaf, other}));
    ms->setSellAtNotSelected(true);
    ms->add(leaf);
    ms->add(other);
    ms->run(k1);
    double pos1 = leaf->getTM()->getPosition(k1.back().datetime, stk).number;
    REQUIRE_GT(pos1, 0.0);

    /** @arg the running sub is left untouched despite the zero quota: not liquidated, still
     * driven and mirrored */
    auto af = AF_FixedWeightList({0.0, 1.0});
    af->setParam<bool>("adjust_running_sys", false);
    ms->setAF(af);
    ms->run(k2, false);
    double leaf_pos2 = leaf->getTM()->getPosition(k2.back().datetime, stk).number;
    double parent_pos2 = ms->getTM()->getPosition(k2.back().datetime, stk).number;
    CHECK_GT(leaf_pos2, pos1);
    CHECK_GT(parent_pos2, pos1);
    CHECK_EQ(leaf_pos2, doctest::Approx(parent_pos2));
}

class SelectBeforeDateSE : public SelectorBase {
public:
    SelectBeforeDateSE(const SystemList& list, Datetime stop_date)
    : SelectorBase("SelectBeforeDateSE"), m_list(list), m_stop_date(stop_date) {}

    SelectorPtr _clone() override {
        return std::make_shared<SelectBeforeDateSE>(m_list, m_stop_date);
    }

    void _calculate() override {}

    SystemWeightList _getSelected(Datetime date) override {
        SystemWeightList ret;
        if (date < m_stop_date) {
            for (auto& sys : m_list) {
                ret.emplace_back(sys, 1.0);
            }
        }
        return ret;
    }

    SystemList m_list;
    Datetime m_stop_date;
};

/** @par Check point: a trade_on_close=false liquidation on a suspended close still queues the
 * parent leg (the sub side is queued too), so the parent holding is not stranded */
TEST_CASE("test_MultiSystem_liquidation_parent_leg_on_suspended_close") {
    Stock stk = getStock("sh600005");
    Stock market = getStock("sh600000");
    REQUIRE(!stk.isNull());
    REQUIRE(!market.isNull());
    KQuery full(Datetime(20000401), Datetime(20000731));
    KQuery q1(Datetime(20000501), Datetime(20000630));
    KQuery q2(Datetime(20000630), Datetime(20000707));
    KData k1 = market.getKData(q1);
    KData k2 = market.getKData(q2);
    REQUIRE(k1.size() > 10);
    REQUIRE(k2.size() > 2);
    // Premise: the instrument is suspended on the clearing close
    REQUIRE_UNARY(stk.getKData(KQuery(Datetime(20000630), Datetime(20000701))).empty());

    auto leaf = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", false);
    leaf->setTO(stk.getKData(full));

    auto short_sg =
      make_shared<ScriptedShortSG>(Datetime(200006150000LL), Datetime(203001010000LL));
    short_sg->setParam<bool>("support_borrow_stock", true);
    auto short_leaf =
      SYS_Simple(crtTM(), MM_FixedCount(100), EnvironmentPtr(), ConditionPtr(), short_sg);
    short_leaf->setStock(stk);
    short_leaf->setParam<bool>("buy_delay", false);
    short_leaf->setParam<bool>("sell_delay", false);
    short_leaf->setParam<bool>("support_borrow_stock", true);
    short_leaf->setTO(stk.getKData(full));

    auto other = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    other->setStock(getStock("sz000001"));
    other->setParam<bool>("buy_delay", false);

    // From the clearing close on, only `other` stays selected: the two cleared subs are out of
    // the allocation for good (no re-buying after the liquidation legs fill)
    class SelectOthersAfterSE : public SelectorBase {
    public:
        SelectOthersAfterSE(const SystemList& list, const SystemPtr& keep, Datetime stop_date)
        : SelectorBase("SelectOthersAfterSE"), m_list(list), m_keep(keep), m_stop_date(stop_date) {
            m_pro_sys_list = list;
        }

        SelectorPtr _clone() override {
            return std::make_shared<SelectOthersAfterSE>(m_list, m_keep, m_stop_date);
        }

        void _calculate() override {}

        SystemWeightList _getSelected(Datetime date) override {
            SystemWeightList ret;
            for (auto& sys : m_list) {
                if (date >= m_stop_date && sys != m_keep) {
                    continue;
                }
                ret.emplace_back(sys, 1.0);
            }
            return ret;
        }

        SystemList m_list;
        SystemPtr m_keep;
        Datetime m_stop_date;
    };

    auto ms = std::make_shared<MultiSystem>("ms_liq_suspended");
    ms->setTM(crtTM(Datetime(200005010000LL), 300000.0));
    ms->setMode("B");
    ms->setSE(std::make_shared<SelectOthersAfterSE>(SystemList{leaf, short_leaf, other}, other,
                                                    Datetime(200006300000LL)));
    ms->setSellAtNotSelected(true);
    ms->setTradeOnClose(false);
    ms->setParam<bool>("support_borrow_stock", true);
    ms->add(leaf);
    ms->add(short_leaf);
    ms->add(other);
    ms->run(k1);

    /** @arg both books are open before the suspended clearing close */
    double leaf_pos1 = leaf->getTM()->getPosition(k1.back().datetime, stk).number;
    double short_pos1 = short_leaf->getTM()->getShortPosition(stk).number;
    REQUIRE_GT(leaf_pos1, 0.0);
    REQUIRE_GT(short_pos1, 0.0);
    REQUIRE_GT(ms->getTM()->getPosition(k1.back().datetime, stk).number, 0.0);
    REQUIRE_GT(ms->getTM()->getShortPosition(stk).number, 0.0);

    ms->run(k2, false);

    /** @arg both books of both cleared subs are flat: the parent legs waited for a quote and
     * retried instead of being dropped on the suspended close */
    CHECK_EQ(leaf->getTM()->getPosition(k2.back().datetime, stk).number, 0.0);
    CHECK_EQ(ms->getTM()->getPosition(k2.back().datetime, stk).number, 0.0);
    CHECK_EQ(short_leaf->getTM()->getShortPosition(stk).number, 0.0);
    CHECK_EQ(ms->getTM()->getShortPosition(stk).number, 0.0);
}

TEST_CASE("test_MultiSystem_clear_purges_stale_retry_batch") {
    Stock stk = getStock("sh600005");
    Stock market = getStock("sh600000");
    REQUIRE(!stk.isNull());
    REQUIRE(!market.isNull());
    KQuery full(Datetime(20000401), Datetime(20000731));
    KQuery query(Datetime(20000601), Datetime(20000707));
    KData k = market.getKData(query);
    REQUIRE(k.size() > 6);

    auto sg = make_shared<ScriptedBuySellSG>(Datetime(200006290000LL), Datetime(203001010000LL));
    auto leaf = SYS_Simple(crtTM(), MM_FixedCount(100), EnvironmentPtr(), ConditionPtr(), sg);
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", false);
    leaf->setTO(stk.getKData(full));

    auto ms = std::make_shared<MultiSystem>("ms_clear_purge");
    ms->setTM(crtTM(Datetime(200006010000LL), 100000.0));
    ms->setMode("B");
    ms->setSE(std::make_shared<SelectBeforeDateSE>(SystemList{leaf}, Datetime(200006300000LL)));
    ms->setSellAtNotSelected(true);
    ms->setTradeOnClose(false);
    ms->add(leaf);
    ms->run(k);

    /** @arg the queued mirror of the cleared sub is gone: the parent never buys it */
    CHECK_EQ(leaf->getTM()->getPosition(k.back().datetime, stk).number, 0.0);
    CHECK_EQ(ms->getTM()->getPosition(k.back().datetime, stk).number, 0.0);
}

/** @par Check point: a mode B outer follows a nested mode A child through its quota account
 * (the missing topology of the 3x3 matrix) */
TEST_CASE("test_MultiSystem_mode_b_nested_a_follows_account") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 6);

    auto leaf = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", false);
    auto inner = std::make_shared<MultiSystem>("ba_inner_a");
    inner->setMode("A");
    inner->setSubInitCash(100000.0);
    inner->add(leaf);

    auto outer = std::make_shared<MultiSystem>("ba_outer_b");
    outer->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    outer->setMode("B");
    outer->setSE(std::make_shared<SelectAllSE>(SystemList{inner}));
    outer->add(inner);
    outer->run(k);

    double parent = outer->getTM()->getPosition(k.back().datetime, stk).number;
    double inner_pos = inner->getTM()->getPosition(k.back().datetime, stk).number;

    /** @arg the parent mirrors the inner account one for one */
    CHECK_GT(parent, 0.0);
    CHECK_EQ(parent, doctest::Approx(inner_pos));
}

/** @par Check point: with sell_at_not_selected=false a selected zero-quota sub keeps running
 * (no liquidation, no budget) */
TEST_CASE("test_MultiSystem_mode_b_zero_quota_flag_off_keeps") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery full(Datetime(19991110), Datetime(20000225));
    KQuery q1(Datetime(19991110), Datetime(20000120));
    KQuery q2(Datetime(20000121), Datetime(20000225));
    KData k1 = stk.getKData(q1);
    KData k2 = stk.getKData(q2);
    REQUIRE(k1.size() > 5);
    REQUIRE(k2.size() > 5);

    auto leaf = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", false);
    leaf->setTO(stk.getKData(full));
    auto other = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    other->setStock(getStock("sz000001"));
    other->setParam<bool>("buy_delay", false);

    auto ms = std::make_shared<MultiSystem>("ms_b_zero_quota_flag_off");
    ms->setTM(crtTM(Datetime(199911010000LL), 100000.0));
    ms->setMode("B");
    ms->setSE(std::make_shared<SelectAllSE>(SystemList{leaf, other}));
    ms->add(leaf);
    ms->add(other);
    ms->run(k1);
    double pos1 = leaf->getTM()->getPosition(k1.back().datetime, stk).number;
    REQUIRE_GT(pos1, 0.0);

    /** @arg the zero-quota sub is neither liquidated nor reduced */
    ms->setAF(AF_FixedWeightList({0.0, 1.0}));
    ms->run(k2, false);
    CHECK_EQ(leaf->getTM()->getPosition(k2.back().datetime, stk).number, pos1);
    CHECK_EQ(ms->getTM()->getPosition(k2.back().datetime, stk).number, pos1);
}

/** @par Check point: a trade_on_close=false liquidation covers a short sub book and its parent
 * mirror at the next open */
TEST_CASE("test_MultiSystem_liquidation_short_cover_deferred") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 10);

    auto sg = make_shared<ScriptedShortSG>(k[1].datetime, Datetime(203001010000LL));
    sg->setParam<bool>("support_borrow_stock", true);
    auto leaf = SYS_Simple(crtTM(), MM_FixedCount(100), EnvironmentPtr(), ConditionPtr(), sg);
    leaf->setParam<bool>("buy_delay", false);
    leaf->setParam<bool>("sell_delay", false);
    leaf->setParam<bool>("support_borrow_stock", true);
    leaf->setStock(stk);

    auto ms = std::make_shared<MultiSystem>("ms_short_clear");
    ms->setTM(crtTM(Datetime(199911010000LL), 100000.0));
    ms->setMode("B");
    ms->setSE(std::make_shared<SelectBeforeDateSE>(SystemList{leaf}, k[6].datetime));
    ms->setSellAtNotSelected(true);
    ms->setTradeOnClose(false);
    ms->setParam<bool>("support_borrow_stock", true);
    ms->add(leaf);
    ms->run(k);

    /** @arg the short books are covered after the clear */
    CHECK_EQ(leaf->getTM()->getShortPosition(stk).number, 0.0);
    CHECK_EQ(ms->getTM()->getShortPosition(stk).number, 0.0);
}

/** @par Check point: the legacy (no SE) zero-quota path honors adjust_running_sys=false as well:
 * a running sub with a zero quota is left untouched */
TEST_CASE("test_MultiSystem_mode_b_legacy_zero_quota_keeps_running") {
    Stock stk1 = getStock("sh600000");
    Stock stk2 = getStock("sz000001");
    REQUIRE(!stk1.isNull());
    REQUIRE(!stk2.isNull());
    KQuery full1(Datetime(19991110), Datetime(20000225));
    KQuery full2(Datetime(19991110), Datetime(20000225));
    KQuery q1(Datetime(19991110), Datetime(20000120));
    KQuery q2(Datetime(20000121), Datetime(20000225));
    KData k1 = stk1.getKData(q1);
    KData k2 = stk1.getKData(q2);
    REQUIRE(k1.size() > 5);
    REQUIRE(k2.size() > 5);

    auto leaf1 = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf1->setStock(stk1);
    leaf1->setParam<bool>("buy_delay", false);
    leaf1->setTO(stk1.getKData(full1));
    auto leaf2 = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf2->setStock(stk2);
    leaf2->setParam<bool>("buy_delay", false);
    leaf2->setTO(stk2.getKData(full2));

    auto ms = std::make_shared<MultiSystem>("ms_b_legacy_zero_quota_running");
    ms->setTM(crtTM(Datetime(199911010000LL), 200000.0));
    ms->setMode("B");
    ms->setSellAtNotSelected(true);
    ms->add(leaf1);
    ms->add(leaf2);
    ms->run(k1);
    double pos1 = leaf1->getTM()->getPosition(k1.back().datetime, stk1).number;
    REQUIRE_GT(pos1, 0.0);

    /** @arg the running zero-quota sub is left untouched in the legacy flow */
    auto af = AF_FixedWeightList({0.0, 1.0});
    af->setParam<bool>("adjust_running_sys", false);
    ms->setAF(af);
    ms->run(k2, false);
    CHECK_EQ(leaf1->getTM()->getPosition(k2.back().datetime, stk1).number, pos1);
    CHECK_EQ(ms->getTM()->getPosition(k2.back().datetime, stk1).number, pos1);
    CHECK_GT(leaf2->getTM()->getPosition(k2.back().datetime, stk2).number, 0.0);
}

/** @par Check point: a trade_on_close=true liquidation on a suspended close falls back to the
 * deferred legs and completes at the next quoted open (the PF_Simple default path) */
TEST_CASE("test_MultiSystem_liquidation_suspended_close_trade_on_close") {
    Stock stk = getStock("sh600005");
    Stock market = getStock("sh600000");
    REQUIRE(!stk.isNull());
    REQUIRE(!market.isNull());
    KQuery full(Datetime(20000401), Datetime(20000731));
    KQuery q1(Datetime(20000501), Datetime(20000630));
    KQuery q2(Datetime(20000630), Datetime(20000707));
    KData k1 = market.getKData(q1);
    KData k2 = market.getKData(q2);
    REQUIRE(k1.size() > 10);
    REQUIRE(k2.size() > 2);
    REQUIRE_UNARY(stk.getKData(KQuery(Datetime(20000630), Datetime(20000701))).empty());

    auto leaf = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", false);
    leaf->setTO(stk.getKData(full));
    auto other = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    other->setStock(getStock("sz000001"));
    other->setParam<bool>("buy_delay", false);

    class SelectOthersAfterSE : public SelectorBase {
    public:
        SelectOthersAfterSE(const SystemList& list, const SystemPtr& keep, Datetime stop_date)
        : SelectorBase("SelectOthersAfterSE2"), m_list(list), m_keep(keep), m_stop_date(stop_date) {
            m_pro_sys_list = list;
        }

        SelectorPtr _clone() override {
            return std::make_shared<SelectOthersAfterSE>(m_list, m_keep, m_stop_date);
        }

        void _calculate() override {}

        SystemWeightList _getSelected(Datetime date) override {
            SystemWeightList ret;
            for (auto& sys : m_list) {
                if (date >= m_stop_date && sys != m_keep) {
                    continue;
                }
                ret.emplace_back(sys, 1.0);
            }
            return ret;
        }

        SystemList m_list;
        SystemPtr m_keep;
        Datetime m_stop_date;
    };

    auto ms = std::make_shared<MultiSystem>("ms_liq_suspended_toc");
    ms->setTM(crtTM(Datetime(200005010000LL), 300000.0));
    ms->setMode("B");
    ms->setSE(std::make_shared<SelectOthersAfterSE>(SystemList{leaf, other}, other,
                                                    Datetime(200006300000LL)));
    ms->setSellAtNotSelected(true);
    ms->add(leaf);
    ms->add(other);
    ms->run(k1);

    double pos1 = leaf->getTM()->getPosition(k1.back().datetime, stk).number;
    REQUIRE_GT(pos1, 0.0);
    REQUIRE_GT(ms->getTM()->getPosition(k1.back().datetime, stk).number, 0.0);

    ms->run(k2, false);

    /** @arg the liquidation legs were queued at the suspended close and filled later */
    CHECK_EQ(leaf->getTM()->getPosition(k2.back().datetime, stk).number, 0.0);
    CHECK_EQ(ms->getTM()->getPosition(k2.back().datetime, stk).number, 0.0);
}

/** @par Check point: a trade_on_close=false reduction on a suspended close still queues the
 * reduction legs and fills them at the next quoted open */
TEST_CASE("test_MultiSystem_reduction_on_suspended_close_deferred") {
    Stock stk = getStock("sh600005");
    Stock market = getStock("sh600000");
    REQUIRE(!stk.isNull());
    REQUIRE(!market.isNull());
    KQuery full(Datetime(20000401), Datetime(20000731));
    KQuery q1(Datetime(20000501), Datetime(20000630));
    KQuery q2(Datetime(20000630), Datetime(20000704));
    KData k1 = market.getKData(q1);
    KData k2 = market.getKData(q2);
    REQUIRE(k1.size() > 10);
    REQUIRE_GE(k2.size(), 2);
    REQUIRE_UNARY(stk.getKData(KQuery(Datetime(20000630), Datetime(20000701))).empty());

    auto leaf = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", false);
    leaf->setTO(stk.getKData(full));

    auto ms = std::make_shared<MultiSystem>("ms_reduce_suspended");
    ms->setTM(crtTM(Datetime(200005010000LL), 100000.0));
    ms->setMode("B");
    ms->setSE(std::make_shared<SelectAllSE>(SystemList{leaf}));
    ms->setTradeOnClose(false);
    ms->add(leaf);
    ms->run(k1);

    double pos1 = leaf->getTM()->getPosition(k1.back().datetime, stk).number;
    REQUIRE_GT(pos1, 0.0);

    /** @arg the suspended close queues the reduction, executed at the next open */
    ms->setAF(AF_FixedAmount(30000.0));
    ms->run(k2, false);
    CHECK_LT(leaf->getTM()->getPosition(k2.back().datetime, stk).number, pos1);
}

/** @par Check point: a delegated budget above the inner reserve cap (max_total_position) is
 * capped, so the inner layer keeps its reserved cash */
TEST_CASE("test_MultiSystem_cascade_b_b_b_delegated_reserve") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 6);

    auto leaf = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", false);
    auto inner = std::make_shared<MultiSystem>("bbb_inner");
    inner->setMode("B");
    inner->setSE(std::make_shared<SelectAllSE>(SystemList{leaf}));
    inner->getAF()->setParam<double>("max_total_position", 0.8);
    inner->add(leaf);

    auto middle = std::make_shared<MultiSystem>("bbb_middle");
    middle->setMode("B");
    middle->setSE(std::make_shared<SelectAllSE>(SystemList{inner}));
    middle->add(inner);

    auto outer = std::make_shared<MultiSystem>("bbb_outer");
    outer->setTM(crtTM(Datetime(200001010000LL), 100000.0));
    outer->setMode("B");
    outer->setSE(std::make_shared<SelectAllSE>(SystemList{middle}));
    outer->add(middle);
    outer->run(k);

    double leaf_nav = leaf->getTM()->getFunds(k.back().datetime, KQuery::DAY).net_assets();
    double parent_pos = outer->getTM()->getPosition(k.back().datetime, stk).number;
    double leaf_pos = leaf->getTM()->getPosition(k.back().datetime, stk).number;

    /** @arg the inner layer keeps its 20% reserve: the delegated 100% budget is capped to 80% */
    CHECK_GT(leaf_pos, 0.0);
    CHECK_LE(leaf_nav, 86000.0);
    CHECK_EQ(parent_pos, doctest::Approx(leaf_pos));
}

/** @par Check point: a three-level mode C chain surfaces the leaf trades once per layer and the
 * shared account holds exactly one buy and one sell */
TEST_CASE("test_MultiSystem_cascade_c_c_c_surface_once") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(19991110), Datetime(20000225));
    KData k = stk.getKData(query);
    REQUIRE(k.size() > 10);

    Datetime buy_date = k[k.size() - 4].datetime;
    Datetime sell_date = k[k.size() - 2].datetime;
    auto sg = make_shared<ScriptedBuySellSG>(buy_date, sell_date);
    auto leaf = SYS_Simple(crtTM(), MM_FixedCount(100), EnvironmentPtr(), ConditionPtr(), sg);
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", false);
    leaf->setParam<bool>("sell_delay", false);

    auto inner = std::make_shared<MultiSystem>("ccc_inner");
    inner->setMode("C");
    inner->add(leaf);
    auto middle = std::make_shared<MultiSystem>("ccc_middle");
    middle->setMode("C");
    middle->add(inner);
    auto outer = std::make_shared<MultiSystem>("ccc_outer");
    outer->setTM(crtTM(k[0].datetime, 100000.0));
    outer->setMode("C");
    outer->add(middle);
    outer->run(k);

    size_t buys = 0;
    size_t sells = 0;
    for (const auto& tr : outer->getTM()->getTradeList()) {
        if (tr.business == BUSINESS_BUY) {
            buys++;
        } else if (tr.business == BUSINESS_SELL) {
            sells++;
        }
    }

    /** @arg the shared account only sees the leaf trades once */
    CHECK_EQ(buys, 1);
    CHECK_EQ(sells, 1);
    CHECK_EQ(outer->getTM()->getPosition(k.back().datetime, stk).number, 0.0);
}

/** @par Check point: a zero quota on the middle mode B clears the whole nested subtree and the
 * outer parent holdings */
TEST_CASE("test_MultiSystem_cascade_b_b_a_zero_quota_middle") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery full(Datetime(19991110), Datetime(20000225));
    KQuery q1(Datetime(19991110), Datetime(20000120));
    KQuery q2(Datetime(20000121), Datetime(20000225));
    KData k1 = stk.getKData(q1);
    KData k2 = stk.getKData(q2);
    REQUIRE(k1.size() > 5);
    REQUIRE(k2.size() > 5);

    auto leaf = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", false);
    leaf->setTO(stk.getKData(full));
    auto inner = std::make_shared<MultiSystem>("bba_inner");
    inner->setMode("A");
    inner->setSubInitCash(100000.0);
    inner->add(leaf);
    auto middle = std::make_shared<MultiSystem>("bba_middle");
    middle->setMode("B");
    middle->setSE(std::make_shared<SelectAllSE>(SystemList{inner}));
    middle->add(inner);

    auto other = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    other->setStock(getStock("sz000001"));
    other->setParam<bool>("buy_delay", false);

    auto outer = std::make_shared<MultiSystem>("bba_outer");
    outer->setTM(crtTM(Datetime(199911010000LL), 100000.0));
    outer->setMode("B");
    outer->setSE(std::make_shared<SelectAllSE>(SystemList{middle, other}));
    outer->setSellAtNotSelected(true);
    outer->add(middle);
    outer->add(other);
    outer->run(k1);

    REQUIRE_GT(leaf->getTM()->getPosition(k1.back().datetime, stk).number, 0.0);

    /** @arg the middle zero quota clears the inner A shadow, the middle account and the parent */
    outer->setAF(AF_FixedWeightList({0.0, 1.0}));
    outer->run(k2, false);
    CHECK_EQ(leaf->getTM()->getPosition(k2.back().datetime, stk).number, 0.0);
    CHECK_EQ(inner->getTM()->getPosition(k2.back().datetime, stk).number, 0.0);
    CHECK_EQ(middle->getTM()->getPosition(k2.back().datetime, stk).number, 0.0);
    CHECK_EQ(outer->getTM()->getPosition(k2.back().datetime, stk).number, 0.0);
}

/** @par Check point: the A -> B -> A cascade still mirrors a suspended close reduction only once */
TEST_CASE("test_MultiSystem_cascade_a_b_a_suspended_deferred") {
    Stock stk = getStock("sh600005");
    Stock market = getStock("sh600000");
    REQUIRE(!stk.isNull());
    REQUIRE(!market.isNull());
    KQuery full(Datetime(20000401), Datetime(20000731));
    KQuery q1(Datetime(20000501), Datetime(20000630));
    KQuery q2(Datetime(20000630), Datetime(20000707));
    KData k1 = market.getKData(q1);
    KData k2 = market.getKData(q2);
    REQUIRE(k1.size() > 10);
    REQUIRE(k2.size() > 2);
    REQUIRE_UNARY(stk.getKData(KQuery(Datetime(20000630), Datetime(20000701))).empty());

    auto leaf = SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf->setStock(stk);
    leaf->setParam<bool>("buy_delay", false);
    leaf->setTO(stk.getKData(full));
    auto inner = std::make_shared<MultiSystem>("aba_inner_susp");
    inner->setMode("A");
    inner->setSubInitCash(100000.0);
    inner->add(leaf);
    auto middle = std::make_shared<MultiSystem>("aba_middle_susp");
    middle->setMode("B");
    middle->setSE(std::make_shared<SelectAllSE>(SystemList{inner}));
    middle->setTradeOnClose(false);
    middle->add(inner);
    auto outer = std::make_shared<MultiSystem>("aba_outer_susp");
    outer->setTM(crtTM(Datetime(200005010000LL), 300000.0));
    outer->setMode("A");
    outer->setSE(std::make_shared<SelectAllSE>(SystemList{middle}));
    outer->add(middle);
    outer->run(k1);

    double leaf1 = leaf->getTM()->getPosition(k1.back().datetime, stk).number;
    double inner1 = inner->getTM()->getPosition(k1.back().datetime, stk).number;
    REQUIRE_GT(leaf1, 0.0);
    REQUIRE_GT(inner1, 0.0);

    /** @arg the suspended close reduction of the middle B reaches the inner A account and the
     * outer A shadow once (the leaf shadow is the signal book, unaffected by the reduction) */
    middle->setAF(AF_FixedAmount(30000.0));
    outer->run(k2, false);
    double inner2 = inner->getTM()->getPosition(k2.back().datetime, stk).number;
    double parent2 = outer->getTM()->getPosition(k2.back().datetime, stk).number;
    CHECK_LT(inner2, inner1);
    CHECK_GT(parent2, 0.0);
}

/** @par Check point: a trade_on_close=true composite reduction on a suspended close queues the
 * pro-rata legs (the suspended leaf is valued at its last close) and fills them at the next
 * quoted open */
TEST_CASE("test_MultiSystem_composite_reduction_suspended_close") {
    Stock stk_susp = getStock("sh600005");
    Stock stk_quote = getStock("sh600000");
    Stock market = stk_quote;
    REQUIRE(!stk_susp.isNull());
    REQUIRE(!stk_quote.isNull());
    KQuery full(Datetime(20000401), Datetime(20000731));
    KQuery q1(Datetime(20000501), Datetime(20000630));
    KQuery q2(Datetime(20000630), Datetime(20000707));
    KData k1 = market.getKData(q1);
    KData k2 = market.getKData(q2);
    REQUIRE(k1.size() > 10);
    REQUIRE(k2.size() > 2);
    REQUIRE_UNARY(stk_susp.getKData(KQuery(Datetime(20000630), Datetime(20000701))).empty());

    auto leaf_susp =
      SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf_susp->setStock(stk_susp);
    leaf_susp->setParam<bool>("buy_delay", false);
    leaf_susp->setTO(stk_susp.getKData(full));
    auto leaf_quote =
      SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Cycle());
    leaf_quote->setStock(stk_quote);
    leaf_quote->setParam<bool>("buy_delay", false);
    leaf_quote->setTO(stk_quote.getKData(full));
    auto inner = std::make_shared<MultiSystem>("comp_red_inner");
    inner->setMode("A");
    inner->setSubInitCash(150000.0);
    inner->add(leaf_susp);
    inner->add(leaf_quote);

    auto ms = std::make_shared<MultiSystem>("comp_red_outer");
    ms->setTM(crtTM(Datetime(200005010000LL), 300000.0));
    ms->setMode("B");
    ms->setSE(std::make_shared<SelectAllSE>(SystemList{inner}));
    ms->add(inner);
    ms->run(k1);

    double susp1 = inner->getTM()->getPosition(k1.back().datetime, stk_susp).number;
    double quote1 = inner->getTM()->getPosition(k1.back().datetime, stk_quote).number;
    REQUIRE_GT(susp1, 0.0);
    REQUIRE_GT(quote1, 0.0);

    /** @arg the suspended leaf is valued at its last close: both leaves are reduced pro rata and
     * the quoted leaf is not liquidated just because its sibling is suspended */
    ms->setAF(AF_FixedAmount(70000.0));
    ms->run(k2, false);
    double susp2 = inner->getTM()->getPosition(k2.back().datetime, stk_susp).number;
    double quote2 = inner->getTM()->getPosition(k2.back().datetime, stk_quote).number;
    CHECK_LT(susp2, susp1);
    CHECK_GT(quote2, 0.0);

    /** @arg the quoted leaf is not fully liquidated at the suspended close: the pro-rata ratio
     * covers both leaves, so the close-time parent sell stays partial */
    double sold_quote_at_close = 0.0;
    for (const auto& tr : ms->getTM()->getTradeList()) {
        if (tr.datetime == Datetime(200006300000LL) && tr.business == BUSINESS_SELL &&
            tr.stock == stk_quote) {
            sold_quote_at_close += tr.number;
        }
    }
    CHECK_GT(sold_quote_at_close, 0.0);
    // The lot rounding alone cannot explain the gap: without the suspended leaf in the value base
    // the ratio clamps to 1.0 and the quoted leaf is sold in full
    CHECK_LT(sold_quote_at_close, quote1 - 1000.0);
}

/** @} */ /** @} */
