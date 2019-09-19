/*
 * test_iniparser.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 * 
 *  Created on: 2010-5-26
 *      Author: fasiondog
 */


#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_utilities
    #include <boost/test/unit_test.hpp>
#endif

#include <hikyuu/utilities/thread/ThreadPool.h>

using namespace hku;

/**
 * @defgroup test_hikyuu_ThreadPool test_hikyuu_ThreadPool
 * @ingroup test_hikyuu_utilities
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_ThreadPool ) {
    ThreadPool tg;
    for (int i = 0; i < 10; i++) {
        tg.submit([&]() {
            std::cout << i << ": --------------------" << std::endl;
        });
    }
    //tg.wait_finish();
    std::cout << "*********************" << std::endl;
}

/** @} */