/*
 * test_iniparser.cpp
 *
 *  Created on: 2019-12-13
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/datetime/TimeDelta.h>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <fstream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <hikyuu/serialization/TimeDelta_serialization.h>

using namespace hku;

namespace bt = boost::posix_time;
namespace bd = boost::gregorian;

/**
 * @defgroup test_hikyuu_TimeDelta test_hikyuu_TimeDelta
 * @ingroup test_hikyuu_datetime_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_TimeDelta") {
    /** @arg days  超出限定值 */
    CHECK_THROWS(TimeDelta(99999999LL + 1));
    CHECK_THROWS(TimeDelta(-99999999LL - 1));

#if !HKU_DISABLE_ASSERT
    /** @arg hours 超出限定值 */
    CHECK_THROWS(TimeDelta(0, 100001));
    CHECK_THROWS(TimeDelta(0, -100001));

    /** @arg minutes 超出限定值 */
    CHECK_THROWS(TimeDelta(0, 0, 100001));
    CHECK_THROWS(TimeDelta(0, 0, -100001));

    /** @arg seconds 超出限定值 */
    CHECK_THROWS(TimeDelta(0, 0, 0, 8640000));
    CHECK_THROWS(TimeDelta(0, 0, 0, -8640000));

    /** @arg milliseconds 超出限定值 */
    CHECK_THROWS(TimeDelta(0, 0, 0, 0, 8640000000000));
    CHECK_THROWS(TimeDelta(0, 0, 0, 0, -8640000000000));

    /** @arg microseconds 超出限定值 */
    CHECK_THROWS(TimeDelta(0, 0, 0, 0, 0, 8640000000000));
    CHECK_THROWS(TimeDelta(0, 0, 0, 0, 0, -8640000000000));
