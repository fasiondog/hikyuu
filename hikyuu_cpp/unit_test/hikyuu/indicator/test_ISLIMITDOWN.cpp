/*
 * test_ISLIMITDOWN.cpp
 *
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created for: the regression of the previous K-line read at index 0 of IIsLimitDown
 */

#include "doctest/doctest.h"
#include <cmath>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/ISLIMITDOWN.h>

using namespace hku;

/**
 * @defgroup test_indicator_ISLIMITDOWN test_indicator_ISLIMITDOWN
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/**
 * @par Test points
 * The first bar has no previous close to compare with, so it must stay NaN rather than be judged
 * from out-of-range data.
 *
 * Background: _calculate ran the sliding judgment from index 0, which read the KRecord before the
 * first one of the context. The garbage result always landed in the slot hidden by discard(), so
 * it never showed up in the output.
 */
TEST_CASE("test_ISLIMITDOWN") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData kdata = stock.getKData(KQuery(0, 20));
    CHECK_EQ(kdata.size(), 20);

    Indicator result = ISLIMITDOWN();
    result.setContext(kdata);
    CHECK_EQ(result.name(), "ISLIMITDOWN");
    CHECK_EQ(result.size(), 20);

    /** @arg The first bar is discarded and left NaN */
    CHECK_EQ(result.discard(), 1);
    CHECK_UNARY(std::isnan(result[0]));

    /** @arg The other bars are judged against the previous close */
    for (size_t i = 1; i < result.size(); ++i) {
        CHECK_UNARY(!std::isnan(result[i]));
        CHECK_UNARY(result[i] == 0.0 || result[i] == 1.0);
    }
}

/** @par Test points */
TEST_CASE("test_ISLIMITDOWN_window_shift_chain") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData k1 = stock.getKData(KQuery(0, 20));
    KData k2 = stock.getKData(KQuery(19, 25));
    KData k3 = stock.getKData(KQuery(24, 30));
    CHECK_EQ(k2.front().datetime, k1.back().datetime);
    CHECK_EQ(k3.front().datetime, k2.back().datetime);

    // Chaining the windows so that every shift starts at the previous last bar drives the
    // incremental path into start_pos == 0 with an already zeroed discard, where the judgment
    // would read the KRecord before the first one of the context
    Indicator r = ISLIMITDOWN();
    r.setContext(k1);
    r.setContext(k2);
    r.setContext(k3);

    /** @arg the chained shift keeps the same values as a fresh calculation */
    Indicator expect = ISLIMITDOWN(k3);
    CHECK_EQ(r.size(), expect.size());
    CHECK_EQ(r.discard(), expect.discard());
    CHECK_EQ(r.discard(), 1);
    for (size_t i = expect.discard(); i < expect.size(); ++i) {
        CHECK_EQ(r[i], expect[i]);
    }
}

/** @par Test points */
TEST_CASE("test_ISLIMITDOWN_ex_right") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sz000001");
    KData k = stock.getKData(KQuery(0, 120));
    REQUIRE_EQ(k.size(), 120);

    /** @arg a no-recover context judges on recovered closes: no Null anywhere */
    auto weights = stock.getWeight(k[0].datetime, k[119].datetime);
    REQUIRE_UNARY(!weights.empty());
    Indicator r = ISLIMITDOWN(k);
    for (size_t i = r.discard(); i < r.size(); ++i) {
        CHECK_UNARY(r[i] == 0.0 || r[i] == 1.0);
    }

    /** @arg the no-recover context judgment equals the backward-recovered one */
    KQuery bq = k.getQuery();
    bq.recoverType(KQuery::BACKWARD);
    KData kb = k.getKData(bq);
    REQUIRE_EQ(kb.size(), k.size());
    Indicator rb = ISLIMITDOWN(kb);
    CHECK_EQ(r.size(), rb.size());
    for (size_t i = r.discard(); i < r.size(); ++i) {
        CHECK_EQ(r[i], rb[i]);
    }
}

/** @} */
