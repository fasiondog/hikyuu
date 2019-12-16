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

    /** @arg hours 超出限定值 */
    BOOST_CHECK_THROW(TimeDelta(0, 10001), hku::exception);
    BOOST_CHECK_THROW(TimeDelta(0, -10001), hku::exception);

    /** @arg minutes 超出限定值 */
    BOOST_CHECK_THROW(TimeDelta(0, 0, 50001), hku::exception);
    BOOST_CHECK_THROW(TimeDelta(0, 0, -50001), hku::exception);

    /** @arg seconds 超出限定值 */
    BOOST_CHECK_THROW(TimeDelta(0, 0, 0, 86400), hku::exception);
    BOOST_CHECK_THROW(TimeDelta(0, 0, 0, -86400), hku::exception);

    /** @arg milliseconds 超出限定值 */
    BOOST_CHECK_THROW(TimeDelta(0, 0, 0, 0, 1000), hku::exception);
    BOOST_CHECK_THROW(TimeDelta(0, 0, 0, 0, -1000), hku::exception);

    /** @arg microseconds 超出限定值 */
    BOOST_CHECK_THROW(TimeDelta(0, 0, 0, 0, 0, 1000), hku::exception);
    BOOST_CHECK_THROW(TimeDelta(0, 0, 0, 0, 0, -1000), hku::exception);

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
    std::cout << td << std::endl;
    std::cout << td.str() << std::endl;
    BOOST_CHECK(td.isNegative());
    BOOST_CHECK(td.days() == -1);
    BOOST_CHECK(td.hours() == 0);
    BOOST_CHECK(td.minutes() == 0);
    BOOST_CHECK(td.seconds() == 0);
    BOOST_CHECK(td.milliseconds() == 0);
    BOOST_CHECK(td.microseconds() == 0);
    BOOST_CHECK(td.ticks() == -86400000000LL);

    /** @arg 负时长初始化, hours = -1 */
    /*td = TimeDelta(0, -1);
    std::cout << td.str() << std::endl;
    std::cout << td << std::endl;
    std::cout << td.ticks() << std::endl;
    BOOST_CHECK(td.isNegative());
    BOOST_CHECK(td.days() == 0);
    BOOST_CHECK(td.hours() == 1);
    BOOST_CHECK(td.minutes() == 0);
    BOOST_CHECK(td.seconds() == 0);
    BOOST_CHECK(td.milliseconds() == 0);
    BOOST_CHECK(td.microseconds() == 0);
    BOOST_CHECK(td.ticks() == -60 * 60 * 1000000LL);*/

    /** @arg 负时长初始化, microseconds = -1 */
    td = TimeDelta(0, 0, 0, 0, 0, -1);
    std::cout << td.str() << std::endl;
    std::cout << td << std::endl;
    std::cout << td.ticks() << std::endl;
    BOOST_CHECK(td.isNegative());
    BOOST_CHECK(td.days() == -1);
    BOOST_CHECK(td.hours() == 0);
    BOOST_CHECK(td.minutes() == 0);
    BOOST_CHECK(td.seconds() == 0);
    BOOST_CHECK(td.milliseconds() == 999);
    BOOST_CHECK(td.microseconds() == 999);
    BOOST_CHECK(td.ticks() == -1LL);
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