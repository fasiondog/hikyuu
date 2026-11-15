/*
 * test_AllocateFunds.cpp
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Unit tests for the portfolio-level fund allocation (AF): the base parameters and their
 *  domains, the fixed-weight-list parameter validation, the L1/L2/L3 hooks driven directly with
 *  hand-built contexts / suggestions (the gross aggregation, the weight lookup, the reversal legs
 *  and the shrink-only L3 clipping), the dynamic position total control and the EV holding.
 */

#include "../../test_config.h"
#include <cmath>
#include <limits>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_sys/allocatefunds/crt/AF_EqualWeight.h>
#include <hikyuu/trade_sys/allocatefunds/crt/AF_FixedAmount.h>
#include <hikyuu/trade_sys/allocatefunds/crt/AF_FixedWeightList.h>
#include <hikyuu/trade_sys/environment/EnvironmentBase.h>
#include "../system/create_test_sys.h"

using namespace hku;

/**
 * @defgroup test_AllocateFunds test_AllocateFunds
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** A minimal EV for the tests: 20000101 -> 1.0 (full), 20000102 -> 0.5 (half), any other day is
 *  never registered (invalid) */
class TestEv : public EnvironmentBase {
public:
    TestEv() : EnvironmentBase("TestEv") {}
    void _calculate() override {
        _addValid(Datetime(200001010000LL), 1.0);
        _addValid(Datetime(200001020000LL), 0.5);
    }
    EnvironmentPtr _clone() override {
        return std::make_shared<TestEv>();
    }
};

/** A custom AF standing for an EV-driven subclass: the dynamic total control returns the EV value
 *  of the day (the intended usage of the EV held by the base) */
class EVScaleAf : public AllocateFundsBase {
public:
    EVScaleAf() : AllocateFundsBase("EVScaleAf") {}
    price_t _getTotalPositionScale(const Datetime& date) const override {
        HKU_IF_RETURN(!getEV(), 1.0);
        HKU_IF_RETURN(!getEV()->isValid(date), 0.0);
        price_t value = getEV()->getValue(date);
        return value > 0.0 ? value : 0.0;
    }
    ALLOCATE_FUNDS_IMP(EVScaleAf)
};

/** A custom AF with a fixed total position scale (any value, to test the base guarantee and the
 *  cap composition) */
class FixedScaleAf : public AllocateFundsBase {
public:
    FixedScaleAf() : AllocateFundsBase("FixedScaleAf"), m_scale(1.0) {}
    explicit FixedScaleAf(price_t scale) : AllocateFundsBase("FixedScaleAf"), m_scale(scale) {}
    price_t _getTotalPositionScale(const Datetime&) const override {
        return m_scale;
    }
    price_t m_scale;
    ALLOCATE_FUNDS_IMP(FixedScaleAf)
};

/** @par Test points */
TEST_CASE("test_AllocateFunds_base_params") {
    AFPtr af = AF_EqualWeight();

    /** @arg the base parameter family and their defaults */
    CHECK_EQ(af->getParam<double>("max_single_position"), doctest::Approx(1.0));
    CHECK_EQ(af->getParam<double>("max_total_position"), doctest::Approx(1.0));
    CHECK_EQ(af->getParam<double>("fixed_amount"), doctest::Approx(0.0));
    CHECK_EQ(af->getParam<bool>("adjust_running_sys"), true);

    /** @arg the legacy base-class weight-list parameter has been removed (the fixed weights go
     * through AF_FixedWeightList) */
    CHECK_UNARY(!af->haveParam("weight-list"));

    /** @arg the deprecated hyphenated parameter names are rejected instead of registering
     * dead keys */
    CHECK_THROWS_AS(af->setParam<double>("max-single-position", 0.5), std::exception);
    CHECK_THROWS_AS(af->setParam<double>("max-total-position", 0.5), std::exception);
    CHECK_THROWS_AS(af->setParam<double>("fixed-amount", 0.5), std::exception);

    /** @arg non-default values are accepted and read back unchanged */
    af->setParam<double>("max_single_position", 0.5);
    af->setParam<double>("max_total_position", 0.8);
    af->setParam<double>("fixed_amount", 30000.0);
    af->setParam<bool>("adjust_running_sys", false);
    CHECK_EQ(af->getParam<double>("max_single_position"), doctest::Approx(0.5));
    CHECK_EQ(af->getParam<double>("max_total_position"), doctest::Approx(0.8));
    CHECK_EQ(af->getParam<double>("fixed_amount"), doctest::Approx(30000.0));
    CHECK_EQ(af->getParam<bool>("adjust_running_sys"), false);

    /** @arg max_single_position = 0 disables the limit, a negative value is rejected */
    CHECK_NOTHROW(af->setParam<double>("max_single_position", 0.0));
    CHECK_EQ(af->getParam<double>("max_single_position"), doctest::Approx(0.0));
    CHECK_THROWS_AS(af->setParam<double>("max_single_position", -1.0), std::exception);
}

