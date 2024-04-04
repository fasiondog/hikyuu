/*
 * test_FixedPercent_SL.cpp
 *
 *  Created on: 2013-4-19
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/stoploss/crt/ST_FixedPercent.h>

using namespace hku;

/**
 * @defgroup test_FixedPercent_SL test_FixedPercent_SL
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
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

    /** @arg 非法参数 */
    CHECK_THROWS_AS(ST_FixedPercent(0.0), std::exception);
    CHECK_THROWS_AS(ST_FixedPercent(1.01), std::exception);
    CHECK_THROWS_AS(ST_FixedPercent(-1.0), std::exception);
}

/** @} */
