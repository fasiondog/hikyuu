#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_base
    #include <boost/test/unit_test.hpp>
#endif

#include <hikyuu/StockManager.h>
#include <hikyuu/data_driver/HistoryFinanceReader.h>

using namespace hku;

BOOST_AUTO_TEST_CASE( test_temp ) {
    StockManager& sm = StockManager::instance();
    string dirname(sm.datadir() + "\\downloads\\finance");

    std::cout << "**************************" << std::endl;

    HistoryFinanceReader rd = HistoryFinanceReader(dirname);
    PriceList result = rd.getHistoryFinanceInfo(Datetime(201109300000), "SH", "600000");
    BOOST_CHECK(result.size() == 286);

    float x = 0xf8f8f8f8;
    std::cout << result.size() << std::endl;
    std::cout << x << std::endl;

    std::cout << "**************************" << std::endl;
}
