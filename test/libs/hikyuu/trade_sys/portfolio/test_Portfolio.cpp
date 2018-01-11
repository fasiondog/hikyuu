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
#include <hikyuu/trade_sys/portfolio/Portfolio.h>

using namespace hku;

/**
 * @defgroup test_Portfolio test_Portfolio
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_Portfolio) {
    PortfolioPtr pf = make_shared<Portfolio>();

    BOOST_CHECK(pf->name() == "Portfolio");

    StockManager& sm = StockManager::instance();

    pf->addStock(sm["sh000001"]);
    //std::cout << sys << std::endl;
}

/** @} */


