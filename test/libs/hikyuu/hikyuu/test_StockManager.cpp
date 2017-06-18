/*
 * test_StockManager.cpp
 *
 *  Created on: 2011-11-10
 *      Author: fasiondog
 */

#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_base
    #include <boost/test/unit_test.hpp>
#endif

#include <hikyuu/StockManager.h>
#include <hikyuu/utilities/util.h>
#include <hikyuu/Log.h>

using namespace hku;

/**
 * @defgroup test_hikyuu_StockManager test_hikyuu_StockManager
 * @ingroup test_hikyuu_base_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_StockManager_size ) {
    StockManager& sm = StockManager::instance();
    /** @arg 检测是否和测试数据中证券数相符 */
    BOOST_CHECK(sm.size() == 4729);
}

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_StockManager_getStock ) {
    StockManager& sm = StockManager::instance();
    Stock stock;
    Stock null_stock = Null<Stock>();

    /** @arg 根据“市场简称证券代码”查询，对应的市场不存在 */
    BOOST_CHECK(sm.getStock("z000001") == null_stock);

    /** @arg 根据“市场简称证券代码”查询，对应的市场存在，但证券代码不存在 */
    BOOST_CHECK(sm.getStock("sh1000001") == null_stock);

    /** @arg 根据“市场简称证券代码”查询正常存在的stock */
    stock = sm.getStock("sh000001");
    BOOST_CHECK(stock.market() == "SH");
    BOOST_CHECK(stock.code() == "000001");
    BOOST_CHECK(stock.name() == HKU_STR("上证指数"));
    BOOST_CHECK(stock.type() == 2);
    BOOST_CHECK(stock.valid() == true);
    BOOST_CHECK(stock.startDatetime() == Datetime(199012190000));
    BOOST_CHECK(stock.lastDatetime() == Null<Datetime>());
    BOOST_CHECK(stock.tick() == 0.001);
    BOOST_CHECK(stock.tickValue() == 0.001);
    BOOST_CHECK(stock.unit() == 1.0);
    BOOST_CHECK(stock.precision() == 3);
    BOOST_CHECK(stock.atom() == 1);
    BOOST_CHECK(stock.minTradeNumber() == 1);
    BOOST_CHECK(stock.maxTradeNumber() == 1000000);
}

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_StockManager_getMarketInfo ) {
    StockManager& sm = StockManager::instance();

    /** @arg 查询不存在的市场 */
    BOOST_CHECK(Null<MarketInfo>() == sm.getMarketInfo("XXX"));
    BOOST_CHECK(Null<MarketInfo>() == sm.getMarketInfo(""));

    /** @arg 查询已存在的市场，市场简称全部大写 */
    MarketInfo marketInfo = sm.getMarketInfo("SH");
    BOOST_CHECK(marketInfo != Null<MarketInfo>());
    BOOST_CHECK(marketInfo.market() == "SH");
    BOOST_CHECK(marketInfo.code() == "000001");
    BOOST_CHECK(marketInfo.name() == HKU_STR("上海证劵交易所"));
    BOOST_CHECK(marketInfo.description() == HKU_STR("上海市场"));
    BOOST_CHECK(marketInfo.lastDate() == Datetime(201112060000L));

    /** @arg 查询已存在的市场，市场简称全部小写 */
    marketInfo = sm.getMarketInfo("sh");
    BOOST_CHECK(marketInfo != Null<MarketInfo>());
    BOOST_CHECK(marketInfo.market() == "SH");


    /** @arg 查询已存在的市场，市场简称大小写混写 */
    marketInfo = sm.getMarketInfo("Sh");
    BOOST_CHECK(marketInfo != Null<MarketInfo>());
    BOOST_CHECK(marketInfo.market() == "SH");
}

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_StockManager_getStockTypeInfo ) {
    StockManager& sm = StockManager::instance();
    StockTypeInfo stockTypeInfo;

    /** @arg 查询不存在的type */
    BOOST_CHECK(sm.getStockTypeInfo(999) == Null<StockTypeInfo>());

    /** @arg 查询第一条记录，即type=0 */
    stockTypeInfo = sm.getStockTypeInfo(0);
    BOOST_CHECK(stockTypeInfo != Null<StockTypeInfo>());
    BOOST_CHECK(stockTypeInfo.type() == 0);
    BOOST_CHECK(stockTypeInfo.description() == HKU_STR("Block"));
    BOOST_CHECK(stockTypeInfo.tick() == 100);
    BOOST_CHECK(stockTypeInfo.precision() == 2);
    BOOST_CHECK(stockTypeInfo.minTradeNumber() == 100);
    BOOST_CHECK(stockTypeInfo.maxTradeNumber() == 1000000);

    /** @arg 查询最后一条记录，即type=8 */
    stockTypeInfo = sm.getStockTypeInfo(8);
    BOOST_CHECK(stockTypeInfo != Null<StockTypeInfo>());
    BOOST_CHECK(stockTypeInfo.type() == 8);
#if !defined(BOOST_MSVC)
    //msvc中文支持不好，此处无法编译
    BOOST_CHECK(stockTypeInfo.description() == HKU_STR("创业板"));
#endif
    BOOST_CHECK(stockTypeInfo.tick() == 0.01);
    BOOST_CHECK(stockTypeInfo.precision() == 2);
    BOOST_CHECK(stockTypeInfo.minTradeNumber() == 100);
    BOOST_CHECK(stockTypeInfo.maxTradeNumber() == 1000000);

    /** @arg 查询正常存在信息 */
    stockTypeInfo = sm.getStockTypeInfo(2);
    BOOST_CHECK(stockTypeInfo != Null<StockTypeInfo>());
    BOOST_CHECK(stockTypeInfo.type() == 2);
    BOOST_CHECK(stockTypeInfo.description() == HKU_STR("指数"));
    BOOST_CHECK(stockTypeInfo.tick() == 0.001);
    BOOST_CHECK(stockTypeInfo.precision() == 3);
    BOOST_CHECK(stockTypeInfo.minTradeNumber() == 1);
    BOOST_CHECK(stockTypeInfo.maxTradeNumber() == 1000000);
}

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_StockManager_getAllMarket ) {
    StockManager& sm = StockManager::instance();

    /** @arg 检测测试数据中的Market */
    MarketList result(sm.getAllMarket());
    BOOST_CHECK(result.size() == 2);
#if defined(BOOST_WINDOWS)
    BOOST_CHECK(result[0] == "SH");
    BOOST_CHECK(result[1] == "SZ");
#else
    BOOST_CHECK(result[0] == "SZ");
    BOOST_CHECK(result[1] == "SH");
#endif
}

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_StockManager_getBlock ) {
    StockManager& sm = StockManager::instance();

    Block result = sm.getBlock("地域板块", "陕西");
    BOOST_CHECK(result.size() != 0);

    BlockList blk_list = sm.getBlockList("地域板块");
    blk_list = sm.getBlockList();
    BOOST_CHECK(blk_list.size() != 0);

}

/** @} */