/** @par Test points */
TEST_CASE("test_AllocateFunds_checkParam") {
    AFPtr af = AF_EqualWeight();

    /** @arg a non-finite max_single_position is rejected (it would silently disable the
     *  concentration cap) */
    CHECK_THROWS_AS(af->setParam<double>("max_single_position", std::nan("")), std::exception);
    CHECK_THROWS_AS(
      af->setParam<double>("max_single_position", std::numeric_limits<double>::infinity()),
      std::exception);

    /** @arg a non-finite max_total_position is rejected (it would silently disable the total
     *  budget cap) */
    CHECK_THROWS_AS(af->setParam<double>("max_total_position", std::nan("")), std::exception);

    /** @arg the parameter domains: max_single_position in [0, 1], max_total_position >= 0 with
     *  >1 as the documented leverage */
    CHECK_NOTHROW(af->setParam<double>("max_single_position", 0.0));
    CHECK_NOTHROW(af->setParam<double>("max_single_position", 1.0));
    CHECK_THROWS_AS(af->setParam<double>("max_single_position", -1.0), std::exception);
    CHECK_THROWS_AS(af->setParam<double>("max_single_position", 2.0), std::exception);
    CHECK_NOTHROW(af->setParam<double>("max_total_position", 0.0));
    CHECK_NOTHROW(af->setParam<double>("max_total_position", 2.0));
    CHECK_THROWS_AS(af->setParam<double>("max_total_position", -1.0), std::exception);

    /** @arg fixed_amount: 0 (disabled) and a positive amount are valid */
    CHECK_NOTHROW(af->setParam<double>("fixed_amount", 0.0));
    CHECK_NOTHROW(af->setParam<double>("fixed_amount", 30000.0));

    /** @arg fixed_amount: a negative value is rejected (it would silently fall back to weights) */
    CHECK_THROWS_AS(af->setParam<double>("fixed_amount", -1.0), std::exception);

    /** @arg fixed_amount: NaN / inf are rejected */
    CHECK_THROWS_AS(af->setParam<double>("fixed_amount", std::nan("")), std::exception);
    CHECK_THROWS_AS(af->setParam<double>("fixed_amount", std::numeric_limits<double>::infinity()),
                    std::exception);

    /** @arg the AF_FixedAmount factory goes through the same validation */
    CHECK_NOTHROW(AF_FixedAmount(20000.0));
    CHECK_THROWS_AS(AF_FixedAmount(-1.0), std::exception);
}

/** @par Test points */
TEST_CASE("test_AF_FixedWeightList_checkParam") {
    /** @arg a valid non-empty weight list is accepted */
    CHECK_NOTHROW(AF_FixedWeightList(vector<double>{1.0, 2.0, 3.0}));

    /** @arg an all-zero weight list is rejected (would actively clear the holdings otherwise) */
    CHECK_THROWS_AS(AF_FixedWeightList(vector<double>{0.0, 0.0, 0.0}), std::exception);

    /** @arg a negative weight is rejected */
    CHECK_THROWS_AS(AF_FixedWeightList(vector<double>{-1.0, 2.0}), std::exception);

    /** @arg a non-finite (inf) weight is rejected */
    CHECK_THROWS_AS(
      AF_FixedWeightList(vector<double>{std::numeric_limits<double>::infinity(), 1.0}),
      std::exception);

    /** @arg an overflowing sum (each item finite but the sum is inf) is rejected */
    CHECK_THROWS_AS(AF_FixedWeightList(vector<double>{1e308, 1e308}), std::exception);

    /** @arg an empty weight list is rejected instead of degrading to the equal weight */
    CHECK_THROWS_AS(AF_FixedWeightList(PriceList{}), std::exception);

    /** @arg the same holds through set_param, so a configured instance cannot be emptied either */
    AFPtr af = AF_FixedWeightList(vector<double>{1.0, 2.0});
    CHECK_THROWS_AS(af->setParam<PriceList>("weights", PriceList()), std::exception);
}

