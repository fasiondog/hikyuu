#ifdef TEST_ALL_IN_ONE
#include <boost/test/unit_test.hpp>
#else
#define BOOST_TEST_MODULE test_hikyuu_base
#include <boost/test/unit_test.hpp>
#endif

#include <hikyuu/datetime/TimeDelta.h>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace hku;

namespace bt = boost::posix_time;
namespace bd = boost::gregorian;

BOOST_AUTO_TEST_CASE(test_TimeDelta) {
    // auto x = TimeDelta(99999999, 10000, 50000, 86399, 999, 999);
    // auto x = TimeDelta(99999999, 23, 59, 59, 999, 999);
    auto x = TimeDelta(1, 10000, 50000, 86399, 0, 0);
    std::cout << x << std::endl;
    std::cout << x.time_duration() << std::endl;
    std::cout << x.time_duration().hours() << std::endl;
    std::cout << x.time_duration().minutes() << std::endl;
    std::cout << x.time_duration().is_pos_infinity() << std::endl;
    std::cout << x.time_duration().is_neg_infinity() << std::endl;
}
