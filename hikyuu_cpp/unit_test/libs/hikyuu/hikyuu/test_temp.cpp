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
    Stock stk = getStock("sh600000");
    PriceList result = stk.getHistoryFinanceInfo(Datetime(201109300000));
    BOOST_CHECK(result.size() == 286);
    BOOST_CHECK_CLOSE(result[0], 1.067, 0.00001);
    BOOST_CHECK_CLOSE(result[1], 1.061, 0.00001);
    BOOST_CHECK_CLOSE(result[2], 1.360, 0.00001);
    BOOST_CHECK_CLOSE(result[3], 7.482, 0.00001);
    BOOST_CHECK_CLOSE(result[9], 0.0, 0.00001);
    BOOST_CHECK_CLOSE(result[14], 7.87818e+09, 0.00001);
    BOOST_CHECK_CLOSE(result[282], 6.327156e+06, 0.00001);
    BOOST_CHECK_CLOSE(result[285], 0.0, 0.00001);
    //for (int i = 0; i < 286; i++) {
    //    std::cout << result[i] << std::endl;
    //}
}
