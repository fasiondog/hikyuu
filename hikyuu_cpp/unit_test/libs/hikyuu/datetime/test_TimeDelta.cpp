#ifdef TEST_ALL_IN_ONE
#include <boost/test/unit_test.hpp>
#else
#define BOOST_TEST_MODULE test_hikyuu_base
#include <boost/test/unit_test.hpp>
#endif

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
BOOST_AUTO_TEST_CASE(test_TimeDelta) {
    /** @arg days  超出限定值 */
    BOOST_CHECK_THROW(TimeDelta(99999999LL + 1), hku::exception);
    BOOST_CHECK_THROW(TimeDelta(-99999999LL - 1), hku::exception);

#if !HKU_DISABLE_ASSERT
    /** @arg hours 超出限定值 */
    BOOST_CHECK_THROW(TimeDelta(0, 100001), hku::exception);
    BOOST_CHECK_THROW(TimeDelta(0, -100001), hku::exception);

    /** @arg minutes 超出限定值 */
    BOOST_CHECK_THROW(TimeDelta(0, 0, 100001), hku::exception);
    BOOST_CHECK_THROW(TimeDelta(0, 0, -100001), hku::exception);

    /** @arg seconds 超出限定值 */
    BOOST_CHECK_THROW(TimeDelta(0, 0, 0, 8640000), hku::exception);
    BOOST_CHECK_THROW(TimeDelta(0, 0, 0, -8640000), hku::exception);

    /** @arg milliseconds 超出限定值 */
    BOOST_CHECK_THROW(TimeDelta(0, 0, 0, 0, 8640000000000), hku::exception);
    BOOST_CHECK_THROW(TimeDelta(0, 0, 0, 0, -8640000000000), hku::exception);

    /** @arg microseconds 超出限定值 */
    BOOST_CHECK_THROW(TimeDelta(0, 0, 0, 0, 0, 8640000000000), hku::exception);
    BOOST_CHECK_THROW(TimeDelta(0, 0, 0, 0, 0, -8640000000000), hku::exception);
#endif

    /** @arg microseconds总值超出限定值 */
    BOOST_CHECK_THROW(TimeDelta(99999999LL, 23, 59, 60, 999, 999), hku::exception);

    /** @arg 正常初始化，时分秒毫秒微秒都在各自的进制范围内 */
    TimeDelta td(7, 10, 20, 3, 5, 7);
    BOOST_CHECK(td.days() == 7);
    BOOST_CHECK(td.hours() == 10);
    BOOST_CHECK(td.minutes() == 20);
    BOOST_CHECK(td.seconds() == 3);
    BOOST_CHECK(td.milliseconds() == 5);
    BOOST_CHECK(td.microseconds() == 7);
    BOOST_CHECK(td.isNegative() == false);

    /** @arg 正常初始化，时大于23 */
    td = TimeDelta(0, 24);
    BOOST_CHECK(td.days() == 1);
    BOOST_CHECK(td.hours() == 0);
    BOOST_CHECK(td.minutes() == 0);
    BOOST_CHECK(td.seconds() == 0);
    BOOST_CHECK(td.milliseconds() == 0);
    BOOST_CHECK(td.microseconds() == 0);
    BOOST_CHECK(td.ticks() == 86400000000LL);
    BOOST_CHECK(td.isNegative() == false);

    /** @arg 正常初始化，分大于59 */
    td = TimeDelta(0, 0, 60);
    BOOST_CHECK(td.days() == 0);
    BOOST_CHECK(td.hours() == 1);
    BOOST_CHECK(td.minutes() == 0);
    BOOST_CHECK(td.seconds() == 0);
    BOOST_CHECK(td.milliseconds() == 0);
    BOOST_CHECK(td.microseconds() == 0);
    BOOST_CHECK(td.ticks() == 3600000000LL);
    BOOST_CHECK(td.isNegative() == false);

    /** @arg 正常初始化，秒大于59 */
    td = TimeDelta(0, 0, 0, 60);
    BOOST_CHECK(td.days() == 0);
    BOOST_CHECK(td.hours() == 0);
    BOOST_CHECK(td.minutes() == 1);
    BOOST_CHECK(td.seconds() == 0);
    BOOST_CHECK(td.milliseconds() == 0);
    BOOST_CHECK(td.microseconds() == 0);
    BOOST_CHECK(td.ticks() == 60000000LL);
    BOOST_CHECK(td.isNegative() == false);

    /** @arg 负时长初始化, 天数为 -1 */
    td = TimeDelta(-1);
    BOOST_CHECK(td.isNegative());
    BOOST_CHECK(td.days() == -1);
    BOOST_CHECK(td.hours() == 0);
    BOOST_CHECK(td.minutes() == 0);
    BOOST_CHECK(td.seconds() == 0);
    BOOST_CHECK(td.milliseconds() == 0);
    BOOST_CHECK(td.microseconds() == 0);
    BOOST_CHECK(td.ticks() == -86400000000LL);

    /** @arg 负时长初始化, hours = -1 */
    td = TimeDelta(0, -1);
    BOOST_CHECK(td.isNegative());
    BOOST_CHECK(td.days() == -1);
    BOOST_CHECK(td.hours() == 23);
    BOOST_CHECK(td.minutes() == 0);
    BOOST_CHECK(td.seconds() == 0);
    BOOST_CHECK(td.milliseconds() == 0);
    BOOST_CHECK(td.microseconds() == 0);
    BOOST_CHECK(td.ticks() == -60 * 60 * 1000000LL);

    /** @arg 负时长初始化, minutes = -1 */
    td = TimeDelta(0, 0, -1);
    BOOST_CHECK(td.isNegative());
    BOOST_CHECK(td.days() == -1);
    BOOST_CHECK(td.hours() == 23);
    BOOST_CHECK(td.minutes() == 59);
    BOOST_CHECK(td.seconds() == 0);
    BOOST_CHECK(td.milliseconds() == 0);
    BOOST_CHECK(td.microseconds() == 0);
    BOOST_CHECK(td.ticks() == -60000000LL);

    /** @arg 负时长初始化, seconds = -1 */
    td = TimeDelta(0, 0, 0, -1);
    BOOST_CHECK(td.isNegative());
    BOOST_CHECK(td.days() == -1);
    BOOST_CHECK(td.hours() == 23);
    BOOST_CHECK(td.minutes() == 59);
    BOOST_CHECK(td.seconds() == 59);
    BOOST_CHECK(td.milliseconds() == 0);
    BOOST_CHECK(td.microseconds() == 0);
    BOOST_CHECK(td.ticks() == -1000000LL);

    /** @arg 负时长初始化, milliseconds = -1 */
    td = TimeDelta(0, 0, 0, 0, -1);
    BOOST_CHECK(td.isNegative());
    BOOST_CHECK(td.days() == -1);
    BOOST_CHECK(td.hours() == 23);
    BOOST_CHECK(td.minutes() == 59);
    BOOST_CHECK(td.seconds() == 59);
    BOOST_CHECK(td.milliseconds() == 999);
    BOOST_CHECK(td.microseconds() == 0);
    BOOST_CHECK(td.ticks() == -1000LL);

    /** @arg 负时长初始化, microseconds = -1 */
    td = TimeDelta(0, 0, 0, 0, 0, -1);
    BOOST_CHECK(td.isNegative());
    BOOST_CHECK(td.days() == -1);
    BOOST_CHECK(td.hours() == 23);
    BOOST_CHECK(td.minutes() == 59);
    BOOST_CHECK(td.seconds() == 59);
    BOOST_CHECK(td.milliseconds() == 999);
    BOOST_CHECK(td.microseconds() == 999);
    BOOST_CHECK(td.ticks() == -1LL);

    /** @arg 负时长初始化, microseconds = -999 */
    td = TimeDelta(0, 0, 0, 0, 0, -999);
    BOOST_CHECK(td.isNegative());
    BOOST_CHECK(td.days() == -1);
    BOOST_CHECK(td.hours() == 23);
    BOOST_CHECK(td.minutes() == 59);
    BOOST_CHECK(td.seconds() == 59);
    BOOST_CHECK(td.milliseconds() == 999);
    BOOST_CHECK(td.microseconds() == 1);
    BOOST_CHECK(td.ticks() == -999LL);

    /** @arg 负时长初始化，所有参数均为负数 */
    td = TimeDelta(-1, -2, -13, -11, -12, -15);
    BOOST_CHECK(td.isNegative());
    BOOST_CHECK(td.days() == -2);
    BOOST_CHECK(td.hours() == 21);
    BOOST_CHECK(td.minutes() == 46);
    BOOST_CHECK(td.seconds() == 48);
    BOOST_CHECK(td.milliseconds() == 987);
    BOOST_CHECK(td.microseconds() == 985);
    BOOST_CHECK(td.ticks() == -94391012015LL);

    /** @arg 正时长初始化，参数正负混合 */
    td = TimeDelta(2, -23, -10, 4, 80, -917);
    BOOST_CHECK(td.days() == 1);
    BOOST_CHECK(td.hours() == 0);
    BOOST_CHECK(td.minutes() == 50);
    BOOST_CHECK(td.seconds() == 4);
    BOOST_CHECK(td.milliseconds() == 79);
    BOOST_CHECK(td.microseconds() == 83);
    BOOST_CHECK(td.ticks() == 89404079083LL);
}

