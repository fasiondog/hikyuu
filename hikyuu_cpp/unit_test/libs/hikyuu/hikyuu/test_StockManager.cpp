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

#include <boost/filesystem.hpp>
#include <hikyuu/StockManager.h>
#include <hikyuu/utilities/util.h>
#include <hikyuu/Log.h>

using namespace hku;
using namespace boost::filesystem;

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
    BOOST_CHECK(result.size() == 3);
#if defined(BOOST_WINDOWS)
    BOOST_CHECK(result[0] == "TMP");
    BOOST_CHECK(result[1] == "SH");
    BOOST_CHECK(result[2] == "SZ");
#else
    BOOST_CHECK(result[0] == "TMP");
    BOOST_CHECK(result[1] == "SZ");
    BOOST_CHECK(result[2] == "SH");
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

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_StockManager_TempCsvStock) {
    StockManager& sm = StockManager::instance();

    path tmp_dir(sm.tmpdir());
    tmp_dir = tmp_dir.parent_path();

    string day_filename(tmp_dir.string() + "/test_day_data.csv");
    string min_filename(tmp_dir.string() + "/test_min_data.csv");

    /** @arg 增加临时增加返还的Stock的基本属性 */
    Stock stk = sm.addTempCsvStock("test", day_filename, min_filename);
    BOOST_CHECK(stk.isNull() == false);
    BOOST_CHECK(stk.market() == "TMP");
    BOOST_CHECK(stk.code() == "TEST");
    BOOST_CHECK(stk.market_code() == "TMPTEST");
    BOOST_CHECK(stk.getCount(KQuery::DAY) == 100);
    BOOST_CHECK(stk.getCount(KQuery::MIN) == 24000);

    /** @arg 增加临时增加返还的Stock的KRecord[0]（第一个数据）读取*/
    KRecord record;
    record = stk.getKRecord(0);
    BOOST_CHECK(record.datetime == Datetime(201703070000));
    BOOST_CHECK((record.openPrice-3233.09) < 0.00001);
    BOOST_CHECK((record.highPrice-3242.66) < 0.00001);
    BOOST_CHECK((record.lowPrice-3226.82) < 0.00001);
    BOOST_CHECK((record.closePrice-3242.41) < 0.00001);
    BOOST_CHECK((record.transAmount-20993120.6) < 0.00001);
    BOOST_CHECK((record.transCount-164064235.0) < 0.00001);

    /** @arg 增加临时增加返还的Stock的KRecord[10]（中间的数据）读取*/
    record = stk.getKRecord(10);
    BOOST_CHECK(record.datetime == Datetime(201703210000));
    BOOST_CHECK((record.openPrice-3250.25) < 0.00001);
    BOOST_CHECK((record.highPrice-3262.22) < 0.00001);
    BOOST_CHECK((record.lowPrice-3246.70) < 0.00001);
    BOOST_CHECK((record.closePrice-3261.61) < 0.00001);
    BOOST_CHECK((record.transAmount-21912127.0) < 0.00001);
    BOOST_CHECK((record.transCount-162719306.0) < 0.00001);

    /** @arg 增加临时增加返还的Stock的KRecord[99]（最后一个数据）读取*/
    record = stk.getKRecord(99);
    BOOST_CHECK(record.datetime == Datetime(201707310000));
    BOOST_CHECK((record.openPrice-3252.75) < 0.00001);
    BOOST_CHECK((record.highPrice-3276.95) < 0.00001);
    BOOST_CHECK((record.lowPrice-3251.19) < 0.00001);
    BOOST_CHECK((record.closePrice-3273.03) < 0.00001);
    BOOST_CHECK((record.transAmount-25352591.70) < 0.00001);
    BOOST_CHECK((record.transCount-246039440.0) < 0.00001);

    /** @arg 使用getStock获取临时加入的Stock */
    stk = sm.getStock("tmptest");
    BOOST_CHECK(stk.isNull() == false);
    BOOST_CHECK(stk.market() == "TMP");
    BOOST_CHECK(stk.code() == "TEST");
    BOOST_CHECK(stk.market_code() == "TMPTEST");
    BOOST_CHECK(stk.getCount(KQuery::DAY) == 100);
    BOOST_CHECK(stk.getCount(KQuery::MIN) == 24000);

    /** @arg 使用getStock获取临时加入的Stock的KRecord[10]读取*/
    record = stk.getKRecord(10);
    BOOST_CHECK(record.datetime == Datetime(201703210000));
    BOOST_CHECK((record.openPrice-3250.25) < 0.00001);
    BOOST_CHECK((record.highPrice-3262.22) < 0.00001);
    BOOST_CHECK((record.lowPrice-3246.70) < 0.00001);
    BOOST_CHECK((record.closePrice-3261.61) < 0.00001);
    BOOST_CHECK((record.transAmount-21912127.0) < 0.00001);
    BOOST_CHECK((record.transCount-162719306.0) < 0.00001);

    /** @arg 删除临时加入的Stock */
    sm.removeTempCsvStock("test");
    stk = sm.getStock("tmptest");
    BOOST_CHECK(stk.isNull() == true);
}

/** @} */