/** @par Test points */
TEST_CASE("test_AllocateFunds_L2_gross_aggregated") {
    auto af = AF_EqualWeight();
    af->setParam<double>("max_total_position", 0.5);  // budget 50000
    Datetime date(200001010000LL);
    Stock a = getStock("sz000001");
    Stock b = getStock("sh600000");
    REQUIRE(!a.isNull());
    REQUIRE(!b.isNull());
    auto sys1 = create_test_sys(3, 5);
    auto sys2 = create_test_sys(5, 10);
    std::unordered_map<SYSPtr, double> weights{{sys1, 1.0}, {sys2, 1.0}};

    TradeSuggestion s;
    s.plan_price = 10.0;
    TradeSuggestion a_long = s;
    a_long.stock = a;
    a_long.sys = sys1;
    a_long.target_position_ratio = 0.5;
    TradeSuggestion a_short = a_long;
    a_short.sys = sys2;
    a_short.target_position_ratio = -0.5;
    TradeSuggestion b_long = s;
    b_long.stock = b;
    b_long.sys = sys1;
    b_long.target_position_ratio = 0.5;

    /** @arg a same-instrument hedge does not inflate the gross: the whole 50000 budget stays
     *  available to the other instrument (5000 shares, against 5000/3 when counted per suggestion)
     */
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);  // no position: net assets = 100000
    tm->setParam<bool>("support_borrow_stock", true);
    TradeSuggestionList suggestions{a_long, a_short, b_long};
    af->_toTargets(date, tm, suggestions, weights, KQuery(), "A");
    REQUIRE_EQ(suggestions.size(), 1);
    CHECK_UNARY(suggestions[0].stock == b);
    CHECK_EQ(suggestions[0].number, doctest::Approx(5000.0));

    /** @arg the clamped path measures the gross on the clamped aggregate too: the a leg nets
     *  -25000 and is clamped to 0, leaving the full budget to the other instrument */
    auto tm_plain = crtTM(Datetime(200001010000LL), 100000.0);  // no securities lending
    a_short.target_position_ratio = -0.75;
    TradeSuggestionList clamped{a_long, a_short, b_long};
    af->_toTargets(date, tm_plain, clamped, weights, KQuery(), "A");
    REQUIRE_EQ(clamped.size(), 1);
    CHECK_UNARY(clamped[0].stock == b);
    CHECK_EQ(clamped[0].number, doctest::Approx(5000.0));
}

/** @par Test points */
TEST_CASE("test_AllocateFunds_L2_no_weight_skipped") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto af = AF_EqualWeight();
    Datetime date(200001010000LL);
    Stock stk = getStock("sz000001");
    REQUIRE(!stk.isNull());
    auto sys1 = create_test_sys(3, 5);
    auto sys2 = create_test_sys(5, 10);

    TradeSuggestion s;
    s.stock = stk;
    s.sys = sys1;
    s.plan_price = 10.0;
    s.target_position_ratio = 0.5;

    /** @arg an empty weight table (L1 produced nothing) drops the sub-system suggestion instead
     *  of pricing it by a fabricated 1.0 weight */
    TradeSuggestionList empty_table{s};
    af->_toTargets(date, tm, empty_table, AllocateFundsBase::Weights(), KQuery(), "A");
    CHECK_UNARY(empty_table.empty());

    /** @arg a cloned sub-system is another instance: it has no weight and is dropped */
    std::unordered_map<SYSPtr, double> weights{{sys2, 1.0}};
    TradeSuggestion cloned = s;
    cloned.sys = sys1->clone();
    REQUIRE(cloned.sys != nullptr);
    TradeSuggestionList clone_list{cloned};
    af->_toTargets(date, tm, clone_list, weights, KQuery(), "A");
    CHECK_UNARY(clone_list.empty());

    /** @arg a parent-synthesized suggestion (no sub-system) keeps the neutral weight: 0.5 x
     *  100000 -> 5000 shares */
    TradeSuggestion parent_s = s;
    parent_s.sys = nullptr;
    TradeSuggestionList parent_list{parent_s};
    af->_toTargets(date, tm, parent_list, AllocateFundsBase::Weights(), KQuery(), "A");
    REQUIRE_EQ(parent_list.size(), 1);
    CHECK_EQ(parent_list[0].number, doctest::Approx(5000.0));
}

