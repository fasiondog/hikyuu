/*
 * test_SYS_Simple.cpp
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
#include <hikyuu/trade_sys/portfolio/crt/PF_Simple.h>

using namespace hku;

/**
 * @defgroup test_Portfolio test_Portfolio
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 Portfolio基础操作 */
BOOST_AUTO_TEST_CASE( test_PF_for_base) {
    PortfolioPtr pf = PF_Simple();
    BOOST_CHECK(pf->name() == "Portfolio");

    /** @arg 克隆操作 */
    PFPtr pf2 = pf->clone();
    BOOST_CHECK(pf2.get() != pf.get());
    BOOST_CHECK(pf2->name() == pf->name());
}

/** @} */


