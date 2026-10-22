/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-10-23
 *      Author: fasiondog
 */

#include "../../test_config.h"
#include <cmath>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/multifactor/buildin_scfilter.h>

using namespace hku;

/**
 * @defgroup test_ScoresFilterBase test_ScoresFilterBase
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

namespace {

// Positional cross-section: TopN / IgnoreNan are order- and value-based, no market data is
// touched. The NaN record sits in the middle, so IgnoreNan changes both the length and the tail
// of the list, which makes any hidden extra child filter observable through the output.
ScoreRecordList buildCrossSection() {
    StockManager& sm = StockManager::instance();
    std::vector<string> codes{"sz000001", "sz000002", "sh600000",
                              "sh600004", "sh600005", "sz000004"};
    std::vector<ScoreRecord::value_t> values{5.0, 4.0, Null<price_t>(), 2.0, 1.0};

    ScoreRecordList scores;
    for (const auto& code : codes) {
        if (scores.size() == values.size()) {
            break;
        }
        Stock stk = sm[code];
        if (stk.isNull()) {
            continue;
        }
        scores.emplace_back(stk, values[scores.size()]);
    }
    return scores;
}

}  // namespace

/** @par Test points */
TEST_CASE("test_ScoresFilterBase_operator_or_does_not_modify_operands") {
    KQuery query = KQueryByDate(Datetime(20110104), Datetime(20111206), KQuery::DAY);
    Datetime date(201106010000);
    ScoreRecordList cs = buildCrossSection();
    REQUIRE_EQ(cs.size(), 5);

    auto f1 = SCFilter_IgnoreNan();
    auto f2 = SCFilter_TopN(2);
    auto f3 = SCFilter_TopN(4);

    // Reuse the same f1 in two different combinations
    auto a = f1 | f2;
    auto b = f1 | f3;

    /** @arg f1 keeps its own behavior after being combined twice */
    ScoreRecordList ret1 = f1->filter(cs, date, query);
    REQUIRE_EQ(ret1.size(), 4);
    for (const auto& sc : ret1) {
        CHECK_FALSE(std::isnan(sc.value));
    }

    /** @arg the first combination is IgnoreNan -> TopN(2) */
    ScoreRecordList ret_a = a->filter(cs, date, query);
    REQUIRE_EQ(ret_a.size(), 2);
    CHECK_EQ(ret_a[0].value, doctest::Approx(5.0));
    CHECK_EQ(ret_a[1].value, doctest::Approx(4.0));

    /** @arg the second combination is not polluted by the first one: IgnoreNan -> TopN(4) */
    ScoreRecordList ret_b = b->filter(cs, date, query);
    REQUIRE_EQ(ret_b.size(), 4);
    CHECK_EQ(ret_b[0].value, doctest::Approx(5.0));
    CHECK_EQ(ret_b[3].value, doctest::Approx(1.0));

    /** @arg the combination returns a new instance, not the left operand itself */
    CHECK_FALSE(a == f1);
    CHECK_FALSE(b == f1);

    /** @arg the right operands are not modified either */
    CHECK_EQ(f2->filter(cs, date, query).size(), 2);
    CHECK_EQ(f3->filter(cs, date, query).size(), 4);

    /** @arg changing a right operand afterwards does not affect the combination */
    f2->setParam<int>("topn", 5);
    CHECK_EQ(f2->filter(cs, date, query).size(), 5);
    CHECK_EQ(a->filter(cs, date, query).size(), 2);
}

/** @par Test points */
TEST_CASE("test_ScoresFilterBase_operator_or_chain_reuse") {
    KQuery query = KQueryByDate(Datetime(20110104), Datetime(20111206), KQuery::DAY);
    Datetime date(201106010000);
    ScoreRecordList cs = buildCrossSection();
    REQUIRE_EQ(cs.size(), 5);

    auto f1 = SCFilter_IgnoreNan();
    auto f2 = SCFilter_TopN(2);

    auto x = f1 | f2;               // IgnoreNan -> TopN(2)
    auto y = x | SCFilter_TopN(1);  // IgnoreNan -> TopN(2) -> TopN(1)

    /** @arg reusing the combined node as the left operand does not modify it */
    CHECK_EQ(x->filter(cs, date, query).size(), 2);

    /** @arg the operands of the earlier combine stay unchanged */
    CHECK_EQ(f1->filter(cs, date, query).size(), 4);
    CHECK_EQ(f2->filter(cs, date, query).size(), 2);

    /** @arg the new chain applies in order: IgnoreNan -> TopN(2) -> TopN(1) */
    ScoreRecordList ret = y->filter(cs, date, query);
    REQUIRE_EQ(ret.size(), 1);
    CHECK_EQ(ret[0].value, doctest::Approx(5.0));
}

/** @par Test points */
TEST_CASE("test_ScoresFilterBase_operator_or_null_operand") {
    KQuery query = KQueryByDate(Datetime(20110104), Datetime(20111206), KQuery::DAY);
    Datetime date(201106010000);
    ScoreRecordList cs = buildCrossSection();
    REQUIRE_EQ(cs.size(), 5);

    auto f1 = SCFilter_TopN(2);
    ScoresFilterPtr null_filter;

    /** @arg a | null returns a clone of a, without attaching any child */
    auto ret1 = f1 | null_filter;
    CHECK_FALSE(ret1 == f1);
    CHECK_EQ(ret1->filter(cs, date, query).size(), 2);

    /** @arg changing a afterwards does not affect the returned clone */
    f1->setParam<int>("topn", 4);
    CHECK_EQ(ret1->filter(cs, date, query).size(), 2);

    /** @arg null | b returns a clone of b */
    auto f2 = SCFilter_TopN(3);
    auto ret2 = null_filter | f2;
    CHECK_FALSE(ret2 == f2);
    CHECK_EQ(ret2->filter(cs, date, query).size(), 3);
    f2->setParam<int>("topn", 5);
    CHECK_EQ(ret2->filter(cs, date, query).size(), 3);

    /** @arg null | null returns null */
    auto ret3 = null_filter | null_filter;
    CHECK_FALSE(ret3);
}

/** @} */