/** @par Test points */
TEST_CASE("test_AllocateFunds_L2_reversal_legs") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    tm->setParam<bool>("support_borrow_stock", true);
    auto af = AF_EqualWeight();
    af->setParam<double>("fixed_amount", 2500.0);  // pin the target value: -2500 -> -250 shares
    Datetime date(200001010000LL);
    Stock stk = getStock("sz000001");
    REQUIRE(!stk.isNull());
    auto sys = create_test_sys(3, 5);
    std::unordered_map<SYSPtr, double> weights{{sys, 1.0}};
    REQUIRE(!tm->buy(date, stk, 10.0, 500.0).isNull());

    TradeSuggestion s;
    s.stock = stk;
    s.sys = sys;
    s.plan_price = 10.0;
    s.target_position_ratio = -0.9;

    /** @arg a long -> short reversal is split into two legs: sell the long, then open the short */
    TradeSuggestionList suggestions{s};
    af->_toTargets(date, tm, suggestions, weights, KQuery(), "A");
    REQUIRE_EQ(suggestions.size(), 2);
    CHECK(suggestions[0].type == SuggestionType::SELL);
    CHECK_UNARY(!suggestions[0].short_side);
    CHECK_EQ(suggestions[0].number, doctest::Approx(500.0));
    CHECK(suggestions[1].type == SuggestionType::SELL);
    CHECK_UNARY(suggestions[1].short_side);
    CHECK_EQ(suggestions[1].number, doctest::Approx(250.0));

    /** @arg an opening leg below one lot is dropped: -50 shares still sells the long 500 but
     *  emits no sub-lot short leg */
    af->setParam<double>("fixed_amount", 500.0);
    TradeSuggestionList sub_lot{s};
    af->_toTargets(date, tm, sub_lot, weights, KQuery(), "A");
    REQUIRE_EQ(sub_lot.size(), 1);
    CHECK(sub_lot[0].type == SuggestionType::SELL);
    CHECK_UNARY(!sub_lot[0].short_side);
    CHECK_EQ(sub_lot[0].number, doctest::Approx(500.0));
}

/** @par Test points */
TEST_CASE("test_AllocateFunds_L2_plan_price_first") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);  // no position: net assets = 100000
    auto af = AF_EqualWeight();
    Datetime date(200001010000LL);
    Stock stk = getStock("sz000001");
    REQUIRE(!stk.isNull());
    auto sys1 = create_test_sys(3, 5);
    auto sys2 = create_test_sys(5, 10);
    std::unordered_map<SYSPtr, double> weights{{sys1, 1.0}, {sys2, 1.0}};

    TradeSuggestion s;
    s.stock = stk;
    s.sys = sys1;
    s.plan_price = 10.0;
    s.target_position_ratio = 0.25;  // 25000 each, 50000 in total
    TradeSuggestion s2 = s;
    s2.sys = sys2;
    s2.plan_price = 12.0;

    /** @arg the first suggestion fixes the price: 50000 / 10 = 5000 shares */
    TradeSuggestionList suggestions{s, s2};
    af->_toTargets(date, tm, suggestions, weights, KQuery(), "A");
    REQUIRE_EQ(suggestions.size(), 1);
    CHECK_EQ(suggestions[0].plan_price, doctest::Approx(10.0));
    CHECK_EQ(suggestions[0].number, doctest::Approx(5000.0));

    /** @arg the position decides, not the value: reversed, 50000 / 12 = 4166.67 shares */
    TradeSuggestionList reversed{s2, s};
    af->_toTargets(date, tm, reversed, weights, KQuery(), "A");
    REQUIRE_EQ(reversed.size(), 1);
    CHECK_EQ(reversed[0].plan_price, doctest::Approx(12.0));
    CHECK_EQ(reversed[0].number, doctest::Approx(50000.0 / 12.0));
}

