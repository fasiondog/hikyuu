/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2026-10-12
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <set>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/CYCLE.h>
#include <hikyuu/indicator/crt/KDATA.h>

using namespace hku;

/**
 * @defgroup test_indicator_CYCLE test_indicator_CYCLE
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_CYCLE") {
    StockManager& sm = StockManager::instance();
    KData k = sm["sh600004"].getKData(KQuery(-100));
    if (k.empty()) {
        return;
    }

    /** @arg query mode rebalances every adjust_cycle bars from the first bar */
    Indicator r = CYCLE(3, "query")(k);
    CHECK_EQ(r.name(), "CYCLE");
    CHECK_EQ(r.size(), k.size());
    CHECK_EQ(r[0], 1);
    CHECK_EQ(r[1], 0);
    CHECK_EQ(r[3], 1);
    CHECK_EQ(r[6], 1);
}

/** @par Test points */
TEST_CASE("test_CYCLE_quarter_no_delay") {
    StockManager& sm = StockManager::instance();
    KData k = sm["sh600004"].getKData(KQuery(-250));
    if (k.empty()) {
        return;
    }

    /** @arg quarter mode fires on the N-th day of the quarter, at most once per quarter */
    Indicator r = CYCLE(1, "quarter", false)(k);
    std::set<Datetime> quarters;
    size_t hits = 0;
    for (size_t i = 0; i < r.size(); ++i) {
        if (r[i] > 0.0) {
            hits++;
            Datetime d = k[i].datetime;
            CHECK_UNARY(d == d.startOfQuarter());
            CHECK_UNARY(quarters.find(d.startOfQuarter()) == quarters.end());
            quarters.insert(d.startOfQuarter());
        }
    }
    CHECK_UNARY(hits >= 1);
}

/** @par Test points */
TEST_CASE("test_CYCLE_adjust_mode_case_insensitive") {
    StockManager& sm = StockManager::instance();
    KData k = sm["sh600004"].getKData(KQuery(-100));
    if (k.empty()) {
        return;
    }

    /** @arg mixed-case adjust_mode matches the lowercase result instead of all-zero */
    Indicator upper = CYCLE(2, "Week")(k);
    Indicator lower = CYCLE(2, "week")(k);
    CHECK_EQ(upper.size(), lower.size());
    size_t hits = 0;
    for (size_t i = 0; i < lower.size(); ++i) {
        CHECK_EQ(upper[i], lower[i]);
        if (lower[i] > 0.0) {
            hits++;
        }
    }
    CHECK_UNARY(hits > 0);

    /** @arg mixed-case query mode keeps the every-cycle shape */
    Indicator q = CYCLE(3, "Query")(k);
    Indicator r = CYCLE(3, "query")(k);
    for (size_t i = 0; i < r.size(); ++i) {
        CHECK_EQ(q[i], r[i]);
    }
    CHECK_EQ(q[0], 1);
}

/** @} */
