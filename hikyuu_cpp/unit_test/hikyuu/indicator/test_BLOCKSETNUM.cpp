/*
 * test_BLOCKSETNUM.cpp
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2026-10-12
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/BLOCKSETNUM.h>

using namespace hku;

/**
 * @defgroup test_indicator_BLOCKSETNUM test_indicator_BLOCKSETNUM
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_BLOCKSETNUM") {
    StockManager& sm = StockManager::instance();
    BlockList bl = sm.getBlockList();
    if (bl.empty()) {
        return;
    }
    Block block = bl[0];
    if (block.empty()) {
        return;
    }

    KQuery q(0, 200);
    DatetimeList dates = sm.getTradingCalendar(q, "SH");
    size_t total = dates.size();
    if (total < 50) {
        return;
    }

    Indicator r = BLOCKSETNUM(block, q);
    CHECK_EQ(r.name(), "BLOCKSETNUM");
    CHECK_EQ(r.size(), total);
    CHECK_EQ(r.discard(), 0);

    /** @arg all values in valid range */
    for (size_t i = 0; i < total; ++i) {
        CHECK_UNARY(r[i] >= 0 && r[i] <= (double)block.size());
    }

    /** @arg spot-check against manual linear scan at a middle bar */
    size_t mid = total / 2;
    size_t manual = 0;
    for (auto iter = block.begin(); iter != block.end(); ++iter) {
        Datetime last = iter->lastDatetime().isNull() ? Datetime::max() : iter->lastDatetime();
        if (dates[mid] >= iter->startDatetime() && dates[mid] <= last) {
            manual++;
        }
    }
    CHECK_EQ(r[mid], (double)manual);

    /** @arg incremental path matches static */
    KData k = sm.getStock("sh600000").getKData(q);
    if (k.size() >= 5) {
        Indicator r2 = BLOCKSETNUM(block);
        r2.setContext(k);
        CHECK_EQ(r2.size(), k.size());
    }
}

/** @} */
