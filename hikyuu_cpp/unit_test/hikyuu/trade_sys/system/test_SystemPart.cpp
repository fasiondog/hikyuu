/*
 * test_SystemPart.cpp
 *
 *  Created on: 2013-4-29
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/trade_sys/system/SystemPart.h>

using namespace hku;

/**
 * @defgroup test_SystemPart test_SystemPart
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_getSystemPartName") {
    SystemPart part = PART_ENVIRONMENT;
    CHECK_EQ(getSystemPartName(part), "EV");

    part = PART_CONDITION;
    CHECK_EQ(getSystemPartName(part), "CN");

    part = PART_SIGNAL;
    CHECK_EQ(getSystemPartName(part), "SG");

    part = PART_STOPLOSS;
    CHECK_EQ(getSystemPartName(part), "ST");

    part = PART_MONEYMANAGER;
    CHECK_EQ(getSystemPartName(part), "MM");

    part = PART_PROFITGOAL;
    CHECK_EQ(getSystemPartName(part), "PG");

    part = PART_SLIPPAGE;
    CHECK_EQ(getSystemPartName(part), "SP");

    part = PART_INVALID;
    CHECK_EQ(getSystemPartName(part), "--");
    CHECK_EQ(getSystemPartName(part + 1), "--");
}

/** @par 检测点 */
TEST_CASE("test_getSystemPartEnum") {
    string part;
    part = "EV";
    CHECK_EQ(getSystemPartEnum(part), PART_ENVIRONMENT);

    part = "CN";
    CHECK_EQ(getSystemPartEnum(part), PART_CONDITION);

    part = "SG";
    CHECK_EQ(getSystemPartEnum(part), PART_SIGNAL);

    part = "ST";
    CHECK_EQ(getSystemPartEnum(part), PART_STOPLOSS);

    part = "TP";
    CHECK_EQ(getSystemPartEnum(part), PART_TAKEPROFIT);

    part = "MM";
    CHECK_EQ(getSystemPartEnum(part), PART_MONEYMANAGER);

    part = "PG";
    CHECK_EQ(getSystemPartEnum(part), PART_PROFITGOAL);

    part = "SP";
    CHECK_EQ(getSystemPartEnum(part), PART_SLIPPAGE);

    part = "E";
    CHECK_EQ(getSystemPartEnum(part), PART_INVALID);

    part = "--";
    CHECK_EQ(getSystemPartEnum(part), PART_INVALID);
}

/** @} */
