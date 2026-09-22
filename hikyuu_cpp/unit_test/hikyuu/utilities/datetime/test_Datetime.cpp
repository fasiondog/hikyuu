/*
 * test_datetime.cpp
 *
 *  Created on: 2012-8-25
 *      Author: fasiondog
 */

#include "doctest/doctest.h"

#include <hikyuu/utilities/datetime/Datetime.h>
#include <hikyuu/utilities/Null.h>
#include "hikyuu/utilities/Log.h"

using namespace hku;

/**
 * @defgroup test_hikyuu_Datetime test_hikyuu_Datetime
 * @ingroup test_hikyuu_datetime_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_Datetime") {
    Datetime null_datetime = Null<Datetime>();

    /** @arg The default no-argument constructor returns Null<Datetime>() */
    Datetime d;
    CHECK(d == Null<Datetime>());
    CHECK(Null<Datetime>() == d);
    d = Datetime(201208062359);
    CHECK(d < Null<Datetime>());

    /** @arg Constructed from Null<unsigned long long>() */
    d = Datetime(Null<unsigned long long>());
    CHECK(d == Null<Datetime>());

    /** @arg An implicit construction from bd::date */
    d = Datetime(200101010110);
    d = Datetime(d.date());
    CHECK(Datetime(200101010000) == d);

    /** @arg Test the unsigned long long construction */
    CHECK(Datetime(20010101) == Datetime(2001, 1, 1));
    CHECK(Datetime(20101231) == Datetime(2010, 12, 31));
    CHECK(Datetime(200101010102) == Datetime(2001, 1, 1, 1, 2));
    CHECK(Datetime(20010101010205) == Datetime(2001, 1, 1, 1, 2, 5));

    /** @arg Test the string construction */
    d = Datetime(200101010000L);
    CHECK(Datetime("2001-1-1") == d);
    CHECK(Datetime("2001-01-1") == d);
    CHECK(Datetime("2001-1-01") == d);
    CHECK(Datetime("2001-01-01") == d);
    CHECK(Datetime("2001/1/1") == d);
    CHECK(Datetime("2001/1/01") == d);
    CHECK(Datetime("2001/01/1") == d);
    CHECK(Datetime("2001/01/01") == d);
    CHECK(Datetime("20010101") == d);
    d = Datetime(2001, 1, 2, 3, 4, 5, 0, 0);
    CHECK(Datetime("2001-1-2 3:4:5") == d);
    CHECK(Datetime("2001/1/2 3:4:5") == d);
    CHECK(Datetime("20010102T030405") == d);
    CHECK(Datetime("20010102t030405") == d);
    CHECK(Datetime("20010102 3:4:5") == d);
    CHECK_THROWS(Datetime("2001"));

    /** @arg An invalid year */
    CHECK_THROWS_AS(Datetime(99999, 1, 1), std::out_of_range);
    CHECK_THROWS_AS(Datetime(000001010000L), std::out_of_range);

    /** @arg An invalid month */
    CHECK_THROWS_AS(Datetime(2010, 13, 1), std::out_of_range);
    CHECK_THROWS_AS(Datetime(2010, 0, 1), std::out_of_range);

    CHECK_THROWS_AS(Datetime(201013010000L), std::out_of_range);
    CHECK_THROWS_AS(Datetime(201000010000L), std::out_of_range);

    /** @arg An invalid day */
    CHECK_THROWS_AS(Datetime(2010, 1, 0), std::out_of_range);
    CHECK_THROWS_AS(Datetime(2010, 1, 32), std::out_of_range);

    CHECK_THROWS_AS(Datetime(201001000000L), std::out_of_range);
    CHECK_THROWS_AS(Datetime(201001320000L), std::out_of_range);

    /** @arg An invalid hour */
    CHECK_THROWS_AS(Datetime(201001012400L), std::out_of_range);
    CHECK_THROWS_AS(Datetime(201001012500L), std::out_of_range);

    /** @arg An invalid minute */
    CHECK_THROWS_AS(Datetime(201001010060L), std::out_of_range);
    CHECK_THROWS_AS(Datetime(201001010061L), std::out_of_range);

    /** @arg Invalid parameters */
    CHECK_THROWS_AS(Datetime(2001010203041LL), std::out_of_range);

    /** @arg The attribute reading */
    d = Datetime(2012, 8, 6, 23, 59, 2, 13, 15);
    CHECK(2012 == d.year());
    CHECK(8 == d.month());
    CHECK(6 == d.day());
    CHECK(23 == d.hour());
    CHECK(59 == d.minute());
    CHECK(2 == d.second());
    CHECK(13 == d.millisecond());
    CHECK(15 == d.microsecond());

    /** @arg Get the attributes of Null<Datetime> */
    d = Datetime();
    CHECK_THROWS_AS(d.year(), std::logic_error);
    CHECK_THROWS_AS(d.month(), std::logic_error);
    CHECK_THROWS_AS(d.day(), std::logic_error);
    CHECK_THROWS_AS(d.hour(), std::logic_error);
    CHECK_THROWS_AS(d.minute(), std::logic_error);
    CHECK_THROWS_AS(d.second(), std::logic_error);
    CHECK_THROWS_AS(d.millisecond(), std::logic_error);
    CHECK_THROWS_AS(d.microsecond(), std::logic_error);

    /** @arg Convert a normal date into unsigned long long */
    d = Datetime(201208062359);
    unsigned long long x = d.number();
    CHECK(x == 201208062359);

    long long y = x;
    Datetime m(y);
    CHECK(m == d);

    /** @arg The ymdhs series of methods converting into unsigned long long */
    d = Datetime(20120806135249ULL);
    CHECK(d.year() == 2012);
    CHECK(d.month() == 8);
    CHECK(d.day() == 6);
    CHECK(d.hour() == 13);
    CHECK(d.minute() == 52);
    CHECK(d.second() == 49);
    CHECK(d.ym() == 201208ULL);
    CHECK(d.ymd() == 20120806ULL);
    CHECK(d.ymdh() == 2012080613ULL);
    CHECK(d.ymdhm() == 201208061352ULL);
    CHECK(d.ymdhms() == 20120806135249ULL);

    /** @arg Compatible with the oracle datetime notation */
    d = Datetime(2021, 4, 25, 23, 16, 27);
    CHECK_EQ(d.hex(), 0x1415041917101bULL);
    CHECK_EQ(d, Datetime::fromHex(d.hex()));

    /** @arg Convert Null<Datetime>() into a number */
    x = Null<unsigned long long>();
    d = Null<Datetime>();
    CHECK(x == d.number());

    /** @arg Test str */
    d = Datetime("2001-Jan-01 06:30:00");
    CHECK("2001-01-01 06:30:00" == d.str());
    d = Datetime("2001-Jan-01 06:30:11.001");
    CHECK("2001-01-01 06:30:11.001000" == d.str());

    /** @arg Test dayOfWeek */
    CHECK(Datetime(201801010000L).dayOfWeek() == 1);
    CHECK(Datetime(201801060000L).dayOfWeek() == 6);
    CHECK(Datetime(201801070000L).dayOfWeek() == 0);

    /** @arg Test dayOfYear */
    CHECK(Datetime(201801010000L).dayOfYear() == 1);
    CHECK(Datetime(201812310000L).dayOfYear() == 365);

    /** @arg Test dateOfWeek */
    CHECK(null_datetime.dateOfWeek(-1) == null_datetime);
    CHECK(null_datetime.dateOfWeek(0) == null_datetime);
    CHECK(null_datetime.dateOfWeek(4) == null_datetime);
    CHECK(null_datetime.dateOfWeek(6) == null_datetime);
    CHECK(null_datetime.dateOfWeek(7) == null_datetime);
    CHECK(Datetime(201901010000).dateOfWeek(-1) == Datetime(201812300000));
    CHECK(Datetime(201901010000).dateOfWeek(0) == Datetime(201812300000));
    CHECK(Datetime(201901010000).dateOfWeek(1) == Datetime(201812310000));
    CHECK(Datetime(201901010000).dateOfWeek(2) == Datetime(201901010000));
    CHECK(Datetime(201901010000).dateOfWeek(3) == Datetime(201901020000));
    CHECK(Datetime(201901010000).dateOfWeek(4) == Datetime(201901030000));
    CHECK(Datetime(201901010000).dateOfWeek(5) == Datetime(201901040000));
    CHECK(Datetime(201901010000).dateOfWeek(6) == Datetime(201901050000));
    CHECK(Datetime(201901010000).dateOfWeek(7) == Datetime(201901050000));

    /** @arg Test startOfDay */
    CHECK(null_datetime.startOfDay() == null_datetime);
    CHECK(Datetime::min().startOfDay() == Datetime::min());
    CHECK(Datetime::max().startOfDay() == Datetime::max());
    CHECK(Datetime(201812310110).startOfDay() == Datetime(201812310000));

    /** @arg Test endOfDay */
    CHECK(null_datetime.endOfDay() == null_datetime);
    CHECK(Datetime::min().endOfDay() == Datetime(1400, 1, 1, 23, 59, 59));
    CHECK(Datetime::max().endOfDay() == Datetime::max());
    CHECK(Datetime(201812310110).endOfDay() == Datetime(2018, 12, 31, 23, 59, 59));

    /** @arg Test startOfWeek */
    CHECK(null_datetime.startOfWeek() == null_datetime);
    CHECK(Datetime::min().startOfWeek() == Datetime::min());
    CHECK(Datetime::max().startOfWeek() == Datetime(9999, 12, 27));
    CHECK(Datetime(201812310000).startOfWeek() == Datetime(201812310000));
    CHECK(Datetime(201901050000).startOfWeek() == Datetime(201812310000));
    CHECK(Datetime(201901060000).startOfWeek() == Datetime(201812310000));
    CHECK(Datetime(201901090000).startOfWeek() == Datetime(201901070000));

    /** @arg Test endOfWeek */
    CHECK(null_datetime.endOfWeek() == null_datetime);
    CHECK(Datetime::min().endOfWeek() == Datetime(1400, 1, 5));
    CHECK(Datetime::max().endOfWeek() == Datetime::max());
    CHECK(Datetime(201812310000).endOfWeek() == Datetime(201901060000));
    CHECK(Datetime(201901050000).endOfWeek() == Datetime(201901060000));
    CHECK(Datetime(201901060000).endOfWeek() == Datetime(201901060000));
    CHECK(Datetime(201901090000).endOfWeek() == Datetime(201901130000));

    /** @arg Test startOfMonth */
    CHECK(null_datetime.startOfMonth() == null_datetime);
    CHECK(Datetime::min().startOfMonth() == Datetime::min());
    CHECK(Datetime::max().startOfMonth() == Datetime(9999, 12, 1));
    CHECK(Datetime(201901050000).startOfMonth() == Datetime(201901010000));
    CHECK(Datetime(201902050000).startOfMonth() == Datetime(201902010000));
    CHECK(Datetime(201902010000).startOfMonth() == Datetime(201902010000));
    CHECK(Datetime(201903030000).startOfMonth() == Datetime(201903010000));
    CHECK(Datetime(201904030000).startOfMonth() == Datetime(201904010000));
    CHECK(Datetime(201912030000).startOfMonth() == Datetime(201912010000));

    /** @arg Test endOfMonth */
    CHECK(null_datetime.endOfMonth() == null_datetime);
    CHECK(Datetime::max().endOfMonth() == Datetime::max());
    CHECK(Datetime::min().endOfMonth() == Datetime(1400, 1, 31));
    CHECK(Datetime(201801010000L).endOfMonth() == Datetime(201801310000L));
    CHECK(Datetime(201802010000L).endOfMonth() == Datetime(201802280000L));
    CHECK(Datetime(201803010000L).endOfMonth() == Datetime(201803310000L));
    CHECK(Datetime(201804010000L).endOfMonth() == Datetime(201804300000L));
    CHECK(Datetime(201805010000L).endOfMonth() == Datetime(201805310000L));
    CHECK(Datetime(201806010000L).endOfMonth() == Datetime(201806300000L));
    CHECK(Datetime(201807010000L).endOfMonth() == Datetime(201807310000L));
    CHECK(Datetime(201808010000L).endOfMonth() == Datetime(201808310000L));
    CHECK(Datetime(201809010000L).endOfMonth() == Datetime(201809300000L));
    CHECK(Datetime(201810010000L).endOfMonth() == Datetime(201810310000L));
    CHECK(Datetime(201811010000L).endOfMonth() == Datetime(201811300000L));
    CHECK(Datetime(201812010000L).endOfMonth() == Datetime(201812310000L));

    /** @arg Test startOfQuarter */
    CHECK(null_datetime.startOfQuarter() == null_datetime);
    CHECK(Datetime::min().startOfQuarter() == Datetime::min());
    CHECK(Datetime::max().startOfQuarter() == Datetime(9999, 10, 1));
    CHECK(Datetime(201902050000).startOfQuarter() == Datetime(201901010000));
    CHECK(Datetime(201901010000).startOfQuarter() == Datetime(201901010000));
    CHECK(Datetime(201903310000).startOfQuarter() == Datetime(201901010000));
    CHECK(Datetime(201904010000).startOfQuarter() == Datetime(201904010000));
    CHECK(Datetime(201905010000).startOfQuarter() == Datetime(201904010000));
    CHECK(Datetime(201906300000).startOfQuarter() == Datetime(201904010000));
    CHECK(Datetime(201907010000).startOfQuarter() == Datetime(201907010000));
    CHECK(Datetime(201907060000).startOfQuarter() == Datetime(201907010000));
    CHECK(Datetime(201909300000).startOfQuarter() == Datetime(201907010000));
    CHECK(Datetime(201910010000).startOfQuarter() == Datetime(201910010000));
    CHECK(Datetime(201911010000).startOfQuarter() == Datetime(201910010000));
    CHECK(Datetime(201912310000).startOfQuarter() == Datetime(201910010000));

    /** @arg Test endOfQuarter */
    CHECK(null_datetime.endOfQuarter() == null_datetime);
    CHECK(Datetime::min().endOfQuarter() == Datetime(1400, 3, 31));
    CHECK(Datetime::max().endOfQuarter() == Datetime::max());
    CHECK(Datetime(201902050000).endOfQuarter() == Datetime(201903310000));
    CHECK(Datetime(201901010000).endOfQuarter() == Datetime(201903310000));
    CHECK(Datetime(201903310000).endOfQuarter() == Datetime(201903310000));
    CHECK(Datetime(201904010000).endOfQuarter() == Datetime(201906300000));
    CHECK(Datetime(201905010000).endOfQuarter() == Datetime(201906300000));
    CHECK(Datetime(201906300000).endOfQuarter() == Datetime(201906300000));
    CHECK(Datetime(201907010000).endOfQuarter() == Datetime(201909300000));
    CHECK(Datetime(201907060000).endOfQuarter() == Datetime(201909300000));
    CHECK(Datetime(201909300000).endOfQuarter() == Datetime(201909300000));
    CHECK(Datetime(201910010000).endOfQuarter() == Datetime(201912310000));
    CHECK(Datetime(201911010000).endOfQuarter() == Datetime(201912310000));
    CHECK(Datetime(201912310000).endOfQuarter() == Datetime(201912310000));

    /** @arg Test startOfHalfyear */
    CHECK(null_datetime.startOfHalfyear() == null_datetime);
    CHECK(Datetime::min().startOfHalfyear() == Datetime::min());
    CHECK(Datetime::max().startOfHalfyear() == Datetime(9999, 7, 1));
    CHECK(Datetime(201812310000).startOfHalfyear() == Datetime(201807010000));
    CHECK(Datetime(201901010000).startOfHalfyear() == Datetime(201901010000));
    CHECK(Datetime(201901020000).startOfHalfyear() == Datetime(201901010000));
    CHECK(Datetime(201906050000).startOfHalfyear() == Datetime(201901010000));
    CHECK(Datetime(201906300000).startOfHalfyear() == Datetime(201901010000));
    CHECK(Datetime(201907010000).startOfHalfyear() == Datetime(201907010000));
    CHECK(Datetime(201907100000).startOfHalfyear() == Datetime(201907010000));
    CHECK(Datetime(201912010000).startOfHalfyear() == Datetime(201907010000));
    CHECK(Datetime(201912310000).startOfHalfyear() == Datetime(201907010000));

    /** @arg Test endOfHalfyear */
    CHECK(null_datetime.endOfHalfyear() == null_datetime);
    CHECK(Datetime::min().endOfHalfyear() == Datetime(1400, 6, 30));
    CHECK(Datetime::max().endOfHalfyear() == Datetime::max());
    CHECK(Datetime(201812310000).endOfHalfyear() == Datetime(201812310000));
    CHECK(Datetime(201901010000).endOfHalfyear() == Datetime(201906300000));
    CHECK(Datetime(201901020000).endOfHalfyear() == Datetime(201906300000));
    CHECK(Datetime(201906050000).endOfHalfyear() == Datetime(201906300000));
    CHECK(Datetime(201906300000).endOfHalfyear() == Datetime(201906300000));
    CHECK(Datetime(201907010000).endOfHalfyear() == Datetime(201912310000));
    CHECK(Datetime(201907100000).endOfHalfyear() == Datetime(201912310000));
    CHECK(Datetime(201912010000).endOfHalfyear() == Datetime(201912310000));
    CHECK(Datetime(201912310000).endOfHalfyear() == Datetime(201912310000));

    /** @arg Test startOfYear */
    CHECK(null_datetime.startOfYear() == null_datetime);
    CHECK(Datetime::min().startOfYear() == Datetime::min());
    CHECK(Datetime::max().startOfYear() == Datetime(9999, 1, 1));
    CHECK(Datetime(201901050000).startOfYear() == Datetime(201901010000));
    CHECK(Datetime(201911050000).startOfYear() == Datetime(201901010000));

    /** @arg Test endOfYear */
    CHECK(null_datetime.endOfYear() == null_datetime);
    CHECK(Datetime::min().endOfYear() == Datetime(1400, 12, 31));
    CHECK(Datetime::max().endOfYear() == Datetime::max());
    CHECK(Datetime(201902050000).endOfYear() == Datetime(201912310000));
    CHECK(Datetime(201912310000).endOfYear() == Datetime(201912310000));

    /** @arg Test nextDay */
    CHECK(null_datetime.nextDay() == null_datetime);
    CHECK(Datetime::max().nextDay() == Datetime::max());
    CHECK(Datetime(201802060000L).nextDay() == Datetime(201802070000L));
    CHECK(Datetime(201802280000L).nextDay() == Datetime(201803010000L));
    CHECK(Datetime(201602280000L).nextDay() == Datetime(201602290000L));

    /** @arg Test nextWeek */
    CHECK(null_datetime.nextWeek() == null_datetime);
    CHECK(Datetime::max().nextWeek() == Datetime::max());
    CHECK(Datetime(201812300000).nextWeek() == Datetime(201812310000));
    CHECK(Datetime(201812310000).nextWeek() == Datetime(201901070000));
    CHECK(Datetime(201901010000).nextWeek() == Datetime(201901070000));
    CHECK(Datetime(201901020000).nextWeek() == Datetime(201901070000));
    CHECK(Datetime(201901030000).nextWeek() == Datetime(201901070000));
    CHECK(Datetime(201901040000).nextWeek() == Datetime(201901070000));
    CHECK(Datetime(201901050000).nextWeek() == Datetime(201901070000));
    CHECK(Datetime(201901060000).nextWeek() == Datetime(201901070000));
    CHECK(Datetime(201901070000).nextWeek() == Datetime(201901140000));

    /** @arg Test nextMonth */
    CHECK(null_datetime.nextMonth() == null_datetime);
    CHECK(Datetime::max().nextMonth() == Datetime::max());
    CHECK(Datetime(201812310000).nextMonth() == Datetime(201901010000));
    CHECK(Datetime(201901010000).nextMonth() == Datetime(201902010000));
    CHECK(Datetime(201901310000).nextMonth() == Datetime(201902010000));

    /** @arg Test nextQuarter */
    CHECK(null_datetime.nextQuarter() == null_datetime);
    CHECK(Datetime::max().nextQuarter() == Datetime::max());
    CHECK(Datetime(201901010000).nextQuarter() == Datetime(201904010000));
    CHECK(Datetime(201902010000).nextQuarter() == Datetime(201904010000));
    CHECK(Datetime(201903310000).nextQuarter() == Datetime(201904010000));
    CHECK(Datetime(201904010000).nextQuarter() == Datetime(201907010000));
    CHECK(Datetime(201905010000).nextQuarter() == Datetime(201907010000));
    CHECK(Datetime(201906300000).nextQuarter() == Datetime(201907010000));
    CHECK(Datetime(201907010000).nextQuarter() == Datetime(201910010000));
    CHECK(Datetime(201907020000).nextQuarter() == Datetime(201910010000));
    CHECK(Datetime(201908010000).nextQuarter() == Datetime(201910010000));
    CHECK(Datetime(201909300000).nextQuarter() == Datetime(201910010000));
    CHECK(Datetime(201910010000).nextQuarter() == Datetime(202001010000));
    CHECK(Datetime(201911010000).nextQuarter() == Datetime(202001010000));
    CHECK(Datetime(201912310000).nextQuarter() == Datetime(202001010000));

    /** @arg Test nextHalfyear */
    CHECK(null_datetime.nextHalfyear() == null_datetime);
    CHECK(Datetime::max().nextHalfyear() == Datetime::max());
    CHECK(Datetime(201812310000).nextHalfyear() == Datetime(201901010000));
    CHECK(Datetime(201901010000).nextHalfyear() == Datetime(201907010000));
    CHECK(Datetime(201905050000).nextHalfyear() == Datetime(201907010000));
    CHECK(Datetime(201906300000).nextHalfyear() == Datetime(201907010000));
    CHECK(Datetime(201907010000).nextHalfyear() == Datetime(202001010000));
    CHECK(Datetime(201910110000).nextHalfyear() == Datetime(202001010000));
    CHECK(Datetime(201912310000).nextHalfyear() == Datetime(202001010000));

    /** @arg Test nextYear */
    CHECK(null_datetime.nextYear() == null_datetime);
    CHECK(Datetime::max().nextYear() == Datetime::max());
    CHECK(Datetime(201812310000).nextYear() == Datetime(201901010000));
    CHECK(Datetime(201901010000).nextYear() == Datetime(202001010000));
    CHECK(Datetime(201901020000).nextYear() == Datetime(202001010000));
    CHECK(Datetime(201907310000).nextYear() == Datetime(202001010000));
    CHECK(Datetime(201912310000).nextYear() == Datetime(202001010000));

    /** @arg Test preDay */
    CHECK(null_datetime.preDay() == null_datetime);
    CHECK(Datetime::min().preDay() == Datetime::min());
    CHECK(Datetime(201812310000).preDay() == Datetime(201812300000));
    CHECK(Datetime(201901010000).preDay() == Datetime(201812310000));
    CHECK(Datetime(201901020000).preDay() == Datetime(201901010000));

    /** @arg Test preWeek */
    CHECK(null_datetime.preWeek() == null_datetime);
    CHECK(Datetime::min().preWeek() == Datetime::min());
    CHECK(Datetime(201812310000).preWeek() == Datetime(201812240000));
    CHECK(Datetime(201901010000).preWeek() == Datetime(201812240000));
    CHECK(Datetime(201901020000).preWeek() == Datetime(201812240000));
    CHECK(Datetime(201901030000).preWeek() == Datetime(201812240000));
    CHECK(Datetime(201901040000).preWeek() == Datetime(201812240000));
    CHECK(Datetime(201901050000).preWeek() == Datetime(201812240000));
    CHECK(Datetime(201901060000).preWeek() == Datetime(201812240000));
    CHECK(Datetime(201901070000).preWeek() == Datetime(201812310000));

    /** @arg Test preMonth */
    CHECK(null_datetime.preMonth() == null_datetime);
    CHECK(Datetime::min().preMonth() == Datetime::min());
    CHECK(Datetime(201602290000).preMonth() == Datetime(201601010000));
    CHECK(Datetime(201901010000).preMonth() == Datetime(201812010000));
    CHECK(Datetime(201901020000).preMonth() == Datetime(201812010000));
    CHECK(Datetime(201901310000).preMonth() == Datetime(201812010000));
    CHECK(Datetime(201902010000).preMonth() == Datetime(201901010000));
    CHECK(Datetime(201902280000).preMonth() == Datetime(201901010000));
    CHECK(Datetime(201903010000).preMonth() == Datetime(201902010000));
    CHECK(Datetime(201903310000).preMonth() == Datetime(201902010000));
    CHECK(Datetime(201904010000).preMonth() == Datetime(201903010000));
    CHECK(Datetime(201904300000).preMonth() == Datetime(201903010000));
    CHECK(Datetime(201905300000).preMonth() == Datetime(201904010000));
    CHECK(Datetime(201912310000).preMonth() == Datetime(201911010000));
    CHECK(Datetime(201912300000).preMonth() == Datetime(201911010000));

    /** @arg Test preQuarter */
    CHECK(null_datetime.preQuarter() == null_datetime);
    CHECK(Datetime::min().preQuarter() == Datetime::min());
    CHECK(Datetime(201901010000).preQuarter() == Datetime(201810010000));
    CHECK(Datetime(201903310000).preQuarter() == Datetime(201810010000));
    CHECK(Datetime(201904010000).preQuarter() == Datetime(201901010000));
    CHECK(Datetime(201906300000).preQuarter() == Datetime(201901010000));
    CHECK(Datetime(201907010000).preQuarter() == Datetime(201904010000));
    CHECK(Datetime(201909300000).preQuarter() == Datetime(201904010000));
    CHECK(Datetime(201910010000).preQuarter() == Datetime(201907010000));
    CHECK(Datetime(201912310000).preQuarter() == Datetime(201907010000));

    /** @arg Test preHalfyear() */
    CHECK(null_datetime.preHalfyear() == null_datetime);
    CHECK(Datetime::min().preHalfyear() == Datetime::min());
    CHECK(Datetime(201901010000).preHalfyear() == Datetime(201807010000));
    CHECK(Datetime(201906300000).preHalfyear() == Datetime(201807010000));
    CHECK(Datetime(201907010000).preHalfyear() == Datetime(201901010000));
    CHECK(Datetime(201912310000).preHalfyear() == Datetime(201901010000));
    CHECK(Datetime(202001010000).preHalfyear() == Datetime(201907010000));

    /** @arg Test preYear */
    CHECK(null_datetime.preYear() == null_datetime);
    CHECK(Datetime::min().preYear() == Datetime::min());
    CHECK(Datetime(201901010000).preYear() == Datetime(201801010000));
    CHECK(Datetime(201912310000).preYear() == Datetime(201801010000));
    CHECK(Datetime(201601010000).preYear() == Datetime(201501010000));
    CHECK(Datetime(201612310000).preYear() == Datetime(201501010000));

    /** @arg Test ticks */
    CHECK(null_datetime.ticks() == Null<uint64_t>());
    CHECK_EQ(Datetime::min().ticks(), 0ULL);
    CHECK_EQ(Datetime::max().ticks(), 271389657600000000ULL);
    CHECK_EQ(Datetime::max().ticks(), (Datetime::max() - Datetime::min()).ticks());
}

/** @par Test points */
TEST_CASE("test_Datetime_related_operator") {
    /** @arg The less-than comparison */
    CHECK(Datetime(200101010000) < Null<Datetime>());
    CHECK(Datetime(200101010000) < Datetime::max());
    CHECK(Datetime::min() < Datetime(200101010000));
    CHECK(Datetime(200101010000) < Datetime(200101020000));
}

/** @} */