/** @par 检测点 */
BOOST_AUTO_TEST_CASE(test_TimeDelta_operator) {
    /** @arg 相加, 正正相加*/
    TimeDelta td = TimeDelta(1, 20, 100, 1, 3, 5) + TimeDelta(30, 3, 2, 4, 5, 6);
    BOOST_CHECK(td.isNegative() == false);
    BOOST_CHECK(td.days() == 32);
    BOOST_CHECK(td.hours() == 0);
    BOOST_CHECK(td.minutes() == 42);
    BOOST_CHECK(td.seconds() == 5);
    BOOST_CHECK(td.milliseconds() == 8);
    BOOST_CHECK(td.microseconds() == 11);
    BOOST_CHECK(td.ticks() == 2767325008011LL);

    /** @arg 相加，和为 0 时长 */
    td = TimeDelta(1) + TimeDelta(-1);
    BOOST_CHECK(td.isNegative() == false);
    BOOST_CHECK(td.days() == 0);
    BOOST_CHECK(td.hours() == 0);
    BOOST_CHECK(td.minutes() == 0);
    BOOST_CHECK(td.seconds() == 0);
    BOOST_CHECK(td.milliseconds() == 0);
    BOOST_CHECK(td.microseconds() == 0);
    BOOST_CHECK(td.ticks() == 0);

    /** @arg 相加，和为负 */
    td = TimeDelta(-1) + TimeDelta(0, -1);
    BOOST_CHECK(td.isNegative() == true);
    BOOST_CHECK(td.days() == -2);
    BOOST_CHECK(td.hours() == 23);
    BOOST_CHECK(td.minutes() == 0);
    BOOST_CHECK(td.seconds() == 0);
    BOOST_CHECK(td.milliseconds() == 0);
    BOOST_CHECK(td.microseconds() == 0);
    BOOST_CHECK(td.ticks() == -90000000000LL);

    /** @arg 相减，结果为0 */
    td = TimeDelta(1, 0, 1) - TimeDelta(0, 24, 0, 60);
    BOOST_CHECK(td.isNegative() == false);
    BOOST_CHECK(td.days() == 0);
    BOOST_CHECK(td.hours() == 0);
    BOOST_CHECK(td.minutes() == 0);
    BOOST_CHECK(td.seconds() == 0);
    BOOST_CHECK(td.milliseconds() == 0);
    BOOST_CHECK(td.microseconds() == 0);
    BOOST_CHECK(td.ticks() == 0);

    /** @arg 相减，结果为负 */
    td = TimeDelta(0, 0, 1) - TimeDelta(0, 1);
    BOOST_CHECK(td.isNegative() == true);
    BOOST_CHECK(td.days() == -1);
    BOOST_CHECK(td.hours() == 23);
    BOOST_CHECK(td.minutes() == 1);
    BOOST_CHECK(td.seconds() == 0);
    BOOST_CHECK(td.milliseconds() == 0);
    BOOST_CHECK(td.microseconds() == 0);
    BOOST_CHECK(td.ticks() == -3540000000LL);

    /** @arg 求绝对值 */
    td = TimeDelta() - Microseconds(1);
    BOOST_CHECK(td == Microseconds(-1));
    BOOST_CHECK(td.abs() == Microseconds(1));

    /** @arg 乘以0 */
    td = TimeDelta(1, 1, 3) * 0;
    BOOST_CHECK(td == TimeDelta());

    /** @arg 乘以大于0的整数 */
    td = TimeDelta(1, 1, 3) * 2;
    BOOST_CHECK(td == TimeDelta(2, 2, 6));

    /** @arg 乘以大于0的小数 */
    td = TimeDelta(2, 2, 6) * 0.5;
    BOOST_CHECK(td == TimeDelta(1, 1, 3));

    /** @arg 乘以小于0的整数 */
    td = TimeDelta(1, 1, 3) * -2;
    BOOST_CHECK(td == TimeDelta(-2, -2, -6));

    /** @arg 乘以小于0的小数 */
    td = TimeDelta(-2, -2, -6) * 0.5;
    BOOST_CHECK(td == TimeDelta(-1, -1, -3));

    /** @arg 正 TimeDelat 乘以负数 */
    td = TimeDelta(1) * -2;
    BOOST_CHECK(td == TimeDelta(-2));

    /** @arg 负 TimeDelat 乘以负数 */
    td = TimeDelta(-1) * -2;
    BOOST_CHECK(td == TimeDelta(2));

    /** @arg 除以 0 */
    BOOST_CHECK_THROW(TimeDelta(1) / 0, hku::exception);

    /** @arg 正常除法 */
    BOOST_CHECK(TimeDelta(2) / 2 == TimeDelta(1));
    BOOST_CHECK(TimeDelta(2) / TimeDelta(1) == 2);

    /** @arg 除以 zero TimeDelta */
    BOOST_CHECK_THROW(TimeDelta(1) / TimeDelta(), hku::exception);

    /** @arg 对零时长取余 */
    BOOST_CHECK_THROW(TimeDelta(1) % TimeDelta(), hku::exception);

    /** @arg 取余 */
    BOOST_CHECK(TimeDelta(3) % TimeDelta(2) == TimeDelta(1));

    /** @arg 相等 */
    BOOST_CHECK(TimeDelta(1, 2, 1, 1, 1, 1) == TimeDelta(1, 2, 1, 1, 1, 1));
    BOOST_CHECK(TimeDelta(1) == TimeDelta(0, 24));
    BOOST_CHECK(TimeDelta(-1) == TimeDelta(0, -24));

    /** @arg 不等 */
    BOOST_CHECK(TimeDelta(1, 2) != TimeDelta(1));

    /** @arg >, <, >=, <= */
    BOOST_CHECK(TimeDelta(1) > TimeDelta(0, 23, 59));
    BOOST_CHECK(TimeDelta(1) >= TimeDelta(0, 23));
    BOOST_CHECK(TimeDelta(0, 0, 0, 0, 0, 1) > TimeDelta());
    BOOST_CHECK(TimeDelta(0, 0, 0, 0, 0, 1) > TimeDelta(0, 0, 0, 0, 0, -1));
    BOOST_CHECK(TimeDelta(1) < TimeDelta(2));
    BOOST_CHECK(TimeDelta(2) <= TimeDelta(2));
}

