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

    /** @arg 测试 endOfMonth*/
    BOOST_CHECK(Datetime(201801010000L).endOfMonth() == Datetime(201801310000L));

    /** @arg 测试nextDay */
    BOOST_CHECK(Datetime(201802060000L).nextDay() == Datetime(201802070000L));
    BOOST_CHECK(Datetime(201802280000L).nextDay() == Datetime(201803010000L));
    BOOST_CHECK(Datetime(201602280000L).nextDay() == Datetime(201602290000L));
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