/** @par Test points */
TEST_CASE("test_AllocateFunds_fixed_amount_truncation") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);  // no position: net assets = 100000
    auto af = AF_EqualWeight();
    af->setParam<double>("fixed_amount", 50000.0);
    Datetime date(200001010000LL);
    auto sys1 = create_test_sys(3, 5);
    auto sys2 = create_test_sys(5, 10);
    auto sys3 = create_test_sys(7, 15);

    /** @arg mode B: the 100000 budget funds two whole 50000 quotas, the third gets zero */
    SubSystemContextList contexts(3);
    contexts[0].sys = sys1;
    contexts[1].sys = sys2;
    contexts[2].sys = sys3;
    af->_allocate(date, tm, contexts, KQuery(), "B");
    CHECK_EQ(contexts[0].quota, doctest::Approx(50000.0));
    CHECK_EQ(contexts[1].quota, doctest::Approx(50000.0));
    CHECK_EQ(contexts[2].quota, doctest::Approx(0.0));

    /** @arg mode A: the same truncation applies per instrument in first-appearance order,
     *  the unfunded instrument is driven flat (no order while the position is empty) */
    Stock a = getStock("sz000001");
    Stock b = getStock("sh600000");
    REQUIRE(!a.isNull());
    REQUIRE(!b.isNull());
    std::unordered_map<SYSPtr, double> weights{{sys1, 1.0}, {sys2, 1.0}};
    af->setParam<double>("fixed_amount", 60000.0);
    TradeSuggestion s;
    s.sys = sys1;
    s.plan_price = 10.0;
    s.target_position_ratio = 0.5;
    TradeSuggestion sa = s;
    sa.stock = a;
    TradeSuggestion sb = s;
    sb.stock = b;
    TradeSuggestionList suggestions{sa, sb};
    af->_toTargets(date, tm, suggestions, weights, KQuery(), "A");
    REQUIRE_EQ(suggestions.size(), 1);
    CHECK_UNARY(suggestions[0].stock == a);
    CHECK_EQ(suggestions[0].number, doctest::Approx(6000.0));  // 60000 / 10
}

/** @par Test points */
TEST_CASE("test_AllocateFunds_fixed_amount_short_clamp_order") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);  // no securities lending by default
    auto af = AF_EqualWeight();
    af->setParam<double>("fixed_amount", 60000.0);  // one quota fits the 100000 budget
    Datetime date(200001010000LL);
    Stock a = getStock("sz000001");
    Stock b = getStock("sh600000");
    REQUIRE(!a.isNull());
    REQUIRE(!b.isNull());
    auto sys = create_test_sys(3, 5);
    std::unordered_map<SYSPtr, double> weights{{sys, 1.0}};

    TradeSuggestion s;
    s.sys = sys;
    s.plan_price = 10.0;
    TradeSuggestion a_short = s;
    a_short.stock = a;
    a_short.target_position_ratio = -0.5;
    TradeSuggestion b_long = s;
    b_long.stock = b;
    b_long.target_position_ratio = 0.5;

    /** @arg the short leg is unexecutable and is zeroed before the budget is charged: the long
     *  instrument still gets the whole quota (60000 / 10 = 6000 shares) */
    TradeSuggestionList suggestions{a_short, b_long};
    af->_toTargets(date, tm, suggestions, weights, KQuery(), "A");
    REQUIRE_EQ(suggestions.size(), 1);
    CHECK_UNARY(suggestions[0].stock == b);
    CHECK_EQ(suggestions[0].number, doctest::Approx(6000.0));

    /** @arg with a securities-lending account both legs consume the budget in order: only the
     *  first one is funded */
    auto tm_borrow = crtTM(Datetime(200001010000LL), 100000.0);
    tm_borrow->setParam<bool>("support_borrow_stock", true);
    TradeSuggestionList both{a_short, b_long};
    af->_toTargets(date, tm_borrow, both, weights, KQuery(), "A");
    REQUIRE_EQ(both.size(), 1);
    CHECK_UNARY(both[0].stock == a);
    CHECK_EQ(both[0].number, doctest::Approx(6000.0));
    CHECK_UNARY(both[0].short_side);
}

/** @par Test points */
TEST_CASE("test_AllocateFunds_fixed_amount_lot_filter_order") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);  // budget 100000 funds one 60000 quota
    auto af = AF_EqualWeight();
    af->setParam<double>("fixed_amount", 60000.0);
    Datetime date(200001010000LL);
    Stock a = getStock("sz000001");
    Stock b = getStock("sh600000");
    REQUIRE(!a.isNull());
    REQUIRE(!b.isNull());
    auto sys = create_test_sys(3, 5);
    std::unordered_map<SYSPtr, double> weights{{sys, 1.0}};

    TradeSuggestion s;
    s.sys = sys;
    TradeSuggestion a_lot = s;
    a_lot.stock = a;
    a_lot.plan_price = 1000.0;  // one board lot costs 100000 > the 60000 quota
    a_lot.target_position_ratio = 0.5;
    TradeSuggestion b_lot = s;
    b_lot.stock = b;
    b_lot.plan_price = 10.0;
    b_lot.target_position_ratio = 0.5;

    /** @arg a target below one board lot is not charged against the budget: the other instrument
     *  still gets the whole quota (60000 / 10 = 6000 shares) */
    TradeSuggestionList suggestions{a_lot, b_lot};
    af->_toTargets(date, tm, suggestions, weights, KQuery(), "A");
    REQUIRE_EQ(suggestions.size(), 1);
    CHECK_UNARY(suggestions[0].stock == b);
    CHECK_EQ(suggestions[0].number, doctest::Approx(6000.0));
}

