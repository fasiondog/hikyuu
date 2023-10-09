/*
 * test_StockManager.cpp
 *
 *  Created on: 2011-11-10
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <boost/filesystem.hpp>
#include <hikyuu/StockManager.h>
#include <hikyuu/utilities/runtimeinfo.h>
#include <hikyuu/Log.h>

using namespace hku;
using namespace boost::filesystem;

/**
 * @defgroup test_hikyuu_StockManager test_hikyuu_StockManager
 * @ingroup test_hikyuu_base_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_StockManager_size") {
    StockManager& sm = StockManager::instance();
    /** @arg 检测是否和测试数据中证券数相符 */
    CHECK_EQ(sm.size(), 4729);
}

/** @par 检测点 */
TEST_CASE("test_StockManager_getStock") {
    StockManager& sm = StockManager::instance();
    Stock stock;
    Stock null_stock = Null<Stock>();

    /** @arg 根据“市场简称证券代码”查询，对应的市场不存在 */
    CHECK_EQ(sm.getStock("z000001"), null_stock);

    /** @arg 根据“市场简称证券代码”查询，对应的市场存在，但证券代码不存在 */
    CHECK_EQ(sm.getStock("sh1000001"), null_stock);

    /** @arg 根据“市场简称证券代码”查询正常存在的stock */
    stock = sm.getStock("sh000001");
    CHECK_EQ(stock.market(), "SH");
    CHECK_EQ(stock.code(), "000001");
    CHECK_EQ(stock.name(), "上证指数");
    HKU_INFO("{}", stock.toString());
    CHECK_EQ(stock.type(), 2);
    CHECK_EQ(stock.valid(), true);
    CHECK_EQ(stock.startDatetime(), Datetime(199012190000));
    CHECK_EQ(stock.lastDatetime(), Null<Datetime>());
    CHECK_EQ(stock.tick(), 0.001);
    CHECK_EQ(stock.tickValue(), 0.001);
    CHECK_EQ(stock.unit(), 1.0);
    CHECK_EQ(stock.precision(), 3);
    CHECK_EQ(stock.atom(), 1);
    CHECK_EQ(stock.minTradeNumber(), 1);
    CHECK_EQ(stock.maxTradeNumber(), 1000000);
}

/** @par 检测点 */
TEST_CASE("test_StockManager_getMarketInfo") {
    StockManager& sm = StockManager::instance();

    /** @arg 查询不存在的市场 */
    CHECK_EQ(Null<MarketInfo>(), sm.getMarketInfo("XXX"));
    CHECK_EQ(Null<MarketInfo>(), sm.getMarketInfo(""));

    /** @arg 查询已存在的市场，市场简称全部大写 */
    MarketInfo marketInfo = sm.getMarketInfo("SH");
    CHECK_NE(marketInfo, Null<MarketInfo>());
    CHECK_EQ(marketInfo.market(), "SH");
    CHECK_EQ(marketInfo.code(), "000001");
    CHECK_EQ(marketInfo.name(), "上海证券交易所");
    CHECK_EQ(marketInfo.description(), "上海市场");
    CHECK_EQ(marketInfo.lastDate(), Datetime(201112060000L));
    CHECK_EQ(marketInfo.openTime1(), TimeDelta(0, 9, 30));
    CHECK_EQ(marketInfo.closeTime1(), TimeDelta(0, 11, 30));
    CHECK_EQ(marketInfo.openTime2(), TimeDelta(0, 13, 00));
    CHECK_EQ(marketInfo.closeTime2(), TimeDelta(0, 15, 00));

    /** @arg 查询已存在的市场，市场简称全部小写 */
    marketInfo = sm.getMarketInfo("sh");
    CHECK_NE(marketInfo, Null<MarketInfo>());
    CHECK_EQ(marketInfo.market(), "SH");

    /** @arg 查询已存在的市场，市场简称大小写混写 */
    marketInfo = sm.getMarketInfo("Sh");
    CHECK_NE(marketInfo, Null<MarketInfo>());
    CHECK_EQ(marketInfo.market(), "SH");
}

