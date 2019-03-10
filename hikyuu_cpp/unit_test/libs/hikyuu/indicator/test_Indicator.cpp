/*
 * test_Indicator.cpp
 *
 *  Created on: 2013-4-11
 *      Author: fasiondog
 */

#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_indicator_suite
    #include <boost/test/unit_test.hpp>
#endif

#include <hikyuu/indicator/Indicator.h>
#include <hikyuu/StockManager.h>

using namespace hku;

/**
 * @defgroup test_indicator_Indicator test_indicator_Indicator
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_operator_add ) {
    /** @arg 正常相加*/
    Indicator ind1;
    Indicator ind2;
    std::cout << ind1.size() << std::endl;
    std::cout << ind2 << std::endl;
    Indicator ind3 = ind1(ind2);
    std::cout << ind3 << std::endl;
}

/** @} */
