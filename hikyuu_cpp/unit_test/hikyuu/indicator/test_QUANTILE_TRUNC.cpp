/*
 * test_QUANTILE_TRUNC.cpp
 *
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created for: the regression of the negative window offset of IQuantileTrunc incremental
 *  calculate
 */

#include "../test_config.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/QUANTILE_TRUNC.h>
#include <hikyuu/indicator/crt/KDATA.h>

using namespace hku;

/**
 * @defgroup test_indicator_QUANTILE_TRUNC test_indicator_QUANTILE_TRUNC
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/**
 * @par Test points
 * Incremental calculate must match a full calculation, including a first context that was fully
 * discarded.
 *
 * Background: IQuantileTrunc declared incremental support but not min_increment_start(), so the
 * framework could only raise start_pos with the previous m_discard. When the first context was
 * shorter than n-1, that m_discard degraded to its own size and left start_pos below n-1, making
 * the window pointer `data_ptr + 1 + i - n` read before the input and write values into the range
 * that must stay NaN.
 */
TEST_CASE("test_QUANTILE_TRUNC_increment_equivalence") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData k_full = stock.getKData(KQuery(0, 30));
    CHECK_EQ(k_full.size(), 30);

    Indicator expect = QUANTILE_TRUNC(CLOSE(), 10, 0.1, 0.9);
    expect.setContext(k_full);

    auto check_with_history = [&](size_t history) {
        Indicator got = QUANTILE_TRUNC(CLOSE(), 10, 0.1, 0.9);
        got.setContext(stock.getKData(KQuery(0, history)));  // cache m_old_context
        got.setContext(k_full);                              // extended at the tail
        CHECK_EQ(got.size(), expect.size());
        CHECK_EQ(got.discard(), expect.discard());
        for (size_t i = 0; i < expect.size(); ++i) {
            double a = expect[i];
            double b = got[i];
            if (std::isnan(a) && std::isnan(b)) {
                continue;
            }
            CHECK_EQ(b, doctest::Approx(a).epsilon(0.0001));
        }
    };

    /** @arg The first context was fully discarded, so start_pos fell below n - 1 */
    check_with_history(5);

    /** @arg The first context already had valid results, the usual incremental path */
    check_with_history(20);
}

/** @par Test points: ±Inf inputs are excluded from quantile computation */
TEST_CASE("test_QUANTILE_TRUNC_inf_input") {
    price_t inf_v = std::numeric_limits<price_t>::infinity();
    Indicator data = PRICELIST({1.0, 2.0, inf_v, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0});
    Indicator r = QUANTILE_TRUNC(data, 10, 0.1, 0.9);
    for (size_t i = 0; i < r.size(); i++) {
        CHECK_UNARY(std::isnan(r[i]) || std::isfinite(r[i]));
    }
}

/** @} */
