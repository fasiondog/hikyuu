/*
 * test_Decline.cpp
 *
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created for: the regression of the reversed operator bug of IDDecline::_increment_calculate
 *      Author: woleigegg
 */

#include "../test_config.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/DECLINE.h>
#include <hikyuu/indicator/crt/ADVANCE.h>
#include <hikyuu/indicator/crt/KDATA.h>

using namespace hku;

/**
 * @defgroup test_indicator_Decline test_indicator_Decline
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/**
 * @par Test points
 * The core regression: the full calculation == the incremental calculation (setContext is called
 * repeatedly on the same instance to trigger _increment_calculate).
 *
 * Background: IDDecline::_increment_calculate once used '>' instead of '<' in the comparison
 * operator (copied from ADVANCE without changing the sign), so the incremental path counted the
 * number of the rising securities. The bug was exposed only in _increment_calculate; the full path
 * _calculate used '<' correctly.
 *
 * Trigger mechanism: the same Indicator instance must be reused with consecutive setContext calls
 * -- the first setContext (a small window) produces and caches m_old_context and the second
 * setContext (a large window extended at the tail) makes can_increment_calculate() hold, forcing
 * the entry into _increment_calculate. Using operator()/ clone() would drop m_old_context and
 * silently fall back to the full calculation, making the test pass falsely.
 *
 * The catching logic: full uses '<' all along (the correct falling count); the [start_pos, total)
 * part of inc goes through the incremental path and before the fix it used '>' (wrong, counting the
 * rising securities) so it differed from full and CHECK_EQ failed, exposing the bug; after the fix
 * '<' matches full and the test passes.
 */
TEST_CASE("test_Decline_increment_equivalence") {
    StockManager& sm = StockManager::instance();
    // A single stock context (not INDEX) is used to satisfy supportIncrementCalculate
    Stock stk = sm.getStock("SH600000");
    // The first bar of KQuery(-20,-10) and KQuery(-20) are physically aligned and the latter is
    // extended at the tail, satisfying the incremental admission
    KData k_full = stk.getKData(KQuery(-20));
    KData k_partial = stk.getKData(KQuery(-20, -10));

    // (1) The full baseline: an independent instance
    Indicator ind_full = DECLINE();
    ind_full.setContext(k_full);

    // (2) The incremental test: the same instance is reused with consecutive setContext to trigger
    // the internal state machine
    Indicator ind_inc = DECLINE();
    ind_inc.setContext(k_partial);  // Cache m_old_context
    ind_inc.setContext(k_full);     // Extended at the tail -> enter _increment_calculate

    CHECK_EQ(ind_full.size(), ind_inc.size());
    CHECK_EQ(ind_full.discard(), ind_inc.discard());

    for (size_t i = 0; i < ind_full.size(); ++i) {
        if (std::isnan(ind_full[i]) && std::isnan(ind_inc[i])) {
            continue;
        }
        CHECK_EQ(ind_inc[i], doctest::Approx(ind_full[i]).epsilon(0.0001));
    }

    // (3) The directional hard assertion (a double safety): DECLINE != ADVANCE in a non-sideways
    // market With the bug present the incremental result of ind_inc equaled ADVANCE; after the fix
    // they must differ
    Indicator ind_adv = ADVANCE();
    ind_adv.setContext(k_full);
    size_t last_idx = ind_full.size() - 1;
    if (!std::isnan(ind_full[last_idx]) && !std::isnan(ind_adv[last_idx]) &&
        ind_full[last_idx] != ind_adv[last_idx]) {
        CHECK_NE(ind_inc[last_idx], ind_adv[last_idx]);
    }
}

/**
 * @par Test points
 * DECLINE and ADVANCE mirror each other, locking the direction semantics: DECLINE counts the falls
 * and ADVANCE the rises. Verification: on the trading days with enough stock data (D+A >= 10,
 * excluding the heterogeneous days with very few aligned data) there exists a day with D != A (not
 * all flat, so the < / > operators really distinguish the   falls from the rises). Note: before the
 * fix _increment_calculate using '>' made the incremental DECLINE result equal ADVANCE; after the
 * fix both the full and the incremental paths use '<', so DECLINE and ADVANCE must   differ on a
 * non-sideways day. At some moments one end is NaN due to the ALIGN date alignment (the suspension
 * / not listed   filling), so only the double-value points are verified.
 */
TEST_CASE("test_Decline_vs_Advance_mirror") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm.getStock("SH600000");
    KData k = stk.getKData(KQuery(-20));

    Indicator decline = DECLINE();
    decline.setContext(k);
    Indicator advance = ADVANCE();
    advance.setContext(k);

    CHECK_EQ(decline.name(), "DECLINE");
    CHECK_EQ(advance.name(), "ADVANCE");
    CHECK_EQ(decline.size(), advance.size());

    // Verify the direction distinction at the double-value points with enough data (D+A>=10)
    bool found_distinct = false;
    for (size_t i = 0; i < decline.size(); ++i) {
        if (std::isnan(decline[i]) || std::isnan(advance[i])) {
            continue;
        }
        price_t sum = decline[i] + advance[i];
        if (sum >= 10.0) {
            CHECK_GT(sum, 0.0);
            if (decline[i] != advance[i]) {
                found_distinct = true;
            }
        }
    }
    // There must be a non-flat day (D != A) within 20 days of the real market data
    CHECK_UNARY(found_distinct);
}

/**
 * @par Test points
 * The ignore_context mode: it does not depend on the context and counts the whole market by
 * query/market/stk_type directly. Verify that it calculates normally and that there are valid
 * (non-NaN) output points.
 */
TEST_CASE("test_Decline_ignore_context") {
    Indicator decline = DECLINE(KQuery(-10), "SH", STOCKTYPE_A, true, true);
    CHECK_EQ(decline.name(), "DECLINE");
    CHECK_EQ(decline.empty(), false);
    CHECK_GT(decline.size(), 0);
    // Traverse to find the first valid non-NaN point for the assertion (the discard points may be
    // NaN due to the alignment)
    bool has_valid = false;
    for (size_t i = 0; i < decline.size(); ++i) {
        if (!std::isnan(decline[i])) {
            CHECK_GE(decline[i], 0.0);
            has_valid = true;
            break;
        }
    }
    CHECK_UNARY(has_valid);
}

/** @} */