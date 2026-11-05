/*
 * test_IndicatorImp_increment.cpp
 *
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-11-04
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <hikyuu/KDataPrivatedBufferImp.h>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/KDATA.h>

using namespace hku;

/**
 * @defgroup test_indicator_increment test_indicator_increment
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_increment_execute_rejects_context_overflow") {
    Stock stock = StockManager::instance().getStock("sh000001");
    REQUIRE_FALSE(stock.isNull());
    KData real = stock.getKData(KQuery(0, 120));
    REQUIRE_EQ(real.size(), 120);

    // A fabricated context holding three rows per date, the shape of a factor value table filled
    // by repeated backfills. It is longer than the real K line rebound afterwards, so the cached
    // results do not fit into the new buffer when shifted
    KRecordList records;
    for (size_t i = 0; i < 100; ++i) {
        KRecord record(real[i].datetime, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
        records.push_back(record);
        records.push_back(record);
        records.push_back(record);
    }
    KData forged = KData(make_shared<KDataPrivatedBufferImp>(stock, KQuery(0, 120), records));
    REQUIRE_EQ(forged.size(), 300);

    // Only a node holding two operands takes the binary increment path
    Indicator target = CLOSE() + CLOSE();
    target.setContext(forged);
    REQUIRE_EQ(target.size(), 300);

    Indicator expect = CLOSE() + CLOSE();
    expect.setContext(real);

    /** @arg the incremental shift falls back to the full recalculation when the cached context
     *       does not fit the new one, instead of writing past the end of the buffer */
    target.setContext(real);
    REQUIRE_EQ(target.size(), real.size());
    for (size_t i = 0; i < real.size(); ++i) {
        if (std::isnan(expect[i])) {
            CHECK_UNARY(std::isnan(target[i]));
        } else {
            CHECK_EQ(target[i], doctest::Approx(expect[i]));
        }
    }
}

/** @par Test points */
TEST_CASE("test_increment_execute_if_rejects_context_overflow") {
    Stock stock = StockManager::instance().getStock("sh000001");
    REQUIRE_FALSE(stock.isNull());
    KData real = stock.getKData(KQuery(0, 120));
    REQUIRE_EQ(real.size(), 120);

    KRecordList records;
    for (size_t i = 0; i < 100; ++i) {
        KRecord record(real[i].datetime, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
        records.push_back(record);
        records.push_back(record);
        records.push_back(record);
    }
    KData forged = KData(make_shared<KDataPrivatedBufferImp>(stock, KQuery(0, 120), records));
    REQUIRE_EQ(forged.size(), 300);

    // An IF-rooted node takes its own incremental path
    Indicator target = IF(CLOSE() > 0, CLOSE() + CLOSE(), CVAL(CLOSE(), 0.0));
    target.setContext(forged);
    REQUIRE_EQ(target.size(), 300);

    Indicator expect = IF(CLOSE() > 0, CLOSE() + CLOSE(), CVAL(CLOSE(), 0.0));
    expect.setContext(real);

    /** @arg the incremental shift of the IF node falls back to the full recalculation when the
     *       cached context does not fit the new one */
    target.setContext(real);
    REQUIRE_EQ(target.size(), real.size());
    for (size_t i = 0; i < real.size(); ++i) {
        if (std::isnan(expect[i])) {
            CHECK_UNARY(std::isnan(target[i]));
        } else {
            CHECK_EQ(target[i], doctest::Approx(expect[i]));
        }
    }
}

/** @} */