/** @par Test points */
TEST_CASE("test_AllocateFunds_hedged_book_flat_target_clears_both_sides") {
    Datetime date(200001010000LL);
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    auto tm = crtTM(date, 1000000.0);
    tm->setParam<bool>("support_borrow_stock", true);
    // Seed a hedged book: the parent holds one long and one short of the same instrument
    REQUIRE(!tm->buy(date, stk, 10.0, 100, 0.0, 0.0, 10.0, PART_SYSTEM, "seed").isNull());
    REQUIRE(!tm->sellShort(date, stk, 10.0, 100, 0.0, 0.0, 10.0, PART_SYSTEM, "seed").isNull());
    REQUIRE_EQ(tm->getPosition(date, stk).number, doctest::Approx(100.0));
    REQUIRE_EQ(tm->getShortPosition(stk).number, doctest::Approx(100.0));

    auto af = AF_EqualWeight();
    TradeSuggestion s;
    s.stock = stk;
    s.plan_price = 10.0;
    s.target_position_ratio = 0.0;  // the orphan zero target synthesized by the parent
    TradeSuggestionList suggestions{s};
    af->_toTargets(date, tm, suggestions, {}, KQuery(), "A");

    /** @arg a flat target closes BOTH sides of a hedged book: the net delta (0 - 0) cannot see it,
     * so the long sell and the short cover must be emitted explicitly */
    REQUIRE_EQ(suggestions.size(), 2);
    CHECK_UNARY(suggestions[0].type == SuggestionType::SELL);
    CHECK_UNARY(!suggestions[0].short_side);
    CHECK_EQ(suggestions[0].number, doctest::Approx(100.0));
    CHECK_UNARY(suggestions[1].type == SuggestionType::BUY);
    CHECK_UNARY(suggestions[1].short_side);
    CHECK_EQ(suggestions[1].number, doctest::Approx(100.0));
}

/** @par Test points */
TEST_CASE("test_AllocateFunds_L3_shrink_only") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto af = AF_EqualWeight();
    af->setParam<double>("max_single_position", 0.25);  // cap 25000 per instrument
    Datetime date(200001010000LL);
    Stock a = getStock("sz000001");
    Stock b = getStock("sh600000");
    REQUIRE(!a.isNull());
    REQUIRE(!b.isNull());
    auto sys = create_test_sys(3, 5);
    std::unordered_map<SYSPtr, double> weights{{sys, 1.0}};

    TradeSuggestion s;
    s.sys = sys;
    s.plan_price = 10.0;
    s.target_position_ratio = 0.5;  // 50000 each, no total budget scaling
    TradeSuggestion sa = s;
    sa.stock = a;
    TradeSuggestion sb = s;
    sb.stock = b;
    TradeSuggestionList suggestions{sa, sb};
    af->allocateTargets(date, tm, suggestions, weights, KQuery(), "A");

    /** @arg both instruments are clipped to the 25000 cap */
    REQUIRE_EQ(suggestions.size(), 2);
    for (const auto& o : suggestions) {
        CHECK_EQ(o.number, doctest::Approx(2500.0));
        CHECK_EQ(o.plan_cash, doctest::Approx(25000.0));
    }

    /** @arg the released budget is not redistributed: the gross stays at 0.5 x net assets */
    double gross = 0.0;
    for (const auto& o : suggestions) {
        gross += o.plan_cash;
    }
    CHECK_EQ(gross, doctest::Approx(50000.0));
}

