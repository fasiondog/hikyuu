/*
 * test_datetime.cpp
 *
 *  Created on: 2012-8-25
 *      Author: fasiondog
 */

#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_datetime
    #include <boost/test/unit_test.hpp>
#endif

#include <hikyuu/datetime/Datetime.h>
#include <hikyuu/utilities/Null.h>

using namespace hku;

/**
 * @defgroup test_hikyuu_Datetime test_hikyuu_Datetime
 * @ingroup test_hikyuu_datetime_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_Datetime ) {
    Datetime null_datetime = Null<Datetime>();

    /** @arg 默认无参构造函数返回Null<Datetime>() */
    Datetime d;
    BOOST_CHECK(d == Null<Datetime>());
    BOOST_CHECK(Null<Datetime>() == d);
    d = Datetime(201208062359);
    BOOST_CHECK(d < Null<Datetime>());

    /** @arg 从Null<unsigned long long>()构造 */
    d = Datetime(Null<unsigned long long>());
    BOOST_CHECK(d == Null<Datetime>());

    /** @arg 从bd::date 隐式构造 */
    d = Datetime(200101010110);
    d = d.date();
    BOOST_CHECK(Datetime(200101010000) == d);

    /** @arg 非法年份 */
    BOOST_CHECK_THROW(Datetime(99999,1,1), std::out_of_range);
    BOOST_CHECK_THROW(Datetime(000001010000L), std::out_of_range);

    /** @arg 非法月份 */
    BOOST_CHECK_THROW(Datetime(2010,13,1), std::out_of_range);
    BOOST_CHECK_THROW(Datetime(2010,0,1), std::out_of_range);

    BOOST_CHECK_THROW(Datetime(201013010000L), std::out_of_range);
    BOOST_CHECK_THROW(Datetime(201000010000L), std::out_of_range);

    /** @arg 非法日期 */
    BOOST_CHECK_THROW(Datetime(2010,1,0), std::out_of_range);
    BOOST_CHECK_THROW(Datetime(2010,1,32), std::out_of_range);

    BOOST_CHECK_THROW(Datetime(201001000000L), std::out_of_range);
    BOOST_CHECK_THROW(Datetime(201001320000L), std::out_of_range);

    /** @arg 非法小时 */
    BOOST_CHECK_THROW(Datetime(201001012400L), std::out_of_range);
    BOOST_CHECK_THROW(Datetime(201001012500L), std::out_of_range);

    /** @arg 非法分钟 */
    BOOST_CHECK_THROW(Datetime(201001010060L), std::out_of_range);
    BOOST_CHECK_THROW(Datetime(201001010061L), std::out_of_range);

    /** @arg 属性读取 */
    d = Datetime(201208062359);
    BOOST_CHECK(2012 == d.year());
    BOOST_CHECK(8 == d.month());
    BOOST_CHECK(6 == d.day());
    BOOST_CHECK(23 == d.hour());
    BOOST_CHECK(59 == d.minute());

    /** @arg 正常日期转化为unsigned long long */
    unsigned long long x = d.number();
    BOOST_CHECK(x == 201208062359);

    long long y = x;
    Datetime m(y);
    BOOST_CHECK(m == d);

    /** @arg Null<Datetime>()转化为number */
    x = Null<unsigned long long>();
    d = Null<Datetime>();
    BOOST_CHECK(x == d.number());

    /** @arg 测试 toString */
    d = Datetime("2001-Jan-01 06:30:00");
    BOOST_CHECK("2001-Jan-01 06:30:00" == d.toString());

    /** @arg 测试 dayOfWeek*/
    BOOST_CHECK(Datetime(201801010000L).dayOfWeek() == 1);
    BOOST_CHECK(Datetime(201801060000L).dayOfWeek() == 6);
    BOOST_CHECK(Datetime(201801070000L).dayOfWeek() == 0);

    /** @arg 测试 dayOfYear*/
    BOOST_CHECK(Datetime(201801010000L).dayOfYear() == 1);
    BOOST_CHECK(Datetime(201812310000L).dayOfYear() == 365);

    /** @arg 测试 dateOfWeek */
    BOOST_CHECK(null_datetime.dateOfWeek(-1) == null_datetime);
    BOOST_CHECK(null_datetime.dateOfWeek(0) == null_datetime);
    BOOST_CHECK(null_datetime.dateOfWeek(4) == null_datetime);
    BOOST_CHECK(null_datetime.dateOfWeek(6) == null_datetime);
    BOOST_CHECK(null_datetime.dateOfWeek(7) == null_datetime);
    BOOST_CHECK(Datetime(201901010000).dateOfWeek(-1) == Datetime(201812300000));
    BOOST_CHECK(Datetime(201901010000).dateOfWeek(0) == Datetime(201812300000));
    BOOST_CHECK(Datetime(201901010000).dateOfWeek(1) == Datetime(201812310000));
    BOOST_CHECK(Datetime(201901010000).dateOfWeek(2) == Datetime(201901010000));
    BOOST_CHECK(Datetime(201901010000).dateOfWeek(3) == Datetime(201901020000));
    BOOST_CHECK(Datetime(201901010000).dateOfWeek(4) == Datetime(201901030000));
    BOOST_CHECK(Datetime(201901010000).dateOfWeek(5) == Datetime(201901040000));
    BOOST_CHECK(Datetime(201901010000).dateOfWeek(6) == Datetime(201901050000));
    BOOST_CHECK(Datetime(201901010000).dateOfWeek(7) == Datetime(201901050000));

    /** @arg 测试 startOfDay */
    BOOST_CHECK(null_datetime.startOfDay() == null_datetime);
    BOOST_CHECK(Datetime::min().startOfDay() == Datetime::min());
    BOOST_CHECK(Datetime::max().startOfDay() == Datetime::max());
    BOOST_CHECK(Datetime(201812310110).startOfDay() == Datetime(201812310000));

    /** @arg 测试 endOfDay */
    BOOST_CHECK(null_datetime.endOfDay() == null_datetime);
    BOOST_CHECK(Datetime::min().endOfDay() == Datetime(1400,1,1,23,59,59));
    BOOST_CHECK(Datetime::max().endOfDay() == Datetime::max());
    BOOST_CHECK(Datetime(201812310110).endOfDay() == Datetime(2018,12,31,23,59,59));

    /** @arg 测试 startOfWeek */
    BOOST_CHECK(null_datetime.startOfWeek() == null_datetime);
    BOOST_CHECK(Datetime::min().startOfWeek() == Datetime::min());
    BOOST_CHECK(Datetime::max().startOfWeek() == Datetime(9999, 12, 27));
    BOOST_CHECK(Datetime(201812310000).startOfWeek() == Datetime(201812310000));
    BOOST_CHECK(Datetime(201901050000).startOfWeek() == Datetime(201812310000));
    BOOST_CHECK(Datetime(201901060000).startOfWeek() == Datetime(201812310000));
    BOOST_CHECK(Datetime(201901090000).startOfWeek() == Datetime(201901070000));

    /** @arg 测试 endOfWeek */
    BOOST_CHECK(null_datetime.endOfWeek() == null_datetime);
    BOOST_CHECK(Datetime::min().endOfWeek() == Datetime(1400, 1, 5));
    BOOST_CHECK(Datetime::max().endOfWeek() == Datetime::max());
    BOOST_CHECK(Datetime(201812310000).endOfWeek() == Datetime(201901060000));
    BOOST_CHECK(Datetime(201901050000).endOfWeek() == Datetime(201901060000));
    BOOST_CHECK(Datetime(201901060000).endOfWeek() == Datetime(201901060000));
    BOOST_CHECK(Datetime(201901090000).endOfWeek() == Datetime(201901130000));
    
    /** @arg 测试 startOfMonth */
    BOOST_CHECK(null_datetime.startOfMonth() == null_datetime);
    BOOST_CHECK(Datetime::min().startOfMonth() == Datetime::min());
    BOOST_CHECK(Datetime::max().startOfMonth() == Datetime(9999, 12, 1));
    BOOST_CHECK(Datetime(201901050000).startOfMonth() == Datetime(201901010000));
    BOOST_CHECK(Datetime(201902050000).startOfMonth() == Datetime(201902010000));
    BOOST_CHECK(Datetime(201902010000).startOfMonth() == Datetime(201902010000));
    BOOST_CHECK(Datetime(201903030000).startOfMonth() == Datetime(201903010000));
    BOOST_CHECK(Datetime(201904030000).startOfMonth() == Datetime(201904010000));
    BOOST_CHECK(Datetime(201912030000).startOfMonth() == Datetime(201912010000));

    /** @arg 测试 endOfMonth*/
    BOOST_CHECK(null_datetime.endOfMonth() == null_datetime);
    BOOST_CHECK(Datetime::max().endOfMonth() == Datetime::max());
    BOOST_CHECK(Datetime::min().endOfMonth() == Datetime(1400, 1, 31));
    BOOST_CHECK(Datetime(201801010000L).endOfMonth() == Datetime(201801310000L));
    BOOST_CHECK(Datetime(201802010000L).endOfMonth() == Datetime(201802280000L));
    BOOST_CHECK(Datetime(201803010000L).endOfMonth() == Datetime(201803310000L));
    BOOST_CHECK(Datetime(201804010000L).endOfMonth() == Datetime(201804300000L));
    BOOST_CHECK(Datetime(201805010000L).endOfMonth() == Datetime(201805310000L));
    BOOST_CHECK(Datetime(201806010000L).endOfMonth() == Datetime(201806300000L));
    BOOST_CHECK(Datetime(201807010000L).endOfMonth() == Datetime(201807310000L));
    BOOST_CHECK(Datetime(201808010000L).endOfMonth() == Datetime(201808310000L));
    BOOST_CHECK(Datetime(201809010000L).endOfMonth() == Datetime(201809300000L));
    BOOST_CHECK(Datetime(201810010000L).endOfMonth() == Datetime(201810310000L));
    BOOST_CHECK(Datetime(201811010000L).endOfMonth() == Datetime(201811300000L));
    BOOST_CHECK(Datetime(201812010000L).endOfMonth() == Datetime(201812310000L));

    /** @arg 测试 startOfQuarter */
    BOOST_CHECK(null_datetime.startOfQuarter() == null_datetime);
    BOOST_CHECK(Datetime::min().startOfQuarter() == Datetime::min());
    BOOST_CHECK(Datetime::max().startOfQuarter() == Datetime(9999, 10, 1));
    BOOST_CHECK(Datetime(201902050000).startOfQuarter() == Datetime(201901010000));
    BOOST_CHECK(Datetime(201901010000).startOfQuarter() == Datetime(201901010000));
    BOOST_CHECK(Datetime(201903310000).startOfQuarter() == Datetime(201901010000));
    BOOST_CHECK(Datetime(201904010000).startOfQuarter() == Datetime(201904010000));
    BOOST_CHECK(Datetime(201905010000).startOfQuarter() == Datetime(201904010000));
    BOOST_CHECK(Datetime(201906300000).startOfQuarter() == Datetime(201904010000));
    BOOST_CHECK(Datetime(201907010000).startOfQuarter() == Datetime(201907010000));
    BOOST_CHECK(Datetime(201907060000).startOfQuarter() == Datetime(201907010000));
    BOOST_CHECK(Datetime(201909300000).startOfQuarter() == Datetime(201907010000));
    BOOST_CHECK(Datetime(201910010000).startOfQuarter() == Datetime(201910010000));
    BOOST_CHECK(Datetime(201911010000).startOfQuarter() == Datetime(201910010000));
    BOOST_CHECK(Datetime(201912310000).startOfQuarter() == Datetime(201910010000));

    /** @arg 测试 endOfQuarter */
    BOOST_CHECK(null_datetime.endOfQuarter() == null_datetime);
    BOOST_CHECK(Datetime::min().endOfQuarter() == Datetime(1400, 3, 31));
    BOOST_CHECK(Datetime::max().endOfQuarter() == Datetime::max());
    BOOST_CHECK(Datetime(201902050000).endOfQuarter() == Datetime(201903310000));
    BOOST_CHECK(Datetime(201901010000).endOfQuarter() == Datetime(201903310000));
    BOOST_CHECK(Datetime(201903310000).endOfQuarter() == Datetime(201903310000));
    BOOST_CHECK(Datetime(201904010000).endOfQuarter() == Datetime(201906300000));
    BOOST_CHECK(Datetime(201905010000).endOfQuarter() == Datetime(201906300000));
    BOOST_CHECK(Datetime(201906300000).endOfQuarter() == Datetime(201906300000));
    BOOST_CHECK(Datetime(201907010000).endOfQuarter() == Datetime(201909300000));
    BOOST_CHECK(Datetime(201907060000).endOfQuarter() == Datetime(201909300000));
    BOOST_CHECK(Datetime(201909300000).endOfQuarter() == Datetime(201909300000));
    BOOST_CHECK(Datetime(201910010000).endOfQuarter() == Datetime(201912310000));
    BOOST_CHECK(Datetime(201911010000).endOfQuarter() == Datetime(201912310000));
    BOOST_CHECK(Datetime(201912310000).endOfQuarter() == Datetime(201912310000));

    /** @arg 测试 startOfHalfyear */
    BOOST_CHECK(null_datetime.startOfHalfyear() == null_datetime);
    BOOST_CHECK(Datetime::min().startOfHalfyear() == Datetime::min());
    BOOST_CHECK(Datetime::max().startOfHalfyear() == Datetime(9999, 7, 1));
    BOOST_CHECK(Datetime(201812310000).startOfHalfyear() == Datetime(201807010000));
    BOOST_CHECK(Datetime(201901010000).startOfHalfyear() == Datetime(201901010000));
    BOOST_CHECK(Datetime(201901020000).startOfHalfyear() == Datetime(201901010000));
    BOOST_CHECK(Datetime(201906050000).startOfHalfyear() == Datetime(201901010000));
    BOOST_CHECK(Datetime(201906300000).startOfHalfyear() == Datetime(201901010000));
    BOOST_CHECK(Datetime(201907010000).startOfHalfyear() == Datetime(201907010000));
    BOOST_CHECK(Datetime(201907100000).startOfHalfyear() == Datetime(201907010000));
    BOOST_CHECK(Datetime(201912010000).startOfHalfyear() == Datetime(201907010000));
    BOOST_CHECK(Datetime(201912310000).startOfHalfyear() == Datetime(201907010000));

    /** @arg 测试 endOfHalfyear */
    BOOST_CHECK(null_datetime.endOfHalfyear() == null_datetime);
    BOOST_CHECK(Datetime::min().endOfHalfyear() == Datetime(1400, 6, 30));
    BOOST_CHECK(Datetime::max().endOfHalfyear() == Datetime::max());
    BOOST_CHECK(Datetime(201812310000).endOfHalfyear() == Datetime(201812310000));
    BOOST_CHECK(Datetime(201901010000).endOfHalfyear() == Datetime(201906300000));
    BOOST_CHECK(Datetime(201901020000).endOfHalfyear() == Datetime(201906300000));
    BOOST_CHECK(Datetime(201906050000).endOfHalfyear() == Datetime(201906300000));
    BOOST_CHECK(Datetime(201906300000).endOfHalfyear() == Datetime(201906300000));
    BOOST_CHECK(Datetime(201907010000).endOfHalfyear() == Datetime(201912310000));
    BOOST_CHECK(Datetime(201907100000).endOfHalfyear() == Datetime(201912310000));
    BOOST_CHECK(Datetime(201912010000).endOfHalfyear() == Datetime(201912310000));
    BOOST_CHECK(Datetime(201912310000).endOfHalfyear() == Datetime(201912310000));

    /** @arg 测试 startOfYear */
    BOOST_CHECK(null_datetime.startOfYear() == null_datetime);
    BOOST_CHECK(Datetime::min().startOfYear() == Datetime::min());
    BOOST_CHECK(Datetime::max().startOfYear() == Datetime(9999, 1, 1));
    BOOST_CHECK(Datetime(201901050000).startOfYear() == Datetime(201901010000));
    BOOST_CHECK(Datetime(201911050000).startOfYear() == Datetime(201901010000));
    
    /** @arg 测试 endOfYear */
    BOOST_CHECK(null_datetime.endOfYear() == null_datetime);
    BOOST_CHECK(Datetime::min().endOfYear() == Datetime(1400, 12, 31));
    BOOST_CHECK(Datetime::max().endOfYear() == Datetime::max());
    BOOST_CHECK(Datetime(201902050000).endOfYear() == Datetime(201912310000));
    BOOST_CHECK(Datetime(201912310000).endOfYear() == Datetime(201912310000));

    /** @arg 测试 nextDay */
    BOOST_CHECK(null_datetime.nextDay() == null_datetime);
    BOOST_CHECK(Datetime::max().nextDay() == Datetime::max());
    BOOST_CHECK(Datetime(201802060000L).nextDay() == Datetime(201802070000L));
    BOOST_CHECK(Datetime(201802280000L).nextDay() == Datetime(201803010000L));
    BOOST_CHECK(Datetime(201602280000L).nextDay() == Datetime(201602290000L));

    /** @arg 测试 nextWeek */
    BOOST_CHECK(null_datetime.nextWeek() == null_datetime);
    BOOST_CHECK(Datetime::max().nextWeek() == Datetime::max());
    BOOST_CHECK(Datetime(201812300000).nextWeek() == Datetime(201812310000));
    BOOST_CHECK(Datetime(201812310000).nextWeek() == Datetime(201901070000));
    BOOST_CHECK(Datetime(201901010000).nextWeek() == Datetime(201901070000));
    BOOST_CHECK(Datetime(201901020000).nextWeek() == Datetime(201901070000));
    BOOST_CHECK(Datetime(201901030000).nextWeek() == Datetime(201901070000));
    BOOST_CHECK(Datetime(201901040000).nextWeek() == Datetime(201901070000));
    BOOST_CHECK(Datetime(201901050000).nextWeek() == Datetime(201901070000));
    BOOST_CHECK(Datetime(201901060000).nextWeek() == Datetime(201901070000));
    BOOST_CHECK(Datetime(201901070000).nextWeek() == Datetime(201901140000));

    /** @arg 测试 nextMonth */
    BOOST_CHECK(null_datetime.nextMonth() == null_datetime);
    BOOST_CHECK(Datetime::max().nextMonth() == Datetime::max());
    BOOST_CHECK(Datetime(201812310000).nextMonth() == Datetime(201901010000));
    BOOST_CHECK(Datetime(201901010000).nextMonth() == Datetime(201902010000));
    BOOST_CHECK(Datetime(201901310000).nextMonth() == Datetime(201902010000));

    /** @arg 测试 nextQuarter */
    BOOST_CHECK(null_datetime.nextQuarter() == null_datetime);
    BOOST_CHECK(Datetime::max().nextQuarter() == Datetime::max());
    BOOST_CHECK(Datetime(201901010000).nextQuarter() == Datetime(201904010000));
    BOOST_CHECK(Datetime(201902010000).nextQuarter() == Datetime(201904010000));
    BOOST_CHECK(Datetime(201903310000).nextQuarter() == Datetime(201904010000));
    BOOST_CHECK(Datetime(201904010000).nextQuarter() == Datetime(201907010000));
    BOOST_CHECK(Datetime(201905010000).nextQuarter() == Datetime(201907010000));
    BOOST_CHECK(Datetime(201906300000).nextQuarter() == Datetime(201907010000));
    BOOST_CHECK(Datetime(201907010000).nextQuarter() == Datetime(201910010000));
    BOOST_CHECK(Datetime(201907020000).nextQuarter() == Datetime(201910010000));
    BOOST_CHECK(Datetime(201908010000).nextQuarter() == Datetime(201910010000));
    BOOST_CHECK(Datetime(201909300000).nextQuarter() == Datetime(201910010000));
    BOOST_CHECK(Datetime(201910010000).nextQuarter() == Datetime(202001010000));
    BOOST_CHECK(Datetime(201911010000).nextQuarter() == Datetime(202001010000));
    BOOST_CHECK(Datetime(201912310000).nextQuarter() == Datetime(202001010000));

    /** @arg 测试 nextHalfyear */
    BOOST_CHECK(null_datetime.nextHalfyear() == null_datetime);
    BOOST_CHECK(Datetime::max().nextHalfyear() == Datetime::max());
    BOOST_CHECK(Datetime(201812310000).nextHalfyear() == Datetime(201901010000));
    BOOST_CHECK(Datetime(201901010000).nextHalfyear() == Datetime(201907010000));
    BOOST_CHECK(Datetime(201905050000).nextHalfyear() == Datetime(201907010000));
    BOOST_CHECK(Datetime(201906300000).nextHalfyear() == Datetime(201907010000));
    BOOST_CHECK(Datetime(201907010000).nextHalfyear() == Datetime(202001010000));
    BOOST_CHECK(Datetime(201910110000).nextHalfyear() == Datetime(202001010000));
    BOOST_CHECK(Datetime(201912310000).nextHalfyear() == Datetime(202001010000));

    /** @arg 测试 nextYear */
    BOOST_CHECK(null_datetime.nextYear() == null_datetime);
    BOOST_CHECK(Datetime::max().nextYear() == Datetime::max());
    BOOST_CHECK(Datetime(201812310000).nextYear() == Datetime(201901010000));
    BOOST_CHECK(Datetime(201901010000).nextYear() == Datetime(202001010000));
    BOOST_CHECK(Datetime(201901020000).nextYear() == Datetime(202001010000));
    BOOST_CHECK(Datetime(201907310000).nextYear() == Datetime(202001010000));
    BOOST_CHECK(Datetime(201912310000).nextYear() == Datetime(202001010000));

    /** @arg 测试 preDay */
    BOOST_CHECK(null_datetime.preDay() == null_datetime);
    BOOST_CHECK(Datetime::min().preDay() == Datetime::min());
    BOOST_CHECK(Datetime(201812310000).preDay() == Datetime(201812300000));
    BOOST_CHECK(Datetime(201901010000).preDay() == Datetime(201812310000));
    BOOST_CHECK(Datetime(201901020000).preDay() == Datetime(201901010000));

    /** @arg 测试 preWeek */
    BOOST_CHECK(null_datetime.preWeek() == null_datetime);
    BOOST_CHECK(Datetime::min().preWeek() == Datetime::min());
    BOOST_CHECK(Datetime(201812310000).preWeek() == Datetime(201812240000));
    BOOST_CHECK(Datetime(201901010000).preWeek() == Datetime(201812240000));
    BOOST_CHECK(Datetime(201901020000).preWeek() == Datetime(201812240000));
    BOOST_CHECK(Datetime(201901030000).preWeek() == Datetime(201812240000));
    BOOST_CHECK(Datetime(201901040000).preWeek() == Datetime(201812240000));
    BOOST_CHECK(Datetime(201901050000).preWeek() == Datetime(201812240000));
    BOOST_CHECK(Datetime(201901060000).preWeek() == Datetime(201812240000));
    BOOST_CHECK(Datetime(201901070000).preWeek() == Datetime(201812310000));

    /** @arg 测试 preMonth */
    BOOST_CHECK(null_datetime.preMonth() == null_datetime);
    BOOST_CHECK(Datetime::min().preMonth() == Datetime::min());
    BOOST_CHECK(Datetime(201602290000).preMonth() == Datetime(201601010000));
    BOOST_CHECK(Datetime(201901010000).preMonth() == Datetime(201812010000));
    BOOST_CHECK(Datetime(201901020000).preMonth() == Datetime(201812010000));
    BOOST_CHECK(Datetime(201901310000).preMonth() == Datetime(201812010000));
    BOOST_CHECK(Datetime(201902010000).preMonth() == Datetime(201901010000));
    BOOST_CHECK(Datetime(201902280000).preMonth() == Datetime(201901010000));
    BOOST_CHECK(Datetime(201903010000).preMonth() == Datetime(201902010000));
    BOOST_CHECK(Datetime(201903310000).preMonth() == Datetime(201902010000));
    BOOST_CHECK(Datetime(201904010000).preMonth() == Datetime(201903010000));
    BOOST_CHECK(Datetime(201904300000).preMonth() == Datetime(201903010000));
    BOOST_CHECK(Datetime(201905300000).preMonth() == Datetime(201904010000));
    BOOST_CHECK(Datetime(201912310000).preMonth() == Datetime(201911010000));
    BOOST_CHECK(Datetime(201912300000).preMonth() == Datetime(201911010000));

    /** @arg 测试 preQuarter */
    BOOST_CHECK(null_datetime.preQuarter() == null_datetime);
    BOOST_CHECK(Datetime::min().preQuarter() == Datetime::min());
    BOOST_CHECK(Datetime(201901010000).preQuarter() == Datetime(201810010000));
    BOOST_CHECK(Datetime(201903310000).preQuarter() == Datetime(201810010000));
    BOOST_CHECK(Datetime(201904010000).preQuarter() == Datetime(201901010000));
    BOOST_CHECK(Datetime(201906300000).preQuarter() == Datetime(201901010000));
    BOOST_CHECK(Datetime(201907010000).preQuarter() == Datetime(201904010000));
    BOOST_CHECK(Datetime(201909300000).preQuarter() == Datetime(201904010000));
    BOOST_CHECK(Datetime(201910010000).preQuarter() == Datetime(201907010000));
    BOOST_CHECK(Datetime(201912310000).preQuarter() == Datetime(201907010000));

    /** @arg 测试 preHalfyear() */
    BOOST_CHECK(null_datetime.preHalfyear() == null_datetime);
    BOOST_CHECK(Datetime::min().preHalfyear() == Datetime::min());
    BOOST_CHECK(Datetime(201901010000).preHalfyear() == Datetime(201807010000));
    BOOST_CHECK(Datetime(201906300000).preHalfyear() == Datetime(201807010000));
    BOOST_CHECK(Datetime(201907010000).preHalfyear() == Datetime(201901010000));
    BOOST_CHECK(Datetime(201912310000).preHalfyear() == Datetime(201901010000));
    BOOST_CHECK(Datetime(202001010000).preHalfyear() == Datetime(201907010000));

    /** @arg 测试 preYear */
    BOOST_CHECK(null_datetime.preYear() == null_datetime);
    BOOST_CHECK(Datetime::min().preYear() == Datetime::min());
    BOOST_CHECK(Datetime(201901010000).preYear() == Datetime(201801010000));
    BOOST_CHECK(Datetime(201912310000).preYear() == Datetime(201801010000));
    BOOST_CHECK(Datetime(201601010000).preYear() == Datetime(201501010000));
    BOOST_CHECK(Datetime(201612310000).preYear() == Datetime(201501010000));
}

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_Datetime_related_operator ) {
    /** @arg 小于比较 */
    BOOST_CHECK(Datetime(200101010000) < Null<Datetime>());
    BOOST_CHECK(Datetime(200101010000) < Datetime::max());
    BOOST_CHECK(Datetime::min() < Datetime(200101010000));
    BOOST_CHECK(Datetime(200101010000) < Datetime(200101020000));

}

/** @} */


