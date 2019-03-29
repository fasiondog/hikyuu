/*
 * test_LIUTONGPAN.cpp
 *
 *  Created on: 2019-3-29
 *      Author: fasiondog
 */


#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_indicator_suite
    #include <boost/test/unit_test.hpp>
#endif

#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/LIUTONGPAN.h>

using namespace hku;

/**
 * @defgroup test_indicator_LIUTONGPAN test_indicator_LIUTONGPAN
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_LIUTONGPAN ) {
    StockManager& sm = StockManager::instance();
    
    Stock stk = getStock("SH600004");
    KQuery query = KQueryByDate(Datetime(200301010000), Datetime(200708250000));
    KData k = stk.getKData(query);

    Indicator liutong = LIUTONGPAN(k);
    BOOST_CHECK(liutong.size() == k.size());
    size_t total = k.size();
    for (int i = 0; i < total; i++) {
        if (k[i].datetime < Datetime(200512200000)) {
            BOOST_CHECK(liutong[i] == 40000);
        } else if (k[i].datetime >= Datetime(200512200000) 
                && k[i].datetime < Datetime(200612200000)) {
            BOOST_CHECK(liutong[i] == 47600);
        } else {
            BOOST_CHECK(liutong[i] == 49696);
        }
    }
    std::cout << total << std::endl;


}

/** @} */