/** @par Test points */
TEST_CASE("test_AllocateFunds_ev_property") {
    auto af = AF_EqualWeight();

    /** @arg no EV is held by default */
    CHECK_UNARY(!af->getEV());

    /** @arg setEV/getEV keeps the same shared instance (no clone on set) */
    auto ev = std::make_shared<TestEv>();
    ev->setQuery(KQuery(0, 10));  // trigger the calculation so the values exist
    af->setEV(ev);
    CHECK_UNARY(af->getEV() == ev);
    CHECK_EQ(af->getEV()->getValue(Datetime(200001020000LL)), doctest::Approx(0.5));

    /** @arg shared_ev=true (the default): the clone shares the EV instance */
    auto af2 = af->clone();
    CHECK_UNARY(af2->getEV() == af->getEV());

    /** @arg shared_ev=false: the clone gets its own copy */
    af->setParam<bool>("shared_ev", false);
    auto af3 = af->clone();
    CHECK_UNARY(af3->getEV() && af3->getEV() != af->getEV());

    /** @arg shared_ev=true: the reset leaves the EV untouched (the values survive) */
    af->setParam<bool>("shared_ev", true);
    af->reset();
    CHECK_EQ(af->getEV()->getValue(Datetime(200001010000LL)), doctest::Approx(1.0));

    /** @arg shared_ev=false: the reset resets the EV (the calculated values are dropped) */
    af->setParam<bool>("shared_ev", false);
    af->reset();
    CHECK_EQ(af->getEV()->getValue(Datetime(200001010000LL)), doctest::Approx(0.0));
}

/** @par Test points */
TEST_CASE("test_AllocateFunds_total_position_scale") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);  // net assets 100000
    Datetime date(200001010000LL);
    auto sys1 = create_test_sys(3, 5);
    auto sys2 = create_test_sys(5, 10);

    /** @arg the default hook returns 1.0: the behavior is unchanged */
    auto af = AF_EqualWeight();
    CHECK_EQ(af->getTotalPositionScale(date), doctest::Approx(1.0));
    SubSystemContextList contexts(2);
    contexts[0].sys = sys1;
    contexts[1].sys = sys2;
    af->_allocate(date, tm, contexts, KQuery(), "B");
    CHECK_EQ(contexts[0].quota, doctest::Approx(50000.0));
    CHECK_EQ(contexts[1].quota, doctest::Approx(50000.0));

    /** @arg the scale 0.5 halves the total budget: the quotas are 25000 each */
    auto half = std::make_shared<FixedScaleAf>(0.5);
    SubSystemContextList half_ctxs(2);
    half_ctxs[0].sys = sys1;
    half_ctxs[1].sys = sys2;
    half->_allocate(date, tm, half_ctxs, KQuery(), "B");
    CHECK_EQ(half_ctxs[0].quota, doctest::Approx(25000.0));
    CHECK_EQ(half_ctxs[1].quota, doctest::Approx(25000.0));

    /** @arg the scale 0 drives everything flat: every quota is 0 */
    auto flat = std::make_shared<FixedScaleAf>(0.0);
    SubSystemContextList flat_ctxs(2);
    flat_ctxs[0].sys = sys1;
    flat_ctxs[1].sys = sys2;
    flat->_allocate(date, tm, flat_ctxs, KQuery(), "B");
    CHECK_EQ(flat_ctxs[0].quota, doctest::Approx(0.0));
    CHECK_EQ(flat_ctxs[1].quota, doctest::Approx(0.0));

    /** @arg the base guarantee: a negative scale is floored to 0 */
    auto neg = std::make_shared<FixedScaleAf>(-1.0);
    CHECK_EQ(neg->getTotalPositionScale(date), doctest::Approx(0.0));

    /** @arg the base guarantee: a NaN scale is 0 */
    auto nan = std::make_shared<FixedScaleAf>(std::nan(""));
    CHECK_EQ(nan->getTotalPositionScale(date), doctest::Approx(0.0));

    /** @arg the base guarantee: +inf maps to 1.0 and -inf maps to 0.0 */
    auto pinf = std::make_shared<FixedScaleAf>(std::numeric_limits<price_t>::infinity());
    auto ninf = std::make_shared<FixedScaleAf>(-std::numeric_limits<price_t>::infinity());
    CHECK_EQ(pinf->getTotalPositionScale(date), doctest::Approx(1.0));
    CHECK_EQ(ninf->getTotalPositionScale(date), doctest::Approx(0.0));

    /** @arg the base guarantee: an over-range scale (2.5) is clamped to 1.0, so an override can
     *  never exceed the max_total_position total control */
    auto over = std::make_shared<FixedScaleAf>(2.5);
    CHECK_EQ(over->getTotalPositionScale(date), doctest::Approx(1.0));

    /** @arg mode A, scale 0.5: the gross target is scaled to half (2500 shares each) */
    Stock a = getStock("sz000001");
    Stock b = getStock("sh600000");
    REQUIRE(!a.isNull());
    REQUIRE(!b.isNull());
    auto sys = create_test_sys(3, 5);
    std::unordered_map<SYSPtr, double> weights{{sys, 1.0}};
    TradeSuggestion s;
    s.stock = a;
    s.sys = sys;
    s.plan_price = 10.0;
    s.target_position_ratio = 0.5;  // each instrument 50000, gross 100000
    TradeSuggestion sb = s;
    sb.stock = b;
    TradeSuggestionList half_targets{s, sb};
    half->_toTargets(date, tm, half_targets, weights, KQuery(), "A");
    REQUIRE_EQ(half_targets.size(), 2);
    for (const auto& o : half_targets) {
        CHECK_EQ(o.number, doctest::Approx(2500.0));
    }

    /** @arg mode A, scale 0: the held instrument is driven flat (SELL the long), the empty one
     *  produces no order */
    auto tm_hold = crtTM(Datetime(200001010000LL), 100000.0);
    REQUIRE(!tm_hold->buy(date, a, 10.0, 1000.0).isNull());
    TradeSuggestionList flat_targets{s, sb};
    flat->_toTargets(date, tm_hold, flat_targets, weights, KQuery(), "A");
    REQUIRE_EQ(flat_targets.size(), 1);
    CHECK_UNARY(flat_targets[0].stock == a);
    CHECK(flat_targets[0].type == SuggestionType::SELL);
    CHECK_EQ(flat_targets[0].number, doctest::Approx(1000.0));

    /** @arg mode A: the over-range scale (2.5) is clamped to 1.0, so the ratio 1.5 target of
     *  150000 stays bounded by the static cap 100000 (10000 shares, never leveraged) */
    TradeSuggestion over_target{s};
    over_target.target_position_ratio = 1.5;
    TradeSuggestionList clamped_targets{over_target};
    over->_toTargets(date, tm, clamped_targets, weights, KQuery(), "A");
    REQUIRE_EQ(clamped_targets.size(), 1);
    CHECK_EQ(clamped_targets[0].number, doctest::Approx(10000.0));

    /** @arg the same target without any override reaches the same bound */
    TradeSuggestionList capped_targets{over_target};
    af->_toTargets(date, tm, capped_targets, weights, KQuery(), "A");
    REQUIRE_EQ(capped_targets.size(), 1);
    CHECK_EQ(capped_targets[0].number, doctest::Approx(10000.0));
}

