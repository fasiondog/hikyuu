/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2026-10-12
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/TIMELINE.h>
#include <hikyuu/indicator/crt/TIMELINEVOL.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/MA.h>

using namespace hku;

/**
 * @defgroup test_indicator_TIMELINE test_indicator_TIMELINE
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_TIMELINE") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm["sh600004"];
    KData k = stk.getKData(KQuery(-10));
    if (k.empty()) {
        return;
    }
    TimeLineList tl = stk.getTimeLineList(k.getQuery());

    /** @arg leaf usage: the result covers the time-line series, length follows the data */
    Indicator t = TIMELINE()(k);
    CHECK_EQ(t.name(), "TIMELINE");
    CHECK_EQ(t.size(), tl.size());
    CHECK_EQ(t.discard(), 0);
    if (!tl.empty()) {
        CHECK_EQ(t[0], doctest::Approx(tl[0].price));
        CHECK_EQ(t[tl.size() - 1], doctest::Approx(tl[tl.size() - 1].price));
    }

    /** @arg TIMELINEVOL reads the same series with the vol part */
    Indicator v = TIMELINEVOL()(k);
    CHECK_EQ(v.name(), "TIMELINEVOL");
    CHECK_EQ(v.size(), tl.size());
    if (!tl.empty()) {
        CHECK_EQ(v[0], tl[0].vol);
    }

    /** @arg date accessors fall back to the context K-line dates (leaf-usage caveat) */
    CHECK_EQ(t.getDatetimeList().size(), k.size());
}

/** @par Test points */
TEST_CASE("test_TIMELINE_context_length_contract") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm["sh600004"];
    KData k = stk.getKData(KQuery(-10));
    if (k.empty()) {
        return;
    }
    TimeLineList tl = stk.getTimeLineList(k.getQuery());

    /** @arg feeding TIMELINE into another indicator fails fast when lengths differ */
    if (tl.size() != k.size()) {
        CHECK_THROWS_AS(MA(TIMELINE(), 5)(k), std::exception);
    } else {
        Indicator r = MA(TIMELINE(), 5)(k);
        CHECK_EQ(r.size(), k.size());
    }
}

/** @} */
