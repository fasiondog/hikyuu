/*
 * test_IF.cpp
 *
 *  Created on: 2019-3-30
 *      Author: fasiondog
 */


#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_indicator_suite
    #include <boost/test/unit_test.hpp>
#endif

#include <hikyuu/StockManager.h>
#include <hikyuu/Context.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/CVAL.h>

using namespace hku;

/**
 * @defgroup test_indicator_IF test_indicator_IF
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_IF ) {
    KData kdata = getStock("SH600000").getKData(KQuery(-10));
    Indicator x = IF(CLOSE() > OPEN(), CVAL(1), CVAL(0));
    x.setContext(kdata);
    for (int i = 0; i < x.size(); i++) {
        std::cout << i << " " << x[i] << std::endl;
    }
}

/** @} */