#endif

    /** @arg microseconds总值超出限定值 */
    CHECK_THROWS(TimeDelta(99999999LL, 23, 59, 60, 999, 999));

    /** @arg 正常初始化，时分秒毫秒微秒都在各自的进制范围内 */
    TimeDelta td(7, 10, 20, 3, 5, 7);
    CHECK(td.days() == 7);
    CHECK(td.hours() == 10);
    CHECK(td.minutes() == 20);
    CHECK(td.seconds() == 3);
    CHECK(td.milliseconds() == 5);
    CHECK(td.microseconds() == 7);
    CHECK(td.isNegative() == false);

    /** @arg 正常初始化，时大于23 */
    td = TimeDelta(0, 24);
    CHECK(td.days() == 1);
    CHECK(td.hours() == 0);
    CHECK(td.minutes() == 0);
    CHECK(td.seconds() == 0);
    CHECK(td.milliseconds() == 0);
    CHECK(td.microseconds() == 0);
    CHECK(td.ticks() == 86400000000LL);
    CHECK(td.isNegative() == false);

    /** @arg 正常初始化，分大于59 */
    td = TimeDelta(0, 0, 60);
    CHECK(td.days() == 0);
    CHECK(td.hours() == 1);
    CHECK(td.minutes() == 0);
    CHECK(td.seconds() == 0);
    CHECK(td.milliseconds() == 0);
    CHECK(td.microseconds() == 0);
    CHECK(td.ticks() == 3600000000LL);
    CHECK(td.isNegative() == false);

    /** @arg 正常初始化，秒大于59 */
    td = TimeDelta(0, 0, 0, 60);
    CHECK(td.days() == 0);
    CHECK(td.hours() == 0);
    CHECK(td.minutes() == 1);
    CHECK(td.seconds() == 0);
    CHECK(td.milliseconds() == 0);
    CHECK(td.microseconds() == 0);
    CHECK(td.ticks() == 60000000LL);
    CHECK(td.isNegative() == false);

    /** @arg 负时长初始化, 天数为 -1 */
    td = TimeDelta(-1);
    CHECK(td.isNegative());
    CHECK(td.days() == -1);
    CHECK(td.hours() == 0);
    CHECK(td.minutes() == 0);
    CHECK(td.seconds() == 0);
    CHECK(td.milliseconds() == 0);
    CHECK(td.microseconds() == 0);
    CHECK(td.ticks() == -86400000000LL);

    /** @arg 负时长初始化, hours = -1 */
    td = TimeDelta(0, -1);
    CHECK(td.isNegative());
    CHECK(td.days() == -1);
    CHECK(td.hours() == 23);
    CHECK(td.minutes() == 0);
    CHECK(td.seconds() == 0);
    CHECK(td.milliseconds() == 0);
    CHECK(td.microseconds() == 0);
    CHECK(td.ticks() == -60 * 60 * 1000000LL);

    /** @arg 负时长初始化, minutes = -1 */
    td = TimeDelta(0, 0, -1);
    CHECK(td.isNegative());
    CHECK(td.days() == -1);
    CHECK(td.hours() == 23);
    CHECK(td.minutes() == 59);
    CHECK(td.seconds() == 0);
    CHECK(td.milliseconds() == 0);
    CHECK(td.microseconds() == 0);
    CHECK(td.ticks() == -60000000LL);

    /** @arg 负时长初始化, seconds = -1 */
    td = TimeDelta(0, 0, 0, -1);
    CHECK(td.isNegative());
    CHECK(td.days() == -1);
    CHECK(td.hours() == 23);
    CHECK(td.minutes() == 59);
    CHECK(td.seconds() == 59);
    CHECK(td.milliseconds() == 0);
    CHECK(td.microseconds() == 0);
    CHECK(td.ticks() == -1000000LL);

    /** @arg 负时长初始化, milliseconds = -1 */
    td = TimeDelta(0, 0, 0, 0, -1);
    CHECK(td.isNegative());
    CHECK(td.days() == -1);
    CHECK(td.hours() == 23);
    CHECK(td.minutes() == 59);
    CHECK(td.seconds() == 59);
    CHECK(td.milliseconds() == 999);
    CHECK(td.microseconds() == 0);
    CHECK(td.ticks() == -1000LL);

    /** @arg 负时长初始化, microseconds = -1 */
    td = TimeDelta(0, 0, 0, 0, 0, -1);
    CHECK(td.isNegative());
    CHECK(td.days() == -1);
    CHECK(td.hours() == 23);
    CHECK(td.minutes() == 59);
    CHECK(td.seconds() == 59);
    CHECK(td.milliseconds() == 999);
    CHECK(td.microseconds() == 999);
    CHECK(td.ticks() == -1LL);

    /** @arg 负时长初始化, microseconds = -999 */
    td = TimeDelta(0, 0, 0, 0, 0, -999);
    CHECK(td.isNegative());
    CHECK(td.days() == -1);
    CHECK(td.hours() == 23);
    CHECK(td.minutes() == 59);
    CHECK(td.seconds() == 59);
    CHECK(td.milliseconds() == 999);
    CHECK(td.microseconds() == 1);
    CHECK(td.ticks() == -999LL);

    /** @arg 负时长初始化，所有参数均为负数 */
    td = TimeDelta(-1, -2, -13, -11, -12, -15);
    CHECK(td.isNegative());
    CHECK(td.days() == -2);
    CHECK(td.hours() == 21);
    CHECK(td.minutes() == 46);
    CHECK(td.seconds() == 48);
    CHECK(td.milliseconds() == 987);
    CHECK(td.microseconds() == 985);
    CHECK(td.ticks() == -94391012015LL);

    /** @arg 正时长初始化，参数正负混合 */
    td = TimeDelta(2, -23, -10, 4, 80, -917);
    CHECK(td.days() == 1);
    CHECK(td.hours() == 0);
    CHECK(td.minutes() == 50);
    CHECK(td.seconds() == 4);
    CHECK(td.milliseconds() == 79);
    CHECK(td.microseconds() == 83);
    CHECK(td.ticks() == 89404079083LL);
}

