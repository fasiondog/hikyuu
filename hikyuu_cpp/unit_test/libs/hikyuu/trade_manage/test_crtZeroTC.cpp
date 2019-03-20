/*
 * test_TC_Zero.cpp
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_trade_manage_suite
    #include <boost/test/unit_test.hpp>
#endif

#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/crt/TC_Zero.h>

#include <hikyuu/config.h>
#if HKU_SUPPORT_SERIALIZATION
#include <fstream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#endif

using namespace hku;

/**
 * @defgroup test_TC_Zero test_TC_Zero
 * @ingroup test_hikyuu_trade_manage_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_TC_Zero ) {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600004");
    TradeCostPtr cost_func = TC_Zero();
    CostRecord result;

    /** @arg 检查name */
    BOOST_CHECK(cost_func->name() == "TC_Zero");

    /** @arg 计算买入成本 */
    result = cost_func->getBuyCost(Datetime(200101010000), stock, 9.01, 1000);
    BOOST_CHECK(result == Null<CostRecord>());

    /** @arg 计算卖出成本 */
    result = cost_func->getSellCost(Datetime(200101010000), stock, 9.01, 1000);
    BOOST_CHECK(result == Null<CostRecord>());

    /** @arg 测试clone */
    TradeCostPtr cost_clone_func = cost_func->clone();
    BOOST_CHECK(cost_clone_func->name() == "TC_Zero");
    result = cost_clone_func->getBuyCost(Datetime(200101010000), stock, 9.01, 1000);
    BOOST_CHECK(result == Null<CostRecord>());
    result = cost_clone_func->getSellCost(Datetime(200101010000), stock, 9.01, 1000);
    BOOST_CHECK(result == Null<CostRecord>());
}


#if HKU_SUPPORT_SERIALIZATION
/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_ZeroCost_export ) {
    StockManager& sm = StockManager::instance();

    string filename(sm.tmpdir());
    filename += "/TC_Zero.xml";

    TradeCostPtr zero = TC_Zero();
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(zero);
    }

    TradeCostPtr zero2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(zero2);
    }

    Stock stock = sm.getStock("sh600004");

    CostRecord result;
    BOOST_CHECK(zero2->name() == "TC_Zero");
    result = zero2->getBuyCost(Datetime(200101010000), stock, 9.01, 1000);
    BOOST_CHECK(result == Null<CostRecord>());
    result = zero2->getSellCost(Datetime(200101010000), stock, 9.01, 1000);
    BOOST_CHECK(result == Null<CostRecord>());
}
#endif /* HKU_SUPPORT_SERIALIZATION */

/** @} */
