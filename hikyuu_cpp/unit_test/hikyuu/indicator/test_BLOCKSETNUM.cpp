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
    REQUIRE_FALSE(bl.empty());
    Block block = bl[0];
    REQUIRE_FALSE(block.empty());

    KQuery q(0, 200);
    DatetimeList dates = sm.getTradingCalendar(q, "SH");
    size_t total = dates.size();
    REQUIRE_UNARY(total >= 50);

    Indicator r = BLOCKSETNUM(block, q);
    CHECK_EQ(r.name(), "BLOCKSETNUM");
    CHECK_EQ(r.size(), total);
    CHECK_EQ(r.discard(), 0);

    /** @arg every bar equals a manual linear membership scan (binary search equivalence) */
    for (size_t i = 0; i < total; ++i) {
        size_t manual = 0;
        for (auto iter = block.begin(); iter != block.end(); ++iter) {
            Datetime last = iter->lastDatetime().isNull() ? Datetime::max() : iter->lastDatetime();
            if (dates[i] >= iter->startDatetime() && dates[i] <= last) {
                manual++;
            }
        }
        CHECK_EQ(r[i], (double)manual);
    }

    /** @arg context-bound usage counts over the context dates point by point */
    KData k = sm.getStock("sh600000").getKData(q);
    REQUIRE_FALSE(k.empty());
    Indicator r2 = BLOCKSETNUM(block);
    r2.setContext(k);
    CHECK_EQ(r2.size(), k.size());
    DatetimeList kdates = k.getDatetimeList();
    for (size_t i = 0; i < k.size(); ++i) {
        size_t manual = 0;
        for (auto iter = block.begin(); iter != block.end(); ++iter) {
            Datetime last = iter->lastDatetime().isNull() ? Datetime::max() : iter->lastDatetime();
            if (kdates[i] >= iter->startDatetime() && kdates[i] <= last) {
                manual++;
            }
        }
        CHECK_EQ(r2[i], (double)manual);
    }

    /** @arg A temporary stock carries no registered validity range (setKRecordList leaves it
     *  Null); it must still count as a member over the whole context instead of being dropped */
    Stock tmp_stock("TEST", "BSNTMP", "BlockSetNum temp stock");
    tmp_stock.setKRecordList({KRecord(Datetime(202001020000), 10.0, 11.0, 9.0, 10.0, 100.0, 100.0)},
                             KQuery::DAY);
    REQUIRE_EQ(tmp_stock.getCount(KQuery::DAY), 1u);
    CHECK_UNARY(tmp_stock.startDatetime().isNull());
    CHECK_UNARY(tmp_stock.lastDatetime().isNull());
    Block tmp_block("TEST", "BLOCKSETNUM_TMP");
    REQUIRE_UNARY(tmp_block.add(tmp_stock));
    Indicator r3 = BLOCKSETNUM(tmp_block);
    r3.setContext(k);
    CHECK_EQ(r3.size(), k.size());
    for (size_t i = 0; i < k.size(); ++i) {
        CHECK_EQ(r3[i], 1.0);
    }
}

/** @} */
