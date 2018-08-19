/*
 * test_SystemPart.cpp
 *
 *  Created on: 2013-4-29
 *      Author: fasiondog
 */


#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_trade_sys_suite
    #include <boost/test/unit_test.hpp>
#endif

#include <hikyuu/trade_sys/system/SystemPart.h>

using namespace hku;

/**
 * @defgroup test_SystemPart test_SystemPart
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_getSystemPartName) {
    SystemPart part = PART_ENVIRONMENT;
    BOOST_CHECK(getSystemPartName(part) == "EV");

    part = PART_CONDITION;
    BOOST_CHECK(getSystemPartName(part) == "CN");

    part = PART_SIGNAL;
    BOOST_CHECK(getSystemPartName(part) == "SG");

    part = PART_STOPLOSS;
    BOOST_CHECK(getSystemPartName(part) == "ST");

    part = PART_MONEYMANAGER;
    BOOST_CHECK(getSystemPartName(part) == "MM");

    part = PART_PROFITGOAL;
    BOOST_CHECK(getSystemPartName(part) == "PG");

    part = PART_SLIPPAGE;
    BOOST_CHECK(getSystemPartName(part) == "SP");

    part = PART_INVALID;
    BOOST_CHECK(getSystemPartName(part) == "--");
    BOOST_CHECK(getSystemPartName(part + 1) == "--");
}


/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_getSystemPartEnum) {
    string part;
    part = "EV";
    BOOST_CHECK(getSystemPartEnum(part) == PART_ENVIRONMENT);

    part = "CN";
    BOOST_CHECK(getSystemPartEnum(part) == PART_CONDITION);

    part = "SG";
    BOOST_CHECK(getSystemPartEnum(part) == PART_SIGNAL);

    part = "ST";
    BOOST_CHECK(getSystemPartEnum(part) == PART_STOPLOSS);

    part = "TP";
    BOOST_CHECK(getSystemPartEnum(part) == PART_TAKEPROFIT);

    part = "MM";
    BOOST_CHECK(getSystemPartEnum(part) == PART_MONEYMANAGER);

    part = "PG";
    BOOST_CHECK(getSystemPartEnum(part) == PART_PROFITGOAL);

    part = "SP";
    BOOST_CHECK(getSystemPartEnum(part) == PART_SLIPPAGE);

    part = "E";
    BOOST_CHECK(getSystemPartEnum(part) == PART_INVALID);

    part = "--";
    BOOST_CHECK(getSystemPartEnum(part) == PART_INVALID);
}

/** @} */