/** @par Test points */
TEST_CASE("test_AllocateFunds_ev_driven_scale") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);  // net assets 100000
    auto af = std::make_shared<EVScaleAf>();
    auto ev = std::make_shared<TestEv>();
    ev->setQuery(KQuery(0, 10));  // trigger the calculation so the values exist
    af->setEV(ev);
    auto sys1 = create_test_sys(3, 5);
    auto sys2 = create_test_sys(5, 10);

    /** @arg the EV value 1.0 keeps the equal weight 1/2 (each 50000) */
    SubSystemContextList contexts(2);
    contexts[0].sys = sys1;
    contexts[1].sys = sys2;
    af->_allocate(Datetime(200001010000LL), tm, contexts, KQuery(), "B");
    CHECK_EQ(contexts[0].quota, doctest::Approx(50000.0));
    CHECK_EQ(contexts[1].quota, doctest::Approx(50000.0));

    /** @arg the EV value 0.5 halves the total budget: the quotas are 25000 each */
    SubSystemContextList half(2);
    half[0].sys = sys1;
    half[1].sys = sys2;
    af->_allocate(Datetime(200001020000LL), tm, half, KQuery(), "B");
    CHECK_EQ(half[0].quota, doctest::Approx(25000.0));
    CHECK_EQ(half[1].quota, doctest::Approx(25000.0));

    /** @arg an invalid (never registered) day drives everything flat: every quota is 0 */
    SubSystemContextList invalid(2);
    invalid[0].sys = sys1;
    invalid[1].sys = sys2;
    af->_allocate(Datetime(200001030000LL), tm, invalid, KQuery(), "B");
    CHECK_EQ(invalid[0].quota, doctest::Approx(0.0));
    CHECK_EQ(invalid[1].quota, doctest::Approx(0.0));

    /** @arg without an EV the subclass falls back to 1.0 (the behavior is unchanged) */
    af->setEV(EnvironmentPtr());
    SubSystemContextList plain(2);
    plain[0].sys = sys1;
    plain[1].sys = sys2;
    af->_allocate(Datetime(200001020000LL), tm, plain, KQuery(), "B");
    CHECK_EQ(plain[0].quota, doctest::Approx(50000.0));
    CHECK_EQ(plain[1].quota, doctest::Approx(50000.0));
}

/** @} */