/** @par 检测点 */
TEST_CASE("test_TimeDelta_operator") {
    /** @arg 相加, 正正相加*/
    TimeDelta td = TimeDelta(1, 20, 100, 1, 3, 5) + TimeDelta(30, 3, 2, 4, 5, 6);
    CHECK(td.isNegative() == false);
    CHECK(td.days() == 32);
    CHECK(td.hours() == 0);
    CHECK(td.minutes() == 42);
    CHECK(td.seconds() == 5);
    CHECK(td.milliseconds() == 8);
    CHECK(td.microseconds() == 11);
    CHECK(td.ticks() == 2767325008011LL);

    /** @arg 相加，和为 0 时长 */
    td = TimeDelta(1) + TimeDelta(-1);
    CHECK(td.isNegative() == false);
    CHECK(td.days() == 0);
    CHECK(td.hours() == 0);
    CHECK(td.minutes() == 0);
    CHECK(td.seconds() == 0);
    CHECK(td.milliseconds() == 0);
    CHECK(td.microseconds() == 0);
    CHECK(td.ticks() == 0);

    /** @arg 相加，和为负 */
    td = TimeDelta(-1) + TimeDelta(0, -1);
    CHECK(td.isNegative() == true);
    CHECK(td.days() == -2);
    CHECK(td.hours() == 23);
    CHECK(td.minutes() == 0);
    CHECK(td.seconds() == 0);
    CHECK(td.milliseconds() == 0);
    CHECK(td.microseconds() == 0);
    CHECK(td.ticks() == -90000000000LL);

    /** @arg + 号 */
    CHECK(+TimeDelta(1) == TimeDelta(1));
    CHECK(+TimeDelta(-1) == TimeDelta(-1));

    /** @arg -号 */
    CHECK(-TimeDelta(1) == TimeDelta(-1));
    CHECK(-TimeDelta(-1) == TimeDelta(1));

    /** @arg 相减，结果为0 */
    td = TimeDelta(1, 0, 1) - TimeDelta(0, 24, 0, 60);
    CHECK(td.isNegative() == false);
    CHECK(td.days() == 0);
    CHECK(td.hours() == 0);
    CHECK(td.minutes() == 0);
    CHECK(td.seconds() == 0);
    CHECK(td.milliseconds() == 0);
    CHECK(td.microseconds() == 0);
    CHECK(td.ticks() == 0);

    /** @arg 相减，结果为负 */
    td = TimeDelta(0, 0, 1) - TimeDelta(0, 1);
    CHECK(td.isNegative() == true);
    CHECK(td.days() == -1);
    CHECK(td.hours() == 23);
    CHECK(td.minutes() == 1);
    CHECK(td.seconds() == 0);
    CHECK(td.milliseconds() == 0);
    CHECK(td.microseconds() == 0);
    CHECK(td.ticks() == -3540000000LL);

    /** @arg 求绝对值 */
    td = TimeDelta() - Microseconds(1);
    CHECK(td == Microseconds(-1));
    CHECK(td.abs() == Microseconds(1));

    /** @arg 乘以0 */
    td = TimeDelta(1, 1, 3) * 0;
    CHECK(td == TimeDelta());

    /** @arg 乘以大于0的整数 */
    td = TimeDelta(1, 1, 3) * 2;
    CHECK(td == TimeDelta(2, 2, 6));

    /** @arg 乘以大于0的小数 */
    td = TimeDelta(2, 2, 6) * 0.5;
    CHECK(td == TimeDelta(1, 1, 3));

    /** @arg 乘以小于0的整数 */
    td = TimeDelta(1, 1, 3) * -2;
    CHECK(td == TimeDelta(-2, -2, -6));

    /** @arg 乘以小于0的小数 */
    td = TimeDelta(-2, -2, -6) * 0.5;
    CHECK(td == TimeDelta(-1, -1, -3));

    /** @arg 正 TimeDelat 乘以负数 */
    td = TimeDelta(1) * -2;
    CHECK(td == TimeDelta(-2));

    /** @arg 负 TimeDelat 乘以负数 */
    td = TimeDelta(-1) * -2;
    CHECK(td == TimeDelta(2));

    /** @arg 除以 0 */
    CHECK_THROWS(TimeDelta(1) / 0);

    /** @arg 正常除法 */
    CHECK(TimeDelta(2) / 2 == TimeDelta(1));
    CHECK(TimeDelta(2) / TimeDelta(1) == 2);
    CHECK(Microseconds(1) / 3 == TimeDelta(0));
    CHECK(Microseconds(2) / 3 == Microseconds(1));

    /** @arg 地板除 */
    CHECK_THROWS(TimeDelta(1).floorDiv(0));
    CHECK(TimeDelta(2).floorDiv(2) == TimeDelta(1));
    CHECK(Microseconds(1).floorDiv(3) == TimeDelta(0));
    CHECK(Microseconds(2).floorDiv(3) == TimeDelta(0));

    /** @arg 除以 zero TimeDelta */
    CHECK_THROWS(TimeDelta(1) / TimeDelta());

    /** @arg 对零时长取余 */
    CHECK_THROWS(TimeDelta(1) % TimeDelta());

    /** @arg 取余 */
    CHECK(TimeDelta(3) % TimeDelta(2) == TimeDelta(1));

    /** @arg 相等 */
    CHECK(TimeDelta(1, 2, 1, 1, 1, 1) == TimeDelta(1, 2, 1, 1, 1, 1));
    CHECK(TimeDelta(1) == TimeDelta(0, 24));
    CHECK(TimeDelta(-1) == TimeDelta(0, -24));

    /** @arg 不等 */
    CHECK(TimeDelta(1, 2) != TimeDelta(1));

    /** @arg >, <, >=, <= */
    CHECK(TimeDelta(1) > TimeDelta(0, 23, 59));
    CHECK(TimeDelta(1) >= TimeDelta(0, 23));
    CHECK(TimeDelta(0, 0, 0, 0, 0, 1) > TimeDelta());
    CHECK(TimeDelta(0, 0, 0, 0, 0, 1) > TimeDelta(0, 0, 0, 0, 0, -1));
    CHECK(TimeDelta(1) < TimeDelta(2));
    CHECK(TimeDelta(2) <= TimeDelta(2));

    /** @arg total_days */
    CHECK(TimeDelta(1, 12).total_days() == 1.5);

    /** @arg total_hours */
    CHECK(TimeDelta(1, 12, 30).total_hours() == 36.5);

    /** @arg total_minutes */
    CHECK(TimeDelta(1, 12, 30, 30).total_minutes() == (24 + 12) * 60 + 30 + 0.5);

    /** @arg total_seconds */
    CHECK(TimeDelta(0, 0, 0, 1, 500).total_seconds() == 1.5);

    /** @arg total_milliseconds */
    CHECK(TimeDelta(0, 0, 0, 0, 1, 500).total_milliseconds() == 1.5);
}

