/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2026-10-12
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <cmath>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/ADVANCE.h>
#include <hikyuu/indicator/crt/DECLINE.h>
#include <hikyuu/indicator/crt/KDATA.h>

using namespace hku;

/**
 * @defgroup test_indicator_ADVANCE test_indicator_ADVANCE
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_ADVANCE_empty_context") {
    /** @arg Factory call has no context: fall back to param query/market, matching DECLINE */
    KQuery q = KQueryByIndex(-10);
    Indicator a = ADVANCE(q, "SH", STOCKTYPE_A);
    Indicator d = DECLINE(q, "SH", STOCKTYPE_A);
    CHECK_EQ(a.name(), "ADVANCE");
    CHECK_EQ(d.name(), "DECLINE");
    CHECK_UNARY(a.size() > 0);
    CHECK_EQ(a.size(), d.size());
    CHECK_EQ(a.discard(), d.discard());
}

/** @par Test points */
TEST_CASE("test_ADVANCE_with_context") {
    StockManager& sm = StockManager::instance();
    KData k = sm["sh600004"].getKData(KQuery(-10));
    HKU_IF_RETURN(k.empty(), void());

    /** @arg Non-index stock context: use context query/market/type as the aggregation basis */
    Indicator a = ADVANCE()(k);
    CHECK_EQ(a.name(), "ADVANCE");
    CHECK_EQ(a.size(), k.size());
    for (size_t i = a.discard(); i < a.size(); ++i) {
        CHECK_UNARY(std::isnan(a[i]) || std::isfinite(a[i]));
    }
}

/** @} */
