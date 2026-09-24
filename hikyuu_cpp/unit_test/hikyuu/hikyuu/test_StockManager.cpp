/*
 * test_StockManager.cpp
 *
 *  Created on: 2011-11-10
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/utilities/runtimeinfo.h>
#include <hikyuu/utilities/Log.h>
#include "../plugin_valid.h"

using namespace hku;

/**
 * @defgroup test_hikyuu_StockManager test_hikyuu_StockManager
 * @ingroup test_hikyuu_base_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_StockManager_size") {
    StockManager& sm = StockManager::instance();
    /** @arg Check that it matches the security count of the test data */
    CHECK_EQ(sm.size(), 4729);
}

/** @par Test points */
TEST_CASE("test_StockManager_getStock") {
    StockManager& sm = StockManager::instance();
    Stock stock;
    Stock null_stock = Null<Stock>();

    /** @arg Query by "market code" while the market does not exist */
    CHECK_EQ(sm.getStock("z000001"), null_stock);

    /** @arg Query by "market code" while the market exists but the security code does not */
    CHECK_EQ(sm.getStock("sh1000001"), null_stock);

    /** @arg Query an existing stock by "market code" */
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

/** @par Test points */
TEST_CASE("test_StockManager_getMarketInfo") {
    StockManager& sm = StockManager::instance();

    /** @arg Query a market that does not exist */
    CHECK_EQ(Null<MarketInfo>(), sm.getMarketInfo("XXX"));
    CHECK_EQ(Null<MarketInfo>(), sm.getMarketInfo(""));

    /** @arg Query an existing market with an all-uppercase market name */
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

    /** @arg Query an existing market with an all-lowercase market name */
    marketInfo = sm.getMarketInfo("sh");
    CHECK_NE(marketInfo, Null<MarketInfo>());
    CHECK_EQ(marketInfo.market(), "SH");

    /** @arg Query an existing market with a mixed-case market name */
    marketInfo = sm.getMarketInfo("Sh");
    CHECK_NE(marketInfo, Null<MarketInfo>());
    CHECK_EQ(marketInfo.market(), "SH");
}

/** @par Test points */
TEST_CASE("test_StockManager_getStockTypeInfo") {
    StockManager& sm = StockManager::instance();
    StockTypeInfo stockTypeInfo;

    /** @arg Query a type that does not exist */
    CHECK_EQ(sm.getStockTypeInfo(999), Null<StockTypeInfo>());

    /** @arg Query the first record, i.e. type=0 */
    stockTypeInfo = sm.getStockTypeInfo(0);
    CHECK_NE(stockTypeInfo, Null<StockTypeInfo>());
    CHECK_EQ(stockTypeInfo.type(), 0);
    CHECK_EQ(stockTypeInfo.description(), "Block");
    CHECK_EQ(stockTypeInfo.tick(), 100);
    CHECK_EQ(stockTypeInfo.precision(), 2);
    CHECK_EQ(stockTypeInfo.minTradeNumber(), 100);
    CHECK_EQ(stockTypeInfo.maxTradeNumber(), 1000000);

    /** @arg Query the last record, i.e. type=8 */
    stockTypeInfo = sm.getStockTypeInfo(8);
    CHECK_NE(stockTypeInfo, Null<StockTypeInfo>());
    CHECK_EQ(stockTypeInfo.type(), 8);
    CHECK_EQ(stockTypeInfo.description(), "创业板");
    CHECK_EQ(stockTypeInfo.tick(), 0.01);
    CHECK_EQ(stockTypeInfo.precision(), 2);
    CHECK_EQ(stockTypeInfo.minTradeNumber(), 100);
    CHECK_EQ(stockTypeInfo.maxTradeNumber(), 1000000);

    /** @arg Query information that exists normally */
    stockTypeInfo = sm.getStockTypeInfo(2);
    CHECK_NE(stockTypeInfo, Null<StockTypeInfo>());
    CHECK_EQ(stockTypeInfo.type(), 2);
    CHECK_EQ(stockTypeInfo.description(), "指数");
    CHECK_EQ(stockTypeInfo.tick(), 0.001);
    CHECK_EQ(stockTypeInfo.precision(), 3);
    CHECK_EQ(stockTypeInfo.minTradeNumber(), 1);
    CHECK_EQ(stockTypeInfo.maxTradeNumber(), 1000000);
}

/** @par Test points */
TEST_CASE("test_StockManager_getAllMarket") {
    StockManager& sm = StockManager::instance();

    /** @arg Check the Market in the test data */
    StringList result(sm.getAllMarket());
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

/** @par Test points */
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

/** @par Test points */
TEST_CASE("test_StockManager_TempCsvStock") {
    StockManager& sm = StockManager::instance();

    string day_filename(fmt::format("{}/test_day_data.csv", sm.datadir()));
    string min_filename(fmt::format("{}/test_min_data.csv", sm.datadir()));

    /** @arg Check the basic attributes of the temporarily added Stock */
    Stock stk = sm.addTempCsvStock("test", day_filename, min_filename);
    CHECK_EQ(stk.isNull(), false);
    CHECK_EQ(stk.market(), "TMP");
    CHECK_EQ(stk.code(), "TEST");
    CHECK_EQ(stk.market_code(), "TMPTEST");
    CHECK_EQ(stk.getCount(KQuery::DAY), 100);
    CHECK_EQ(stk.getCount(KQuery::MIN), 24000);

    /** @arg Read KRecord[0] (the first record) of the temporarily added Stock */
    KRecord record;
    record = stk.getKRecord(0);
    CHECK_EQ(record.datetime, Datetime(201703070000));
    CHECK_LT((record.openPrice - 3233.09), 0.00001);
    CHECK_LT((record.highPrice - 3242.66), 0.00001);
    CHECK_LT((record.lowPrice - 3226.82), 0.00001);
    CHECK_LT((record.closePrice - 3242.41), 0.00001);
    CHECK_LT((record.transAmount - 20993120.6), 0.00001);
    CHECK_LT((record.transCount - 164064235.0), 0.00001);

    /** @arg Read KRecord[10] (a record in the middle) of the temporarily added Stock */
    record = stk.getKRecord(10);
    CHECK_EQ(record.datetime, Datetime(201703210000));
    CHECK_LT((record.openPrice - 3250.25), 0.00001);
    CHECK_LT((record.highPrice - 3262.22), 0.00001);
    CHECK_LT((record.lowPrice - 3246.70), 0.00001);
    CHECK_LT((record.closePrice - 3261.61), 0.00001);
    CHECK_LT((record.transAmount - 21912127.0), 0.00001);
    CHECK_LT((record.transCount - 162719306.0), 0.00001);

    /** @arg Read KRecord[99] (the last record) of the temporarily added Stock */
    record = stk.getKRecord(99);
    CHECK_EQ(record.datetime, Datetime(201707310000));
    CHECK_LT((record.openPrice - 3252.75), 0.00001);
    CHECK_LT((record.highPrice - 3276.95), 0.00001);
    CHECK_LT((record.lowPrice - 3251.19), 0.00001);
    CHECK_LT((record.closePrice - 3273.03), 0.00001);
    CHECK_LT((record.transAmount - 25352591.70), 0.00001);
    CHECK_LT((record.transCount - 246039440.0), 0.00001);

    /** @arg Get the temporarily added Stock with getStock */
    stk = sm.getStock("tmptest");
    CHECK_EQ(stk.isNull(), false);
    CHECK_EQ(stk.market(), "TMP");
    CHECK_EQ(stk.code(), "TEST");
    CHECK_EQ(stk.market_code(), "TMPTEST");
    CHECK_EQ(stk.getCount(KQuery::DAY), 100);
    CHECK_EQ(stk.getCount(KQuery::MIN), 24000);

    /** @arg Read KRecord[10] of the temporarily added Stock obtained with getStock */
    record = stk.getKRecord(10);
    CHECK_EQ(record.datetime, Datetime(201703210000));
    CHECK_LT((record.openPrice - 3250.25), 0.00001);
    CHECK_LT((record.highPrice - 3262.22), 0.00001);
    CHECK_LT((record.lowPrice - 3246.70), 0.00001);
    CHECK_LT((record.closePrice - 3261.61), 0.00001);
    CHECK_LT((record.transAmount - 21912127.0), 0.00001);
    CHECK_LT((record.transCount - 162719306.0), 0.00001);

    /** @arg Remove the temporarily added Stock */
    sm.removeTempCsvStock("test");
    stk = sm.getStock("tmptest");
    CHECK_EQ(stk.isNull(), true);
}

/** @par Test points */
TEST_CASE("test_StockManager_isHoliday") {
    auto& sm = StockManager::instance();
    CHECK_THROWS(sm.isHoliday(Datetime()));
    CHECK_EQ(sm.isHoliday(Datetime(202101010000LL)), true);
    CHECK_EQ(sm.isHoliday(Datetime(202101020000LL)), true);  // Saturday
    CHECK_EQ(sm.isHoliday(Datetime(202101030000LL)), true);  // Sunday
    CHECK_EQ(sm.isHoliday(Datetime(202110010000LL)), true);
    CHECK_EQ(sm.isHoliday(Datetime(202109300000LL)), false);
}

/** @par Test points */
TEST_CASE("test_StockManager_isTradingHours") {
    auto& sm = StockManager::instance();
    CHECK_THROWS(sm.isTradingHours(Datetime()));
    CHECK_EQ(sm.isTradingHours(Datetime(202101010000LL)), false);
    CHECK_EQ(sm.isTradingHours(Datetime(202101020000LL)), false);  // Saturday
    CHECK_EQ(sm.isTradingHours(Datetime(202101030000LL)), false);  // Sunday
    CHECK_EQ(sm.isTradingHours(Datetime(202110010000LL)), false);
    CHECK_EQ(sm.isTradingHours(Datetime(202109300000LL)), false);
    CHECK_EQ(sm.isTradingHours(Datetime(202109300929LL)), false);
    CHECK_EQ(sm.isTradingHours(Datetime(202109300930LL)), true);
    CHECK_EQ(sm.isTradingHours(Datetime(202109301000LL)), true);
    CHECK_EQ(sm.isTradingHours(Datetime(202109301130LL)), true);
    CHECK_EQ(sm.isTradingHours(Datetime(202109301131LL)), false);
    CHECK_EQ(sm.isTradingHours(Datetime(202109301259LL)), false);
    CHECK_EQ(sm.isTradingHours(Datetime(202109301300LL)), true);
    CHECK_EQ(sm.isTradingHours(Datetime(202109301301LL)), true);
    CHECK_EQ(sm.isTradingHours(Datetime(202109301500LL)), true);
    CHECK_EQ(sm.isTradingHours(Datetime(202109301501LL)), false);
    CHECK_THROWS(sm.isTradingHours(Datetime(202109301500LL), "invalid"));
}

/** @par Test points */
TEST_CASE("test_StockManager_getZhBond10") {
    auto& sm = StockManager::instance();
    const auto& result = sm.getZhBond10();
    CHECK_EQ(result.size(), 5536);
    CHECK_EQ(result[0].date, Datetime(20020104));
    CHECK_EQ(result[0].value, doctest::Approx(3.2096));
    CHECK_EQ(result[10].date, Datetime(20020118));
    CHECK_EQ(result[10].value, doctest::Approx(3.2968));
    CHECK_EQ(result[5535].date, Datetime(20240229));
    CHECK_EQ(result[5535].value, doctest::Approx(2.3375));
}

/** @par Test points */
TEST_CASE("test_StockManager_releaseShmServerBaseInfoCache") {
    HKU_IF_RETURN(!pluginValid(), void());

    auto& sm = StockManager::instance();
    const bool org_role = isShmServerRole();
    Stock stk = sm.getStock("sz000001");
    const size_t weight_cnt = stk.getWeight().size();
    const size_t finance_cnt = stk.getHistoryFinance().size();

    /** @arg The call is a no-op in a non-server role (an ordinary standalone / client process): the
     * cache is unaffected */
    setShmServerRole(false);
    sm.releaseShmServerBaseInfoCache();
    CHECK_EQ(stk.getWeight().size(), weight_cnt);
    CHECK_EQ(stk.getHistoryFinance().size(), finance_cnt);

    /** @arg The server role releases the ex-rights/ex-dividend cache: switching back to the
     * standalone mode (where getWeight has no lazy loading fallback) and querying again returns
     * empty, proving that the cache has really been cleared and returned */
    setShmServerRole(true);
    sm.releaseShmServerBaseInfoCache();
    setShmServerRole(false);
    CHECK_EQ(stk.getWeight().size(), 0);

    /** @arg Accessing a released security again in the server role: the on-demand lazy reload heals
     * it and the result matches the one before the release */
    setShmServerRole(true);
    CHECK_EQ(stk.getWeight().size(), weight_cnt);
    CHECK_EQ(stk.getHistoryFinance().size(), finance_cnt);

    // Restore the state: Stock::getWeight has no lazy loading fallback in the standalone / client
    // mode, so the ex-rights/ex-dividend cache must be refilled before the role is restored,
    // avoiding an emptied state affecting the later cases (the historical finance has a fallback)
    for (const auto& stock : sm.getStockList(nullptr)) {
        stock.getWeight();
    }
    setShmServerRole(org_role);
}

/** @} */
