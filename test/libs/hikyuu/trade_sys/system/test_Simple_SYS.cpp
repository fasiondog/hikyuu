/*
 * test_Simple_SYS.cpp
 *
 *  Created on: 2013-4-20
 *      Author: fasiondog
 */


#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_trade_sys_suite
    #include <boost/test/unit_test.hpp>
#endif

#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/system/crt/Simple_SYS.h>

using namespace hku;

/**
 * @defgroup test_Simple_SYS test_Simple_SYS
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_Simple_SYS) {
    //TODO test_Simple_SYS
    //SystemPtr sys = Simple_SYS();
    //std::cout << sys << std::endl;
}

/** @} */