/** @par 检测点 */
TEST_CASE("test_StockManager_getStockTypeInfo") {
    StockManager& sm = StockManager::instance();
    StockTypeInfo stockTypeInfo;

    /** @arg 查询不存在的type */
    CHECK_EQ(sm.getStockTypeInfo(999), Null<StockTypeInfo>());

    /** @arg 查询第一条记录，即type=0 */
    stockTypeInfo = sm.getStockTypeInfo(0);
    CHECK_NE(stockTypeInfo, Null<StockTypeInfo>());
    CHECK_EQ(stockTypeInfo.type(), 0);
    CHECK_EQ(stockTypeInfo.description(), "Block");
    CHECK_EQ(stockTypeInfo.tick(), 100);
    CHECK_EQ(stockTypeInfo.precision(), 2);
    CHECK_EQ(stockTypeInfo.minTradeNumber(), 100);
    CHECK_EQ(stockTypeInfo.maxTradeNumber(), 1000000);

    /** @arg 查询最后一条记录，即type=8 */
    stockTypeInfo = sm.getStockTypeInfo(8);
    CHECK_NE(stockTypeInfo, Null<StockTypeInfo>());
    CHECK_EQ(stockTypeInfo.type(), 8);
    CHECK_EQ(stockTypeInfo.description(), "创业板");
    CHECK_EQ(stockTypeInfo.tick(), 0.01);
    CHECK_EQ(stockTypeInfo.precision(), 2);
    CHECK_EQ(stockTypeInfo.minTradeNumber(), 100);
    CHECK_EQ(stockTypeInfo.maxTradeNumber(), 1000000);

    /** @arg 查询正常存在信息 */
    stockTypeInfo = sm.getStockTypeInfo(2);
    CHECK_NE(stockTypeInfo, Null<StockTypeInfo>());
    CHECK_EQ(stockTypeInfo.type(), 2);
    CHECK_EQ(stockTypeInfo.description(), "指数");
    CHECK_EQ(stockTypeInfo.tick(), 0.001);
    CHECK_EQ(stockTypeInfo.precision(), 3);
    CHECK_EQ(stockTypeInfo.minTradeNumber(), 1);
    CHECK_EQ(stockTypeInfo.maxTradeNumber(), 1000000);
}

/** @par 检测点 */
TEST_CASE("test_StockManager_getAllMarket") {
    StockManager& sm = StockManager::instance();

    /** @arg 检测测试数据中的Market */
    MarketList result(sm.getAllMarket());
    CHECK_EQ(result.size(), 3);
    std::vector<string> want_list{"TMP", "SH", "SZ"};
    for (auto want : want_list) {
        bool found = false;
        for (auto i = 0; i < result.size(); i++) {
            if (result[i] == want) {
                found = true;
                break;
            }
        }
        CHECK_UNARY(found);
    }
}

/** @par 检测点 */
TEST_CASE("test_StockManager_getBlock") {
    if (supportChineseSimple()) {
        StockManager& sm = StockManager::instance();
        Block result = sm.getBlock("地域板块", "陕西");
        CHECK_NE(result.size(), 0);

        BlockList blk_list = sm.getBlockList("地域板块");
        blk_list = sm.getBlockList();
        CHECK_NE(blk_list.size(), 0);
    }
}

