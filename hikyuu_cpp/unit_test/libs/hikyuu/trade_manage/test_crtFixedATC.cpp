/*
 * test_TC_FixedA.cpp
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
#include <hikyuu/trade_manage/crt/TC_FixedA.h>

#include <hikyuu/config.h>
#if HKU_SUPPORT_SERIALIZATION
#include <fstream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#endif

using namespace hku;

/**
 * @defgroup test_TC_FixedA test_TC_FixedA
 * @ingroup test_hikyuu_trade_manage_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_TC_FixedA ) {
    StockManager& sm = StockManager::instance();
    Stock stock;
    CostRecord result, expect;
    TradeCostPtr cost_func = TC_FixedA(0.0018, 5, 0.001, 0.001, 1.0);

    /** @arg Stock is Null */
    result = cost_func->getBuyCost(Datetime(200101010000), stock, 10.0, 100);
    BOOST_CHECK(result == expect);
    result = cost_func->getSellCost(Datetime(200101010000), stock, 10.0, 1000);
    BOOST_CHECK(result == expect);

    /** @arg 买入沪市股票，少于1000股，佣金少于5元 */
    stock = sm.getStock("sh600004");
    result = cost_func->getBuyCost(Datetime(200101010000), stock, 10.0, 100);
    expect.commission = 5.0;
    expect.stamptax = 0.0;
    expect.transferfee = 1.0;
    expect.total = 6.0;
    BOOST_CHECK(result == expect);

    /** @arg 买入沪市股票，等于1000股，佣金大于5元 */
    result = cost_func->getBuyCost(Datetime(200101010000), stock, 10.0, 1000);
    expect.commission = 18.0;
    expect.stamptax = 0.0;
    expect.transferfee = 1.0;
    expect.total = 19.0;
    BOOST_CHECK(result == expect);

    /** @arg 买入沪市股票，大于1000股，佣金大于5元 */
    result = cost_func->getBuyCost(Datetime(200101010000), stock, 10.0, 2100);
    expect.commission = 37.80;
    expect.stamptax = 0.0;
    expect.transferfee = 2.1;
    expect.total = 39.9;
    BOOST_CHECK(result == expect);

    /** @arg 卖出沪市股票，少于1000股，佣金少于5元 */
    stock = sm.getStock("sh600004");
    result = cost_func->getSellCost(Datetime(200101010000), stock, 10.0, 100);
    expect.commission = 5.0;
    expect.stamptax = 1.0;
    expect.transferfee = 1.0;
    expect.total = 7.0;
    BOOST_CHECK(result == expect);

    /** @arg 卖出沪市股票，等于1000股，佣金大于5元 */
    result = cost_func->getSellCost(Datetime(200101010000), stock, 10.0, 1000);
    expect.commission = 18.0;
    expect.stamptax = 10.0;
    expect.transferfee = 1.0;
    expect.total = 29.0;
    BOOST_CHECK(result == expect);

    /** @arg 卖出沪市股票，大于1000股，佣金大于5元 */
    result = cost_func->getSellCost(Datetime(200101010000), stock, 10.0, 2100);
    expect.commission = 37.80;
    expect.stamptax = 21;
    expect.transferfee = 2.1;
    expect.total = 60.9;
    BOOST_CHECK(result == expect);

    /** @arg 买入深市股票，少于1000股，佣金少于5元 */
    stock = sm.getStock("sz000001");
    result = cost_func->getBuyCost(Datetime(200101010000), stock, 10.0, 100);
    expect.commission = 5.0;
    expect.stamptax = 0.0;
    expect.transferfee = 0.0;
    expect.total = 5.0;
    BOOST_CHECK(result == expect);

    /** @arg 买入深市股票，等于1000股，佣金大于5元 */
    result = cost_func->getBuyCost(Datetime(200101010000), stock, 10.0, 1000);
    expect.commission = 18.0;
    expect.stamptax = 0.0;
    expect.transferfee = 0.0;
    expect.total = 18.0;
    BOOST_CHECK(result == expect);

    /** @arg 买入深市股票，大于1000股，佣金大于5元 */
    result = cost_func->getBuyCost(Datetime(200101010000), stock, 10.0, 2100);
    expect.commission = 37.80;
    expect.stamptax = 0.0;
    expect.transferfee = 0.0;
    expect.total = 37.8;
    BOOST_CHECK(result == expect);

    /** @arg 卖出深市股票，少于1000股，佣金少于5元 */
    result = cost_func->getSellCost(Datetime(200101010000), stock, 10.0, 100);
    expect.commission = 5.0;
    expect.stamptax = 1.0;
    expect.transferfee = 0.0;
    expect.total = 6.0;
    BOOST_CHECK(result == expect);

    /** @arg 卖出深市股票，等于1000股，佣金大于5元 */
    result = cost_func->getSellCost(Datetime(200101010000), stock, 10.0, 1000);
    expect.commission = 18.0;
    expect.stamptax = 10.0;
    expect.transferfee = 0.0;
    expect.total = 28.0;
    BOOST_CHECK(result == expect);

    /** @arg 卖出深市股票，大于1000股，佣金大于5元 */
    result = cost_func->getSellCost(Datetime(200101010000), stock, 10.0, 2100);
    expect.commission = 37.80;
    expect.stamptax = 21;
    expect.transferfee = 0.0;
    expect.total = 58.8;
    BOOST_CHECK(result == expect);
}


#if HKU_SUPPORT_SERIALIZATION
/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_FixedATC_export ) {
    StockManager& sm = StockManager::instance();

    string filename(sm.tmpdir());
    filename += "/TC_FixedA.xml";

    TradeCostPtr func1 = TC_FixedA();
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(func1);
    }

    TradeCostPtr func2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(func2);
    }

    BOOST_CHECK(func2->name() == "FixedATradeCost");

    CostRecord result, expect;
    Stock stock = sm.getStock("sh600004");
    result = func2->getBuyCost(Datetime(200101010000), stock, 10.0, 100);
    expect.commission = 5.0;
    expect.stamptax = 0.0;
    expect.transferfee = 1.0;
    expect.total = 6.0;
    BOOST_CHECK(result == expect);

    result = func2->getBuyCost(Datetime(200101010000), stock, 10.0, 1000);
    expect.commission = 18.0;
    expect.stamptax = 0.0;
    expect.transferfee = 1.0;
    expect.total = 19.0;
    BOOST_CHECK(result == expect);
}
#endif /* HKU_SUPPORT_SERIALIZATION */

/** @} */