/** @par 检测点 */
BOOST_AUTO_TEST_CASE(test_TimeDelta_subclass) {
    /** @arg Days */
    BOOST_CHECK(Days(1) == TimeDelta(1));
    BOOST_CHECK(TimeDelta(1) == Days(1));

    /** @arg Hours */
    BOOST_CHECK(Hours(1) == TimeDelta(0, 1));
    BOOST_CHECK(TimeDelta(0, 1) == Hours(1));

    /** @arg Minutes */
    BOOST_CHECK(Minutes(1) == TimeDelta(0, 0, 1));
    BOOST_CHECK(TimeDelta(0, 0, 1) == Minutes(1));

    /** @arg Seconds */
    BOOST_CHECK(Seconds(1) == TimeDelta(0, 0, 0, 1));
    BOOST_CHECK(TimeDelta(0, 0, 0, 1) == Seconds(1));

    /** @arg Milliseconds */
    BOOST_CHECK(Milliseconds(1) == TimeDelta(0, 0, 0, 0, 1));
    BOOST_CHECK(TimeDelta(0, 0, 0, 0, 1) == Milliseconds(1));

    /** @arg Microseconds */
    BOOST_CHECK(Microseconds(1) == TimeDelta(0, 0, 0, 0, 0, 1));
    BOOST_CHECK(TimeDelta(0, 0, 0, 0, 0, 1) == Microseconds(1));
}

/** @par 检测点 */
BOOST_AUTO_TEST_CASE(test_TimeDelta_Datetime_operator) {
    /** @arg Datetime + TimeDelta */
    Datetime d = Datetime(2019, 12, 18) + TimeDelta(1);
    BOOST_CHECK(d == Datetime(2019, 12, 19));

    d = Datetime(2019, 12, 31, 23, 59, 59, 999, 999) + Microseconds(1);
    BOOST_CHECK(d == Datetime(2020, 1, 1));

    d = Datetime(2019, 12, 18) + TimeDelta(-1);
    BOOST_CHECK(d == Datetime(2019, 12, 17));

    /** @arg Datetime - TimeDelta */
    d = Datetime(2019, 12, 18) - TimeDelta(0, 0, 1);
    BOOST_CHECK(d == Datetime(2019, 12, 17, 23, 59));

    d = Datetime(2019, 12, 18) - TimeDelta(-2);
    BOOST_CHECK(d == Datetime(2019, 12, 20));
}

/** @par 检测点 */
BOOST_AUTO_TEST_CASE(test_TimeDelat_serialize) {
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

    BOOST_CHECK(td1 == td2);
}

/** @} */