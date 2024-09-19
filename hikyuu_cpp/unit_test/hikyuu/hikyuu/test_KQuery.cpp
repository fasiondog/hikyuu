/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240916 added by fasiondog
 */

#include "../test_config.h"
#include <hikyuu/KQuery.h>

using namespace hku;

/**
 * @defgroup test_hikyuu_KQuery test_hikyuu_KQuery
 * @ingroup test_hikyuu_base_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_KQuery") {
    KQuery q1 = KQueryByIndex(-1);
    CHECK_EQ(q1.start(), -1);
    CHECK_UNARY(q1.end() == Null<int64_t>());
    CHECK_UNARY(q1.startDatetime() == Null<Datetime>());
    CHECK_UNARY(q1.endDatetime() == Null<Datetime>());

    q1 = KQueryByDate(Datetime(20010101), Datetime(20010110));
    CHECK_UNARY(q1.start() == Null<int64_t>());
    CHECK_UNARY(q1.end() == Null<int64_t>());
    CHECK_EQ(q1.startDatetime(), Datetime(20010101));
    CHECK_EQ(q1.endDatetime(), Datetime(20010110));
}

/** @par 检测点 */
TEST_CASE("test_KQuery_equal") {
    KQuery q1 = KQueryByIndex(-1);
    KQuery q2 = KQueryByIndex(-1);
    CHECK_EQ(q1, q2);

    q1 = KQueryByIndex(-1);
    q2 = KQueryByIndex(2);
    CHECK_NE(q1, q2);

    q1 = KQueryByIndex(2);
    q2 = KQueryByIndex(2, 3);
    CHECK_NE(q1, q2);

    q1 = KQueryByIndex(2);
    q2 = KQueryByIndex(2);
    CHECK_EQ(q1, q2);

    q1 = KQueryByIndex(2, 10);
    q2 = KQueryByIndex(2, 10);
    CHECK_EQ(q1, q2);

    q1 = KQueryByDate(Datetime(20010101));
    q2 = KQueryByIndex(2, 10);
    CHECK_NE(q1, q2);

    q1 = KQueryByDate(Datetime(20010101));
    q2 = KQueryByDate(Datetime(20010101));
    CHECK_EQ(q1, q2);

    q1 = KQueryByDate(Datetime(20010101));
    q2 = KQueryByDate(Datetime(20010101), Datetime(20010102));
    CHECK_NE(q1, q2);

    q1 = KQueryByDate(Datetime(20010101), Datetime(20010110));
    q2 = KQueryByDate(Datetime(20010101), Datetime(20010110));
    CHECK_EQ(q1, q2);
}

/** @} */