/** @par 检测点 */
TEST_CASE("test_TimeDelta_subclass") {
    /** @arg Days */
    CHECK(Days(1) == TimeDelta(1));
    CHECK(TimeDelta(1) == Days(1));

    /** @arg Hours */
    CHECK(Hours(1) == TimeDelta(0, 1));
    CHECK(TimeDelta(0, 1) == Hours(1));

    /** @arg Minutes */
    CHECK(Minutes(1) == TimeDelta(0, 0, 1));
    CHECK(TimeDelta(0, 0, 1) == Minutes(1));

    /** @arg Seconds */
    CHECK(Seconds(1) == TimeDelta(0, 0, 0, 1));
    CHECK(TimeDelta(0, 0, 0, 1) == Seconds(1));

    /** @arg Milliseconds */
    CHECK(Milliseconds(1) == TimeDelta(0, 0, 0, 0, 1));
    CHECK(TimeDelta(0, 0, 0, 0, 1) == Milliseconds(1));

    /** @arg Microseconds */
    CHECK(Microseconds(1) == TimeDelta(0, 0, 0, 0, 0, 1));
    CHECK(TimeDelta(0, 0, 0, 0, 0, 1) == Microseconds(1));
}

/** @par 检测点 */
TEST_CASE("test_TimeDelta_Datetime_operator") {
    /** @arg Datetime + TimeDelta */
    Datetime d = Datetime(2019, 12, 18) + TimeDelta(1);
    CHECK(d == Datetime(2019, 12, 19));

    d = Datetime(2019, 12, 31, 23, 59, 59, 999, 999) + Microseconds(1);
    CHECK(d == Datetime(2020, 1, 1));

    d = Datetime(2019, 12, 18) + TimeDelta(-1);
    CHECK(d == Datetime(2019, 12, 17));

    /** @arg TimeDelta + Datetime*/
    d = TimeDelta(1) + Datetime(2019, 12, 18);
    CHECK(d == Datetime(2019, 12, 19));

    /** @arg Datetime - TimeDelta */
    d = Datetime(2019, 12, 18) - TimeDelta(0, 0, 1);
    CHECK(d == Datetime(2019, 12, 17, 23, 59));

    d = Datetime(2019, 12, 18) - TimeDelta(-2);
    CHECK(d == Datetime(2019, 12, 20));
}

#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_TimeDelat_serialize") {
    string filename(StockManager::instance().tmpdir());
    filename += "/TimeDelta.xml";

    TimeDelta td1(1, 2, 3, 4, 5, 6);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(td1);
    }

    TimeDelta td2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(td2);
    }

    CHECK(td1 == td2);
}

#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */