/*
 * test_ISLIMITUP.cpp
 *
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created for: the regression of the previous K-line read at index 0 of IIsLimitUp
 */

#include "doctest/doctest.h"
#include <cmath>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/ISLIMITUP.h>

using namespace hku;

/**
 * @defgroup test_indicator_ISLIMITUP test_indicator_ISLIMITUP
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
TEST_CASE("test_ISLIMITUP") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData kdata = stock.getKData(KQuery(0, 20));
    CHECK_EQ(kdata.size(), 20);

    Indicator result = ISLIMITUP();
    result.setContext(kdata);
    CHECK_EQ(result.name(), "ISLIMITUP");
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

/** @} */
