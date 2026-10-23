/*
 * test_AllocateFunds.cpp
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Unit tests for the portfolio-level fund allocation (AF): the base parameters, the
 *  fixed-weight-list parameter validation, the L1/L2/L3 hooks driven directly with hand-built
 *  contexts / suggestions (the gross aggregation, the weight lookup, the reversal legs and the
 *  shrink-only L3 clipping).
 */

#include "../../test_config.h"
#include <cmath>
#include <limits>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_sys/allocatefunds/crt/AF_EqualWeight.h>
#include <hikyuu/trade_sys/allocatefunds/crt/AF_FixedAmount.h>
#include <hikyuu/trade_sys/allocatefunds/crt/AF_FixedWeightList.h>
#include "../system/create_test_sys.h"

using namespace hku;

/**
 * @defgroup test_AllocateFunds test_AllocateFunds
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

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

    /** @arg max_single_position is not bounded from below (<=0 means no limit, no clamp on set) */
    CHECK_NOTHROW(af->setParam<double>("max_single_position", 0.0));
    CHECK_EQ(af->getParam<double>("max_single_position"), doctest::Approx(0.0));
    CHECK_NOTHROW(af->setParam<double>("max_single_position", -1.0));
    CHECK_EQ(af->getParam<double>("max_single_position"), doctest::Approx(-1.0));
}

/** @par Test points */
TEST_CASE("test_AllocateFunds_checkParam") {
    AFPtr af = AF_EqualWeight();

    /** @arg a NaN max_single_position is rejected (it would silently disable the concentration cap)
     */
    CHECK_THROWS_AS(af->setParam<double>("max_single_position", std::nan("")), std::exception);

    /** @arg a NaN max_total_position is rejected (it would silently disable the total budget cap)
     */
    CHECK_THROWS_AS(af->setParam<double>("max_total_position", std::nan("")), std::exception);

    /** @arg non-positive / >1 values stay valid: <=0 means no limit and >1 is clamped in use */
    CHECK_NOTHROW(af->setParam<double>("max_single_position", 0.0));
    CHECK_NOTHROW(af->setParam<double>("max_single_position", -1.0));
    CHECK_NOTHROW(af->setParam<double>("max_single_position", 2.0));
    CHECK_NOTHROW(af->setParam<double>("max_total_position", 0.0));
    CHECK_NOTHROW(af->setParam<double>("max_total_position", 2.0));

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

/** @} */
