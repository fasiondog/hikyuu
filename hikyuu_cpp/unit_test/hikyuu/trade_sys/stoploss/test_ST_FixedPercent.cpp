/*
 * test_FixedPercent_SL.cpp
 *
 *  Created on: 2013-4-19
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <cmath>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/stoploss/crt/ST_FixedPercent.h>

using namespace hku;

/**
 * @defgroup test_FixedPercent_SL test_FixedPercent_SL
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_FixedPercent_SL") {
    /** @arg p = 0.2 */
    StoplossPtr sp = ST_FixedPercent(0.2);
    price_t result = sp->getPrice(Datetime(200101010000), 10.0);
    CHECK_LT(std::fabs(result - 8.0), 0.01);

    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sz000001");
    KQuery query(0, 10);
    KData kdata = stock.getKData(query);
    sp->setTO(kdata);
    result = sp->getPrice(Datetime(199101030000), 66.4);
    CHECK_LT(std::fabs(result - 53.12), 0.01);

    /** @arg p = 0.99 */
    sp = ST_FixedPercent(0.99);
    result = sp->getPrice(Datetime(199101030000), 66.397);
    CHECK_EQ(result, doctest::Approx(roundEx(66.397 * 0.01, 2)));

    /** @arg p = 1.0 */
    sp = ST_FixedPercent(1.0);
    result = sp->getPrice(Datetime(199101030000), 66.397);
    CHECK_EQ(result, doctest::Approx(0.0));

    /** @arg Invalid parameters */
    CHECK_THROWS_AS(ST_FixedPercent(0.0), std::exception);
    CHECK_THROWS_AS(ST_FixedPercent(1.01), std::exception);
    CHECK_THROWS_AS(ST_FixedPercent(-1.0), std::exception);
}

/** @arg a stop-loss that cannot be represented by the price tick is invalid (Null), while a normal
 * price keeps a valid level and p = 1.0 still means "no stop-loss" */
TEST_CASE("test_FixedPercent_SL_unrepresentable") {
    // Without setTO the precision falls back to 2, so the price tick is 0.01
    auto sp = ST_FixedPercent(0.5);

    /** @arg the level cannot stay below the price tick: 0.01 * 0.5 = 0.005 rounds to the 0.01
     * tick, which is not below the price, so the one-tick clamp leaves 0 */
    CHECK_UNARY(std::isnan(sp->getPrice(Datetime(200101010000), 0.01)));

    sp = ST_FixedPercent(0.03);
    /** @arg the level would be negative after the one-tick clamp (0.006 - 0.01) */
    CHECK_UNARY(std::isnan(sp->getPrice(Datetime(200101010000), 0.006)));

    /** @arg a regular price still yields the expected level */
    CHECK_EQ(sp->getPrice(Datetime(200101010000), 10.0), doctest::Approx(9.7));

    /** @arg p = 1.0 stays "no stop-loss" (0), it is not reported as invalid */
    sp = ST_FixedPercent(1.0);
    CHECK_EQ(sp->getPrice(Datetime(200101010000), 66.397), doctest::Approx(0.0));
}

/** @} */
