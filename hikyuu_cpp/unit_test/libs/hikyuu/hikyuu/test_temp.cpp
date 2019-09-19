#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_base
    #include <boost/test/unit_test.hpp>
#endif

#include <hikyuu/StockManager.h>
#include <hikyuu/data_driver/HistoryFinanceReader.h>
#include <hikyuu/utilities/thread/ThreadPool.h>

using namespace hku;

BOOST_AUTO_TEST_CASE( test_temp ) {
/*    ThreadPool tg;
    for (int i = 0; i < 10; i++) {
        tg.submit([=]() {
            std::stringstream buf;
            buf << i << ": --------------------" << std::endl;
            std::cout << buf.str();
        });
    }
    tg.wait_finish();
    std::cout << "*********************" << std::endl;
    */
}