/** @par 检测点 */
TEST_CASE("test_StockManager_TempCsvStock") {
    StockManager& sm = StockManager::instance();

    path tmp_dir(sm.tmpdir());
    tmp_dir = tmp_dir.parent_path();

    string day_filename(tmp_dir.string() + "/test_day_data.csv");
    string min_filename(tmp_dir.string() + "/test_min_data.csv");

    /** @arg 增加临时增加返还的Stock的基本属性 */
    Stock stk = sm.addTempCsvStock("test", day_filename, min_filename);
    CHECK_EQ(stk.isNull(), false);
    CHECK_EQ(stk.market(), "TMP");
    CHECK_EQ(stk.code(), "TEST");
    CHECK_EQ(stk.market_code(), "TMPTEST");
    CHECK_EQ(stk.getCount(KQuery::DAY), 100);
    CHECK_EQ(stk.getCount(KQuery::MIN), 24000);

    /** @arg 增加临时增加返还的Stock的KRecord[0]（第一个数据）读取*/
    KRecord record;
    record = stk.getKRecord(0);
    CHECK_EQ(record.datetime, Datetime(201703070000));
    CHECK_LT((record.openPrice - 3233.09), 0.00001);
    CHECK_LT((record.highPrice - 3242.66), 0.00001);
    CHECK_LT((record.lowPrice - 3226.82), 0.00001);
    CHECK_LT((record.closePrice - 3242.41), 0.00001);
    CHECK_LT((record.transAmount - 20993120.6), 0.00001);
    CHECK_LT((record.transCount - 164064235.0), 0.00001);

    /** @arg 增加临时增加返还的Stock的KRecord[10]（中间的数据）读取*/
    record = stk.getKRecord(10);
    CHECK_EQ(record.datetime, Datetime(201703210000));
    CHECK_LT((record.openPrice - 3250.25), 0.00001);
    CHECK_LT((record.highPrice - 3262.22), 0.00001);
    CHECK_LT((record.lowPrice - 3246.70), 0.00001);
    CHECK_LT((record.closePrice - 3261.61), 0.00001);
    CHECK_LT((record.transAmount - 21912127.0), 0.00001);
    CHECK_LT((record.transCount - 162719306.0), 0.00001);

    /** @arg 增加临时增加返还的Stock的KRecord[99]（最后一个数据）读取*/
    record = stk.getKRecord(99);
    CHECK_EQ(record.datetime, Datetime(201707310000));
    CHECK_LT((record.openPrice - 3252.75), 0.00001);
    CHECK_LT((record.highPrice - 3276.95), 0.00001);
    CHECK_LT((record.lowPrice - 3251.19), 0.00001);
    CHECK_LT((record.closePrice - 3273.03), 0.00001);
    CHECK_LT((record.transAmount - 25352591.70), 0.00001);
    CHECK_LT((record.transCount - 246039440.0), 0.00001);

    /** @arg 使用getStock获取临时加入的Stock */
    stk = sm.getStock("tmptest");
    CHECK_EQ(stk.isNull(), false);
    CHECK_EQ(stk.market(), "TMP");
    CHECK_EQ(stk.code(), "TEST");
    CHECK_EQ(stk.market_code(), "TMPTEST");
    CHECK_EQ(stk.getCount(KQuery::DAY), 100);
    CHECK_EQ(stk.getCount(KQuery::MIN), 24000);

    /** @arg 使用getStock获取临时加入的Stock的KRecord[10]读取*/
    record = stk.getKRecord(10);
    CHECK_EQ(record.datetime, Datetime(201703210000));
    CHECK_LT((record.openPrice - 3250.25), 0.00001);
    CHECK_LT((record.highPrice - 3262.22), 0.00001);
    CHECK_LT((record.lowPrice - 3246.70), 0.00001);
    CHECK_LT((record.closePrice - 3261.61), 0.00001);
    CHECK_LT((record.transAmount - 21912127.0), 0.00001);
    CHECK_LT((record.transCount - 162719306.0), 0.00001);

    /** @arg 删除临时加入的Stock */
    sm.removeTempCsvStock("test");
    stk = sm.getStock("tmptest");
    CHECK_EQ(stk.isNull(), true);
}

/** @par 检测点 */
TEST_CASE("test_StockManager_isHoliday") {
    auto& sm = StockManager::instance();
    CHECK_EQ(sm.isHoliday(Datetime(202101010000LL)), true);
    CHECK_EQ(sm.isHoliday(Datetime(202101020000LL)), false);
    CHECK_EQ(sm.isHoliday(Datetime(202110010000LL)), true);
    CHECK_EQ(sm.isHoliday(Datetime(202109300000LL)), false);
}

/** @} */
