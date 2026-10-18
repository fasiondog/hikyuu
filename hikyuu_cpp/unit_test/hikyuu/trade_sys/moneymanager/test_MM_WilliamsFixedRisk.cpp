/*
 * test_MM_WilliamsFixedRisk.cpp
 *
 *  Created on: 2026-10-14
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/data_driver/DataDriverFactory.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_WilliamsFixedRisk.h>

using namespace hku;

/**
 * @defgroup test_MM_WilliamsFixedRisk test_MM_WilliamsFixedRisk
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_MM_WilliamsFixedRisk_param_check") {
    /** @arg p must be within (0, 1], so a leverage-like p > 1 is rejected */
    CHECK_THROWS_AS(MM_WilliamsFixedRisk(0.0, 1000.0), std::exception);
    CHECK_THROWS_AS(MM_WilliamsFixedRisk(-0.1, 1000.0), std::exception);
    CHECK_THROWS_AS(MM_WilliamsFixedRisk(2.0, 1000.0), std::exception);
    CHECK_NOTHROW(MM_WilliamsFixedRisk(1.0, 1000.0));
    CHECK_NOTHROW(MM_WilliamsFixedRisk(0.1, 1000.0));

    /** @arg max_loss must be positive */
    CHECK_THROWS_AS(MM_WilliamsFixedRisk(0.1, 0.0), std::exception);
    CHECK_THROWS_AS(MM_WilliamsFixedRisk(0.1, -1000.0), std::exception);
}

/** @par Test points */
TEST_CASE("test_MM_WilliamsFixedRisk_getBuyNumber") {
    Stock stock("TEST", "MMWFR", "MM Williams Fixed Risk", 1, true, Datetime(199001010000),
                Datetime(209901010000), 1.0, 1.0, 2, 1, 1000000);
    Parameter driver_param;
    driver_param.set<string>("type", "DoNothing");
    stock.setKDataDriver(DataDriverFactory::getKDataDriverPool(driver_param));
    REQUIRE_UNARY(!stock.isNull());

    /** @arg quantity = cash * p / max_loss, i.e. 100000 * 0.1 / 1000 = 10 */
    auto mm = MM_WilliamsFixedRisk(0.1, 1000.0);
    mm->setTM(crtTM(Datetime(200001010000), 100000.0));
    CHECK_EQ(mm->getBuyNumber(Datetime(200001010000), stock, 10.0, 1.0, PART_SIGNAL), 10.0);

    /** @arg A larger p scales the quantity up within the (0, 1] bound */
    auto mm2 = MM_WilliamsFixedRisk(0.5, 1000.0);
    mm2->setTM(crtTM(Datetime(200001010000), 100000.0));
    CHECK_EQ(mm2->getBuyNumber(Datetime(200001010000), stock, 10.0, 1.0, PART_SIGNAL), 50.0);
}

/** @par Test points */
TEST_CASE("test_MM_WilliamsFixedRisk_short") {
    Stock stock("TEST", "MMWFR", "MM Williams Fixed Risk", 1, true, Datetime(199001010000),
                Datetime(209901010000), 1.0, 1.0, 2, 1, 1000000);
    Parameter driver_param;
    driver_param.set<string>("type", "DoNothing");
    stock.setKDataDriver(DataDriverFactory::getKDataDriverPool(driver_param));
    REQUIRE_UNARY(!stock.isNull());

    /** @arg The short entry uses the same sizing as the long buy: 100000 * 0.1 / 1000 = 10, and
     * the per-share risk is not used */
    auto mm = MM_WilliamsFixedRisk(0.1, 1000.0);
    mm->setTM(crtTM(Datetime(200001010000), 100000.0));
    CHECK_EQ(mm->getSellShortNumber(Datetime(200001010000), stock, 10.0, 1.0, PART_SIGNAL), 10.0);
    CHECK_EQ(mm->getSellShortNumber(Datetime(200001010000), stock, 10.0, 5.0, PART_SIGNAL), 10.0);

    /** @arg The short cover liquidates the entire position */
    CHECK_UNARY(
      (mm->getBuyShortNumber(Datetime(200001010000), stock, 10.0, 1.0, PART_SIGNAL) == MAX_DOUBLE));
}

/** @} */
