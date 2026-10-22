/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-10-22
 *      Author: fasiondog
 */

#include "../../test_config.h"
#include <algorithm>
#include <cmath>
#include <vector>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/multifactor/buildin_scfilter.h>

using namespace hku;

/**
 * @defgroup test_GroupSCFilter test_GroupSCFilter
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

namespace {

// A cross-section whose score value equals its index (0..total-1), so the filter's index slicing
// can be checked purely by the returned score values without depending on real stock data.
ScoreRecordList buildLinearScores(size_t total) {
    ScoreRecordList scores;
    Stock stk;  // the Group filter only slices the list, the stock itself is unused
    for (size_t i = 0; i < total; i++) {
        scores.emplace_back(stk, static_cast<ScoreRecord::value_t>(i));
    }
    return scores;
}

// Concatenate every group's returned values in group order.
std::vector<ScoreRecord::value_t> concatGroups(int group, const ScoreRecordList& scores) {
    std::vector<ScoreRecord::value_t> all;
    for (int g = 0; g < group; g++) {
        auto filter = SCFilter_Group(group, g);
        ScoreRecordList ret = filter->filter(
          scores, Datetime(201106010000),
          KQueryByDate(Datetime(20110104), Datetime(20111206), KQuery::DAY));
        for (const auto& sc : ret) {
            all.emplace_back(sc.value);
        }
    }
    return all;
}

}  // namespace

/** @par Test points */
TEST_CASE("test_GroupSCFilter_divisible_equal_sizes") {
    const size_t total = 100, group = 10;
    ScoreRecordList scores = buildLinearScores(total);
    REQUIRE_EQ(scores.size(), total);

    for (int g = 0; g < (int)group; g++) {
        auto filter = SCFilter_Group(group, g);
        ScoreRecordList ret = filter->filter(
          scores, Datetime(201106010000),
          KQueryByDate(Datetime(20110104), Datetime(20111206), KQuery::DAY));

        /** @arg a divisible split gives every group exactly total/group records, in input order */
        REQUIRE_EQ(ret.size(), total / group);
        for (size_t i = 0; i < ret.size(); i++) {
            CHECK_EQ(ret[i].value, static_cast<ScoreRecord::value_t>(g * (total / group) + i));
        }
    }
}

/** @par Test points */
TEST_CASE("test_GroupSCFilter_non_divisible_no_empty_group") {
    // total = 95, group = 10: the old round-up scheme gave the last group only 5 and the split
    // was front-heavy; quantile cut must leave no group empty (total >= group) and cover all.
    const size_t total = 95, group = 10;
    ScoreRecordList scores = buildLinearScores(total);
    REQUIRE_EQ(scores.size(), total);

    size_t prev_size = 0;
    for (int g = 0; g < (int)group; g++) {
        auto filter = SCFilter_Group(group, g);
        ScoreRecordList ret = filter->filter(
          scores, Datetime(201106010000),
          KQueryByDate(Datetime(20110104), Datetime(20111206), KQuery::DAY));

        /** @arg every group is non-empty once total >= group */
        REQUIRE_FALSE(ret.empty());
        /** @arg sizes differ by at most 1 between adjacent groups */
        if (g > 0) {
            CHECK_LE(
              std::abs(static_cast<long long>(ret.size()) - static_cast<long long>(prev_size)), 1);
        }
        prev_size = ret.size();
    }

    /** @arg concatenating every group in order reconstructs the full ordered cross-section */
    auto all = concatGroups(group, scores);
    REQUIRE_EQ(all.size(), total);
    for (size_t i = 0; i < all.size(); i++) {
        CHECK_EQ(all[i], static_cast<ScoreRecord::value_t>(i));
    }
}

/** @par Test points */
TEST_CASE("test_GroupSCFilter_total_less_than_group_partitions") {
    // total = 5, group = 10: the old scheme overlapped stocks across groups and left 5 groups
    // empty; quantile cut must put each stock into exactly one group with no overlap/omission.
    const size_t total = 5, group = 10;
    ScoreRecordList scores = buildLinearScores(total);
    REQUIRE_EQ(scores.size(), total);

    std::vector<ScoreRecord::value_t> all = concatGroups(group, scores);

    /** @arg every stock appears exactly once across all groups (no overlap, no omission) */
    std::sort(all.begin(), all.end());
    REQUIRE_EQ(all.size(), total);
    for (size_t i = 0; i < all.size(); i++) {
        CHECK_EQ(all[i], static_cast<ScoreRecord::value_t>(i));
    }

    /** @arg no single group holds more than one stock when total < group */
    for (int g = 0; g < (int)group; g++) {
        auto filter = SCFilter_Group(group, g);
        ScoreRecordList ret = filter->filter(
          scores, Datetime(201106010000),
          KQueryByDate(Datetime(20110104), Datetime(20111206), KQuery::DAY));
        CHECK_LE(ret.size(), 1);
    }
}

/** @par Test points */
TEST_CASE("test_GroupSCFilter_boundaries") {
    const int group = 10;
    const auto kquery = KQueryByDate(Datetime(20110104), Datetime(20111206), KQuery::DAY);

    /** @arg an empty cross-section yields an empty list for every group */
    for (int g = 0; g < group; g++) {
        auto filter = SCFilter_Group(group, g);
        CHECK_UNARY(filter->filter(ScoreRecordList(), Datetime(201106010000), kquery).empty());
    }

    /** @arg a non-positive group or an out-of-range group index is rejected on construction */
    CHECK_THROWS(SCFilter_Group(0, 0));
    CHECK_THROWS(SCFilter_Group(-1, 0));
    CHECK_THROWS(SCFilter_Group(group, group));
    CHECK_THROWS(SCFilter_Group(group, -1));

    /** @arg total == group puts exactly one stock into each group */
    auto all = concatGroups(group, buildLinearScores(static_cast<size_t>(group)));
    REQUIRE_EQ(all.size(), static_cast<size_t>(group));
    for (size_t i = 0; i < all.size(); i++) {
        CHECK_EQ(all[i], static_cast<ScoreRecord::value_t>(i));
    }
}

/** @} */
