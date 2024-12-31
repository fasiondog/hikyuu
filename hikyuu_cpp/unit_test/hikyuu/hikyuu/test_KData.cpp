/*
 * test_Stock.cpp
 *
 *  Created on: 2011-12-10
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <cmath>
#include <hikyuu/StockManager.h>
#include <hikyuu/KQuery.h>
#include <hikyuu/KData.h>
#include <hikyuu/Stock.h>

using namespace hku;

/**
 * @defgroup test_hikyuu_KData test_hikyuu_KData
 * @ingroup test_hikyuu_base_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_KData_equal") {
    /** @arg kdata为空 */
    KData null_k = Null<KData>();
    KData k1, k2;
    CHECK_EQ(k1, null_k);
    CHECK_EQ(k1, k2);

    /** @arg KData的query不同，但 stock 为空 */
    Stock stk1, stk2;
    k1 = stk1.getKData(KQueryByIndex(0, 10, KQuery::DAY));
    CHECK_UNARY(stk1.isNull());
    CHECK_NE(k1.getQuery(), null_k.getQuery());
    CHECK_EQ(k1, null_k);

    /** @arg stk相同且非空 KData 比较 */
    stk1 = getStock("sh000001");
    k1 = stk1.getKData(KQueryByIndex(0, 10, KQuery::DAY));
    k2 = k1;
    CHECK_EQ(k1, k2);
    k2 = stk1.getKData(KQueryByIndex(0, 10, KQuery::DAY));
    CHECK_EQ(k1, k2);
    k2 = stk1.getKData(KQueryByIndex(1, 10, KQuery::DAY));
    CHECK_NE(k1, k2);

    /** @arg stk不同且非空 KData 比较 */
    stk2 = getStock("sz000001");
    auto query = KQuery(0, 10, KQuery::DAY);
    CHECK_NE(stk1, stk2);
    k1 = stk1.getKData(query);
    k2 = stk2.getKData(query);
    CHECK_NE(k1, k2);

    /** @arg stk1为空，stk2非空 */
    stk1 = Stock();
    CHECK_UNARY(stk1.isNull());
    CHECK_UNARY(!stk2.isNull());
    k1 = stk1.getKData(query);
    k2 = stk2.getKData(query);
    CHECK_NE(k1, k2);
}

/** @par 检测点 */
TEST_CASE("test_getDatetimeList") {
    StockManager& sm = StockManager::instance();
    Stock stock;
    KData kdata;
    KQuery query;
    DatetimeList result;

    /** @arg kdata为空 */
    result = kdata.getDatetimeList();
    CHECK_UNARY(result.empty());

    /** @arg 正常获取DatetimeList */
    stock = sm.getStock("sh600000");
    query = KQuery(0, 1, KQuery::DAY);
    kdata = stock.getKData(query);
    result = kdata.getDatetimeList();
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.front(), Datetime(199911100000));

    query = KQuery(0, Null<int>(), KQuery::DAY);
    kdata = stock.getKData(query);
    result = kdata.getDatetimeList();
    CHECK_EQ(result.size(), 2836);
    CHECK_EQ(result[0], Datetime(199911100000));
    CHECK_EQ(result[10], Datetime(199911240000));
    CHECK_EQ(result[2835], Datetime(201112060000));

    query = KQuery(1, 10, KQuery::WEEK);
    kdata = stock.getKData(query);
    result = kdata.getDatetimeList();
    CHECK_EQ(result.size(), 9);
    CHECK_EQ(result[0], Datetime(199911190000));
    CHECK_EQ(result[1], Datetime(199911260000));
    CHECK_EQ(result[8], Datetime(200001140000));

    query = KQuery(1, 10, KQuery::MONTH);
    kdata = stock.getKData(query);
    result = kdata.getDatetimeList();
    CHECK_EQ(result.size(), 9);
    CHECK_EQ(result[0], Datetime(199912310000));
    CHECK_EQ(result[1], Datetime(200001310000));
    CHECK_EQ(result[8], Datetime(200008310000));

    query = KQuery(1, 10, KQuery::QUARTER);
    kdata = stock.getKData(query);
    result = kdata.getDatetimeList();
    CHECK_EQ(result.size(), 9);
    CHECK_EQ(result[0], Datetime(200003310000));
    CHECK_EQ(result[1], Datetime(200006300000));
    CHECK_EQ(result[8], Datetime(200203310000));

    query = KQuery(1, 10, KQuery::HALFYEAR);
    kdata = stock.getKData(query);
    result = kdata.getDatetimeList();
    CHECK_EQ(result.size(), 9);
    CHECK_EQ(result[0], Datetime(200006300000));
    CHECK_EQ(result[1], Datetime(200012310000));
    CHECK_EQ(result[8], Datetime(200406300000));

    query = KQuery(1, 10, KQuery::YEAR);
    kdata = stock.getKData(query);
    result = kdata.getDatetimeList();
    CHECK_EQ(result.size(), 9);
    CHECK_EQ(result[0], Datetime(200012310000));
    CHECK_EQ(result[1], Datetime(200112310000));
    CHECK_EQ(result[8], Datetime(200812310000));

    query = KQuery(1, 10, KQuery::MIN);
    kdata = stock.getKData(query);
    result = kdata.getDatetimeList();
    CHECK_EQ(result.size(), 9);
    CHECK_EQ(result[0], Datetime(200001040932));
    CHECK_EQ(result[1], Datetime(200001040933));
    CHECK_EQ(result[8], Datetime(200001040940));

    query = KQuery(1, 10, KQuery::MIN5);
    kdata = stock.getKData(query);
    result = kdata.getDatetimeList();
    CHECK_EQ(result.size(), 9);
    CHECK_EQ(result[0], Datetime(200001040940));
    CHECK_EQ(result[1], Datetime(200001040945));
    CHECK_EQ(result[8], Datetime(200001041020));

    query = KQuery(1, 10, KQuery::MIN15);
    kdata = stock.getKData(query);
    result = kdata.getDatetimeList();
    CHECK_EQ(result.size(), 9);
    CHECK_EQ(result[0], Datetime(200001041000));
    CHECK_EQ(result[1], Datetime(200001041015));
    CHECK_EQ(result[8], Datetime(200001041330));

    query = KQuery(1, 10, KQuery::MIN30);
    kdata = stock.getKData(query);
    result = kdata.getDatetimeList();
    CHECK_EQ(result.size(), 9);
    CHECK_EQ(result[0], Datetime(200001041030));
    CHECK_EQ(result[1], Datetime(200001041100));
    CHECK_EQ(result[8], Datetime(200001051030));

    query = KQuery(1, 10, KQuery::MIN60);
    kdata = stock.getKData(query);
    result = kdata.getDatetimeList();
    CHECK_EQ(result.size(), 9);
    CHECK_EQ(result[0], Datetime(200001041130));
    CHECK_EQ(result[1], Datetime(200001041400));
    CHECK_EQ(result[8], Datetime(200001061130));
}

/** @par 检测点 */
TEST_CASE("test_getKData_by_index") {
    StockManager& sm = StockManager::instance();
    Stock stock;
    KData kdata;
    KRecord record, expect;
    KQuery query;
    size_t total;

    /** @arg 查询未设定数据读取器的Stock的数据 */
    kdata = stock.getKData(KQuery());
    CHECK_EQ(kdata.getStock(), Null<Stock>());
    CHECK_EQ(kdata.size(), 0);
    CHECK_EQ(kdata.empty(), true);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.endPos(), 0);
    CHECK_EQ(kdata.lastPos(), 0);

    ///==============================
    /// 测试日线
    ///==============================
    /** @arg SH000001全部K线日线数据, KQuery(0) */
    stock = sm.getStock("sh000001");
    query = KQuery(0);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.getStock(), stock);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.size(), 5121);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.lastPos(), 5120);
    CHECK_EQ(kdata.endPos(), 5121);
    record = kdata[0];
    expect = KRecord(Datetime(199012190000), 96.05, 99.98, 95.79, 99.98, 49.4, 1260);
    CHECK_EQ(record, expect);

    record = kdata[kdata.size() - 1];
    expect =
      KRecord(Datetime(201112060000), 2326.66, 2331.892, 2310.155, 2325.905, 4262559.5, 45917078);
    CHECK_EQ(record, expect);

    /** @arg SH000001日线数据，KQuery(total-1) */
    total = stock.getCount(KQuery::DAY);
    query = KQuery(total - 1);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.size(), 1);
    CHECK_EQ(kdata.startPos(), 5120);
    CHECK_EQ(kdata.endPos(), 5121);
    CHECK_EQ(kdata.lastPos(), 5120);
    record = kdata[0];
    expect =
      KRecord(Datetime(201112060000), 2326.66, 2331.892, 2310.155, 2325.905, 4262559.5, 45917078);
    CHECK_EQ(record, expect);

    /** @arg SH000001日线数据，KQuery(total) */
    total = stock.getCount(KQuery::DAY);
    query = KQuery(total);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.getStock(), stock);
    CHECK_EQ(kdata.size(), 0);
    CHECK_EQ(kdata.empty(), true);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.lastPos(), 0);
    CHECK_EQ(kdata.endPos(), 0);

    /** @arg SH000001日线数据，KQuery(0,1) */
    query = KQuery(0, 1);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 1);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.endPos(), 1);
    CHECK_EQ(kdata.lastPos(), 0);
    record = kdata[0];
    expect = KRecord(Datetime(199012190000), 96.05, 99.98, 95.79, 99.98, 49.4, 1260);
    CHECK_EQ(record, expect);

    /** @arg SH000001日线数据，KQuery(1,2) */
    query = KQuery(1, 2);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 1);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 1);
    CHECK_EQ(kdata.endPos(), 2);
    CHECK_EQ(kdata.lastPos(), 1);
    record = kdata[0];
    expect = KRecord(Datetime(199012200000), 104.3, 104.39, 99.98, 104.39, 8.4, 197);
    CHECK_EQ(record, expect);

    /** @arg SH000001日线数据，KQuery(1,10) */
    query = KQuery(1, 10);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 9);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 1);
    CHECK_EQ(kdata.endPos(), 10);
    CHECK_EQ(kdata.lastPos(), 9);
    record = kdata[0];
    expect = KRecord(Datetime(199012200000), 104.3, 104.39, 99.98, 104.39, 8.4, 197);
    CHECK_EQ(record, expect);

    record = kdata[kdata.size() - 1];
    expect = KRecord(Datetime(199101020000), 127.61, 128.84, 127.61, 128.84, 5.9, 91);
    CHECK_EQ(record, expect);

    /** @arg SH000001日线数据，KQuery(total-1,total) */
    total = stock.getCount(KQuery::DAY);
    query = KQuery(total - 1, total);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 1);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), total - 1);
    CHECK_EQ(kdata.endPos(), total);
    CHECK_EQ(kdata.lastPos(), total - 1);
    record = kdata[0];
    expect =
      KRecord(Datetime(201112060000), 2326.66, 2331.892, 2310.155, 2325.905, 4262559.5, 45917078);
    CHECK_EQ(record, expect);

    /** @arg SH000001日线数据, KQuery(0,0) */
    query = KQuery(0, 0);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 0);
    CHECK_EQ(kdata.empty(), true);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.endPos(), 0);
    CHECK_EQ(kdata.lastPos(), 0);

    /** @arg SH000001日线数据, KQuery(1,1) */
    query = KQuery(1, 1);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 0);
    CHECK_EQ(kdata.empty(), true);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.endPos(), 0);
    CHECK_EQ(kdata.lastPos(), 0);

    /** @arg SH000001日线数据, KQuery(total-1,total-1) */
    total = stock.getCount();
    query = KQuery(total - 1, total - 1);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 0);
    CHECK_EQ(kdata.empty(), true);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.endPos(), 0);
    CHECK_EQ(kdata.lastPos(), 0);

    /** @arg SH000001日线数据，KQuery(-1) */
    query = KQuery(-1);
    total = stock.getCount();
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 1);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), total - 1);
    CHECK_EQ(kdata.endPos(), total);
    CHECK_EQ(kdata.lastPos(), total - 1);
    record = kdata[0];
    expect =
      KRecord(Datetime(201112060000), 2326.66, 2331.892, 2310.155, 2325.905, 4262559.5, 45917078);
    CHECK_EQ(record, expect);

    /** @arg SH000001日线数据，KQuery(-total) */
    total = stock.getCount();
    query = KQuery(-(int64_t)total);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 5121);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.endPos(), total);
    CHECK_EQ(kdata.lastPos(), total - 1);
    record = kdata[0];
    expect = KRecord(Datetime(199012190000), 96.05, 99.98, 95.79, 99.98, 49.4, 1260);
    CHECK_EQ(record, expect);

    record = kdata[kdata.size() - 1];
    expect =
      KRecord(Datetime(201112060000), 2326.66, 2331.892, 2310.155, 2325.905, 4262559.5, 45917078);
    CHECK_EQ(record, expect);

    /** @arg SH000001日线数据，KQuery(-total-1) */
    total = stock.getCount();
    query = KQuery(-1 - (int64_t)total);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 5121);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.endPos(), total);
    CHECK_EQ(kdata.lastPos(), total - 1);
    record = kdata[0];
    expect = KRecord(Datetime(199012190000), 96.05, 99.98, 95.79, 99.98, 49.4, 1260);
    CHECK_EQ(record, expect);

    record = kdata[kdata.size() - 1];
    expect =
      KRecord(Datetime(201112060000), 2326.66, 2331.892, 2310.155, 2325.905, 4262559.5, 45917078);
    CHECK_EQ(record, expect);

    /** @arg SH000001日线数据，KQuery(-total + 1) */
    total = stock.getCount();
    query = KQuery(1 - (int64_t)total);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 5120);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 1);
    CHECK_EQ(kdata.endPos(), total);
    CHECK_EQ(kdata.lastPos(), total - 1);
    record = kdata[0];
    expect = KRecord(Datetime(199012200000), 104.3, 104.39, 99.98, 104.39, 8.4, 197);
    CHECK_EQ(record, expect);

    record = kdata[kdata.size() - 1];
    expect =
      KRecord(Datetime(201112060000), 2326.66, 2331.892, 2310.155, 2325.905, 4262559.5, 45917078);
    CHECK_EQ(record, expect);

    /** @arg SH000001日线数据，KQuery(-2, -1) */
    total = stock.getCount();
    query = KQuery(-2, -1);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 1);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), total - 2);
    CHECK_EQ(kdata.endPos(), total - 1);
    CHECK_EQ(kdata.lastPos(), total - 2);
    record = kdata[0];
    expect =
      KRecord(Datetime(201112050000), 2363.111, 2363.127, 2327.61, 2333.229, 4864121.6, 52214970);
    CHECK_EQ(record, expect);

    /** @arg SH000001日线数据，KQuery(-10, -2) */
    query = KQuery(-10, -2);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 8);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), total - 10);
    CHECK_EQ(kdata.endPos(), total - 2);
    CHECK_EQ(kdata.lastPos(), total - 2 - 1);
    record = kdata[0];
    expect =
      KRecord(Datetime(201111230000), 2415.197, 2418.56, 2390.654, 2395.065, 5004336, 50547851);
    CHECK_EQ(record, expect);

    record = kdata[kdata.size() - 1];
    expect =
      KRecord(Datetime(201112020000), 2374.899, 2378.299, 2344.846, 2360.664, 5732015.7, 59868846);
    CHECK_EQ(record, expect);

    /** @arg SH000001日线数据，KQuery(-1, 1) */
    query = KQuery(-1, 1);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 0);
    CHECK_EQ(kdata.empty(), true);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.endPos(), 0);
    CHECK_EQ(kdata.lastPos(), 0);

    /** @arg SH000001日线数据，KQuery(-total, 1) */
    query = KQuery(-(int64_t)total, 1);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 1);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.endPos(), 1);
    CHECK_EQ(kdata.lastPos(), 0);
    record = kdata[0];
    expect = KRecord(Datetime(199012190000), 96.05, 99.98, 95.79, 99.980, 49.4, 1260);
    CHECK_EQ(record, expect);

    /** @arg SH000001日线数据，KQuery(-total, 2) */
    query = KQuery(-(int64_t)total, 2);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 2);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.endPos(), 2);
    CHECK_EQ(kdata.lastPos(), 1);
    record = kdata[0];
    expect = KRecord(Datetime(199012190000), 96.05, 99.98, 95.79, 99.980, 49.4, 1260);
    CHECK_EQ(record, expect);

    record = kdata[kdata.size() - 1];
    CHECK_EQ(record.datetime, Datetime(199012200000));

    /** @arg SH000001日线数据，KQuery(0, -total) */
    query = KQuery(0, -(int64_t)total);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 0);
    CHECK_EQ(kdata.empty(), true);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.endPos(), 0);
    CHECK_EQ(kdata.lastPos(), 0);

    /** @arg SH000001日线数据，KQuery(1, -1) */
    query = KQuery(1, -1);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 5119);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 1);
    CHECK_EQ(kdata.endPos(), total - 1);
    CHECK_EQ(kdata.lastPos(), total - 2);
    record = kdata[0];
    expect = KRecord(Datetime(199012200000), 104.3, 104.39, 99.98, 104.39, 8.4, 197);
    CHECK_EQ(record, expect);

    record = kdata[kdata.size() - 1];
    expect =
      KRecord(Datetime(201112050000), 2363.111, 2363.127, 2327.61, 2333.229, 4864121.6, 52214970);
    CHECK_EQ(record, expect);

    ///==============================
    /// 测试分钟线
    ///==============================
    /** @arg SH000001全部1分钟K线数据,KQuery(0) */
    query = KQuery(0, Null<int64_t>(), KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 682823L);
    record = kdata[0];
    expect =
      KRecord(Datetime(200001040931), 1366.58, 1368.692, 1366.579, 1368.692, 4124880, 191158);
    CHECK_EQ(record, expect);

    record = kdata[kdata.size() - 1];
    expect = KRecord(Datetime(201112061500), 2325.38, 2326.1, 2324.6, 2325.90, 5215150, 566312);
    CHECK_EQ(record, expect);

    /** @arg SH000001分钟线数据，KQuery(total-1) */
    total = stock.getCount(KQuery::MIN);
    query = KQuery(total - 1, Null<int64_t>(), KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 1);
    record = kdata[0];
    expect = KRecord(Datetime(201112061500), 2325.38, 2326.1, 2324.6, 2325.90, 5215150, 566312);
    CHECK_EQ(record, expect);

    /** @arg SH000001分钟线数据，KQuery(total) */
    total = stock.getCount(KQuery::MIN);
    query = KQuery(total, Null<int64_t>(), KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 0);

    /** @arg SH000001分钟线数据，KQuery(0,1) */
    query = KQuery(0, 1, KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 1);
    record = kdata[0];
    expect =
      KRecord(Datetime(200001040931), 1366.58, 1368.692, 1366.579, 1368.692, 4124880, 191158);
    CHECK_EQ(record, expect);

    /** @arg SH000001分钟线数据，KQuery(1,2) */
    query = KQuery(1, 2, KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 1);
    record = kdata[0];
    CHECK_EQ(record.datetime, Datetime(200001040932));

    /** @arg SH000001分钟线数据，KQuery(1,10) */
    query = KQuery(1, 10, KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 9);
    record = kdata[0];
    CHECK_EQ(record.datetime, Datetime(200001040932));

    record = kdata[kdata.size() - 1];
    CHECK_EQ(record.datetime, Datetime(200001040940));

    /** @arg SH000001分钟线数据，KQuery(total-1,total) */
    total = stock.getCount(KQuery::MIN);
    query = KQuery(total - 1, total, KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 1);
    record = kdata[0];
    CHECK_EQ(record.datetime, Datetime(201112061500));

    /** @arg SH000001分钟线数据, KQuery(0,0) */
    query = KQuery(0, 0, KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 0);

    /** @arg SH000001分钟线数据, KQuery(1,1) */
    query = KQuery(1, 1, KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 0);

    /** @arg SH000001分钟线数据, KQuery(total-1,total-1) */
    total = stock.getCount(KQuery::MIN);
    query = KQuery(total - 1, total - 1, KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 0);

    /** @arg SH000001分钟线数据，KQuery(-1) */
    query = KQuery(-1, Null<int64_t>(), KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 1);
    record = kdata[0];
    CHECK_EQ(record.datetime, Datetime(201112061500));

    /** @arg SH000001分钟线数据，KQuery(-total) */
    total = stock.getCount(KQuery::MIN);
    query = KQuery(-(int64_t)total, Null<int64_t>(), KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 682823);
    record = kdata[0];
    CHECK_EQ(record.datetime, Datetime(200001040931));

    record = kdata[kdata.size() - 1];
    CHECK_EQ(record.datetime, Datetime(201112061500));

    /** @arg SH000001分钟线数据，KQuery(-total-1) */
    total = stock.getCount(KQuery::MIN);
    query = KQuery(-1 - (int64_t)total, Null<int64_t>(), KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 682823);
    record = kdata[0];
    CHECK_EQ(record.datetime, Datetime(200001040931));

    record = kdata[kdata.size() - 1];
    CHECK_EQ(record.datetime, Datetime(201112061500));

    /** @arg SH000001分钟线数据，KQuery(-total + 1) */
    total = stock.getCount(KQuery::MIN);
    query = KQuery(1 - (int64_t)total, Null<int64_t>(), KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 682822);
    record = kdata[0];
    CHECK_EQ(record.datetime, Datetime(200001040932));

    record = kdata[kdata.size() - 1];
    CHECK_EQ(record.datetime, Datetime(201112061500));

    /** @arg SH000001分钟线数据，KQuery(-2, -1) */
    query = KQuery(-2, -1, KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 1);
    record = kdata[0];
    CHECK_EQ(record.datetime, Datetime(201112061459));

    /** @arg SH000001分钟线数据，KQuery(-10, -2) */
    query = KQuery(-10, -2, KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 8);
    record = kdata[0];
    CHECK_EQ(record.datetime, Datetime(201112061451));

    record = kdata[kdata.size() - 1];
    CHECK_EQ(record.datetime, Datetime(201112061458));

    /** @arg SH000001分钟线数据，KQuery(-1, 1) */
    query = KQuery(-1, 1, KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 0);

    /** @arg SH000001分钟线数据，KQuery(-total, 1) */
    query = KQuery(-(int64_t)total, 1, KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 1);
    record = kdata[0];
    CHECK_EQ(record.datetime, Datetime(200001040931));

    /** @arg SH000001分钟线数据，KQuery(-total, 2) */
    query = KQuery(-(int64_t)total, 2, KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 2);
    record = kdata[0];
    CHECK_EQ(record.datetime, Datetime(200001040931));

    record = kdata[kdata.size() - 1];
    CHECK_EQ(record.datetime, Datetime(200001040932));

    /** @arg SH000001分钟线数据，KQuery(0, -total) */
    query = KQuery(0, -(int64_t)total, KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 0);

    /** @arg SH000001分钟线数据，KQuery(1, -1) */
    query = KQuery(1, -1, KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 682821);
    record = kdata[0];
    CHECK_EQ(record.datetime, Datetime(200001040932));

    record = kdata[kdata.size() - 1];
    CHECK_EQ(record.datetime, Datetime(201112061459));

    /** @arg 测试周线 */
    query = KQuery(1, 3, KQuery::WEEK);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 2);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 1);
    CHECK_EQ(kdata.endPos(), 3);
    CHECK_EQ(kdata.lastPos(), 2);
    CHECK_EQ(kdata[0], KRecord(Datetime(199012280000), 113.57, 126.45, 109.13, 126.45, 28.2, 321));
    CHECK_EQ(kdata[1], KRecord(Datetime(199101040000), 126.56, 131.44, 126.48, 131.44, 47.3, 730));

    /** @arg 测试月线 */
    query = KQuery(1, 4, KQuery::MONTH);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 3);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 1);
    CHECK_EQ(kdata.endPos(), 4);
    CHECK_EQ(kdata.lastPos(), 3);
    CHECK_EQ(kdata[0],
             KRecord(Datetime(199101310000), 127.61, 135.19, 127.61, 129.97, 3637.1, 67197));
    CHECK_EQ(kdata[1],
             KRecord(Datetime(199102280000), 129.5, 134.87, 128.06, 133.01, 3027.3, 50982));
    CHECK_EQ(kdata[2],
             KRecord(Datetime(199103310000), 132.53, 132.53, 120.11, 120.19, 1725.3, 24528));

    /** @arg 测试季线 */
    query = KQuery(1, 4, KQuery::QUARTER);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 3);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 1);
    CHECK_EQ(kdata.endPos(), 4);
    CHECK_EQ(kdata.lastPos(), 3);
    CHECK_EQ(kdata[0],
             KRecord(Datetime(199103310000), 127.61, 135.19, 120.11, 120.19, 8389.7, 142707));
    CHECK_EQ(kdata[1],
             KRecord(Datetime(199106300000), 120.69, 137.56, 104.96, 137.56, 12095.6, 222753));
    CHECK_EQ(kdata[2],
             KRecord(Datetime(199109300000), 136.64, 191.18, 131.87, 180.92, 32436.9, 527079));

    /** @arg 测试5分钟线 */
    query = KQuery(1, 4, KQuery::MIN5);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 3);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 1);
    CHECK_EQ(kdata.endPos(), 4);
    CHECK_EQ(kdata.lastPos(), 3);
    CHECK_EQ(kdata[0], KRecord(Datetime(200001040940), 1369.187, 1369.371, 1367.389, 1367.389,
                               70687.3, 251473));
    CHECK_EQ(kdata[1], KRecord(Datetime(200001040945), 1367.389, 1367.389, 1364.645, 1364.763,
                               73577.1, 223346));
    CHECK_EQ(kdata[2], KRecord(Datetime(200001040950), 1364.763, 1364.763, 1363.128, 1363.192,
                               65066.3, 167971));

    /** @arg 测试15分钟线 */
    query = KQuery(1, 4, KQuery::MIN15);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 3);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 1);
    CHECK_EQ(kdata.endPos(), 4);
    CHECK_EQ(kdata.lastPos(), 3);
    CHECK_EQ(kdata[0], KRecord(Datetime(200001041000), 1364.7630, 1364.7630, 1361.4590, 1361.4590,
                               204277.1000, 457452.0000));
    CHECK_EQ(kdata[1], KRecord(Datetime(200001041015), 1361.5580, 1366.0930, 1361.2950, 1365.9270,
                               307531.8000, 548313.0000));
    CHECK_EQ(kdata[2], KRecord(Datetime(200001041030), 1366.0430, 1367.6830, 1365.9460, 1367.6830,
                               279102.7000, 451981.0000));
}

/** @par 检测点 */
TEST_CASE("test_getKData_by_date") {
    StockManager& sm = StockManager::instance();
    Stock stock;
    KData kdata;
    KRecord record, expect;
    KQuery query;
    size_t total;

    ///===================================
    /// 测试日线
    ///===================================

    /** @arg SH000001全部日线数据, KQueryByDate() */
    stock = sm.getStock("sh000001");
    total = stock.getCount();
    query = KQueryByDate();
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), total);
    CHECK_EQ(kdata.size(), 5121);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.endPos(), total);
    CHECK_EQ(kdata.lastPos(), total - 1);
    record = kdata[0];
    expect = KRecord(Datetime(199012190000), 96.05, 99.98, 95.79, 99.98, 49.4, 1260);
    CHECK_EQ(record, expect);

    record = kdata[1];
    expect = KRecord(Datetime(199012200000), 104.3, 104.39, 99.98, 104.39, 8.4, 197);
    CHECK_EQ(record, expect);

    record = kdata[kdata.size() - 1];
    expect =
      KRecord(Datetime(201112060000), 2326.66, 2331.892, 2310.155, 2325.905, 4262559.5, 45917078);
    CHECK_EQ(record, expect);

    /** @arg SH000001日线数据, 起始时间为第一条记录日期的前一天 */
    stock = sm.getStock("sh000001");
    query = KQueryByDate(Datetime(199012180000));
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 5121);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.endPos(), total);
    CHECK_EQ(kdata.lastPos(), total - 1);
    record = kdata[0];
    expect = KRecord(Datetime(199012190000), 96.05, 99.98, 95.79, 99.98, 49.4, 1260);
    CHECK_EQ(record, expect);

    record = kdata[kdata.size() - 1];
    expect =
      KRecord(Datetime(201112060000), 2326.66, 2331.892, 2310.155, 2325.905, 4262559.5, 45917078);
    CHECK_EQ(record, expect);

    /** @arg SH000001日线数据, 起始时间等于第一条记录日期 */
    stock = sm.getStock("sh000001");
    query = KQueryByDate(Datetime(199012190000));
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 5121);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.endPos(), total);
    CHECK_EQ(kdata.lastPos(), total - 1);
    record = kdata[0];
    expect = KRecord(Datetime(199012190000), 96.05, 99.98, 95.79, 99.98, 49.4, 1260);
    CHECK_EQ(record, expect);

    record = kdata[kdata.size() - 1];
    expect =
      KRecord(Datetime(201112060000), 2326.66, 2331.892, 2310.155, 2325.905, 4262559.5, 45917078);
    CHECK_EQ(record, expect);

    /** @arg SH000001日线数据, 起始时间等于第一条记录日期的后一天 */
    stock = sm.getStock("sh000001");
    query = KQueryByDate(Datetime(199012200000));
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 5120);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 1);
    CHECK_EQ(kdata.endPos(), total);
    CHECK_EQ(kdata.lastPos(), total - 1);
    record = kdata[0];
    CHECK_EQ(record.datetime, Datetime(199012200000));

    record = kdata[kdata.size() - 1];
    CHECK_EQ(record.datetime, Datetime(201112060000));

    /** @arg SH000001日线数据, 指定起始时间为第一条记录，截至日期为最后一条记录的前一天 */
    stock = sm.getStock("sh000001");
    query = KQueryByDate(Datetime(199012190000), Datetime(201112050000));
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 5119);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.endPos(), total - 2);
    CHECK_EQ(kdata.lastPos(), total - 3);
    record = kdata[0];
    expect = KRecord(Datetime(199012190000), 96.05, 99.98, 95.79, 99.98, 49.4, 1260);
    CHECK_EQ(record, expect);

    record = kdata[kdata.size() - 1];
    CHECK_EQ(record, KRecord(Datetime(201112020000), 2374.8990, 2378.2990, 2344.8460, 2360.6640,
                             5732015.7000, 59868846.0000));

    /** @arg SH000001日线数据, 指定起始时间为第一条记录，截至日期为最后一条记录 */
    stock = sm.getStock("sh000001");
    query = KQueryByDate(Datetime(199012190000), Datetime(201112060000));
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 5120);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.endPos(), total - 1);
    CHECK_EQ(kdata.lastPos(), total - 2);
    record = kdata[0];
    expect = KRecord(Datetime(199012190000), 96.05, 99.98, 95.79, 99.98, 49.4, 1260);
    CHECK_EQ(record, expect);

    record = kdata[kdata.size() - 1];
    CHECK_EQ(record, KRecord(Datetime(201112050000), 2363.1110, 2363.1270, 2327.6100, 2333.2290,
                             4864121.6000, 52214970.0000));

    /** @arg SH000001日线数据, 指定起始时间为第一条记录，截至日期为最后一条记录的后一天 */
    stock = sm.getStock("sh000001");
    query = KQueryByDate(Datetime(199012190000), Datetime(201112070000));
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 5121);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.endPos(), total);
    CHECK_EQ(kdata.lastPos(), total - 1);
    record = kdata[0];
    expect = KRecord(Datetime(199012190000), 96.05, 99.98, 95.79, 99.98, 49.4, 1260);
    CHECK_EQ(record, expect);

    record = kdata[kdata.size() - 1];
    CHECK_EQ(record, KRecord(Datetime(201112060000), 2326.6600, 2331.8920, 2310.1550, 2325.9050,
                             4262559.5000, 45917078.0000));

    /** @arg SH000001日线数据, 中间任意一段日期 */
    stock = sm.getStock("sh000001");
    query = KQueryByDate(Datetime(199510180000), Datetime(199510240000));
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 4);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 1210);
    CHECK_EQ(kdata.endPos(), 1214);
    CHECK_EQ(kdata.lastPos(), 1213);
    record = kdata[0];
    CHECK_EQ(record, KRecord(Datetime(199510180000), 705.7200, 716.8300, 705.7200, 716.8300,
                             232685.5000, 4273054.0000));

    record = kdata[kdata.size() - 1];
    CHECK_EQ(record, KRecord(Datetime(199510230000), 729.7600, 729.9700, 718.4400, 723.0900,
                             235482.7000, 3877994.0000));

    /** @arg SH000001日线数据, 起始日期等于结束日期 */
    stock = sm.getStock("sh000001");
    query = KQueryByDate(Datetime(199001010000), Datetime(199001010000));
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 0);
    CHECK_EQ(kdata.empty(), true);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.endPos(), 0);
    CHECK_EQ(kdata.lastPos(), 0);

    query = KQueryByDate(Datetime(199012190000), Datetime(199012190000));
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 0);
    CHECK_EQ(kdata.empty(), true);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.endPos(), 0);
    CHECK_EQ(kdata.lastPos(), 0);

    query = KQueryByDate(Datetime(199510240000), Datetime(199510240000));
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 0);
    CHECK_EQ(kdata.empty(), true);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.endPos(), 0);
    CHECK_EQ(kdata.lastPos(), 0);

    query = KQueryByDate(Datetime(201112060000), Datetime(201112060000));
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 0);
    CHECK_EQ(kdata.empty(), true);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.endPos(), 0);
    CHECK_EQ(kdata.lastPos(), 0);

    /** @arg SH000001日线数据, 结束日期等于起始日期的后一天，且数据存在 */
    stock = sm.getStock("sh000001");
    query = KQueryByDate(Datetime(199012190000), Datetime(199012200000));
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 1);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.endPos(), 1);
    CHECK_EQ(kdata.lastPos(), 0);
    record = kdata[0];
    CHECK_EQ(record, KRecord(Datetime(199012190000), 96.0500, 99.9800, 95.7900, 99.9800, 49.4000,
                             1260.0000));

    query = KQueryByDate(Datetime(201111140000), Datetime(201111150000));
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 1);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 5104);
    CHECK_EQ(kdata.endPos(), 5105);
    CHECK_EQ(kdata.lastPos(), 5104);
    record = kdata[0];
    CHECK_EQ(record, KRecord(Datetime(201111140000), 2498.6720, 2529.6320, 2496.3350, 2528.7140,
                             8252378.1000, 81229813.0000));

    query = KQueryByDate(Datetime(201112060000), Datetime(201112070000));
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 1);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 5120);
    CHECK_EQ(kdata.endPos(), 5121);
    CHECK_EQ(kdata.lastPos(), 5120);
    record = kdata[0];
    CHECK_EQ(record, KRecord(Datetime(201112060000), 2326.6600, 2331.8920, 2310.1550, 2325.9050,
                             4262559.5000, 45917078.0000));

    ///===================================
    /// 测试周线
    ///===================================
    /** @arg 起始时间为第一条记录日期的前一天 */
    total = stock.getCount(KQuery::WEEK);
    query = KQueryByDate(Datetime(199012200000), Null<Datetime>(), KQuery::WEEK);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), total);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.endPos(), total);
    CHECK_EQ(kdata.lastPos(), total - 1);
    CHECK_EQ(kdata[0], KRecord(Datetime(199012210000), 96.0500, 109.1300, 95.7900, 109.1300,
                               59.4000, 1485.0000));
    CHECK_EQ(kdata[1], KRecord(Datetime(199012280000), 113.5700, 126.4500, 109.1300, 126.4500,
                               28.2000, 321.0000));
    CHECK_EQ(kdata[kdata.size() - 1], KRecord(Datetime(201112090000), 2363.1110, 2363.1270,
                                              2310.1550, 2325.9050, 9126681.1000, 98132048.0000));

    /** @arg 起始时间等于第一条记录日期 */
    stock = sm.getStock("sh000001");
    query = KQueryByDate(Datetime(199012210000), Null<Datetime>(), KQuery::WEEK);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), total);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.endPos(), total);
    CHECK_EQ(kdata.lastPos(), total - 1);
    CHECK_EQ(kdata[0], KRecord(Datetime(199012210000), 96.0500, 109.1300, 95.7900, 109.1300,
                               59.4000, 1485.0000));
    CHECK_EQ(kdata[10], KRecord(Datetime(199103010000), 134.3700, 134.8700, 132.4700, 132.5300,
                                827.1000, 11500.0000));
    CHECK_EQ(kdata[kdata.size() - 1], KRecord(Datetime(201112090000), 2363.1110, 2363.1270,
                                              2310.1550, 2325.9050, 9126681.1000, 98132048.0000));

    /** @arg 起始时间等于第一条记录日期的后一天 */
    stock = sm.getStock("sh000001");
    query = KQueryByDate(Datetime(199012220000), Null<Datetime>(), KQuery::WEEK);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), total - 1);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 1);
    CHECK_EQ(kdata.endPos(), total);
    CHECK_EQ(kdata.lastPos(), total - 1);
    CHECK_EQ(kdata[0], KRecord(Datetime(199012280000), 113.5700, 126.4500, 109.1300, 126.4500,
                               28.2000, 321.0000));
    CHECK_EQ(kdata[1], KRecord(Datetime(199101040000), 126.5600, 131.4400, 126.4800, 131.4400,
                               47.3000, 730.0000));
    CHECK_EQ(kdata[kdata.size() - 1], KRecord(Datetime(201112090000), 2363.1110, 2363.1270,
                                              2310.1550, 2325.9050, 9126681.1000, 98132048.0000));

    /** @arg 指定起始时间为第一条记录，截至日期为最后一条记录的前一天且不等于前一条记录的日期 */
    stock = sm.getStock("sh000001");
    query = KQueryByDate(Datetime(199012210000), Datetime(201112080000), KQuery::WEEK);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), total - 1);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.endPos(), total - 1);
    CHECK_EQ(kdata.lastPos(), total - 2);
    CHECK_EQ(kdata[0], KRecord(Datetime(199012210000), 96.0500, 109.1300, 95.7900, 109.1300,
                               59.4000, 1485.0000));
    CHECK_EQ(kdata[kdata.size() - 1], KRecord(Datetime(201112020000), 2383.8930, 2423.5590,
                                              2319.4400, 2360.6640, 32821965.5000, 336317856.0000));

    /** @arg 指定起始时间为第一条记录，截至日期为最后一条记录 */
    query = KQueryByDate(Datetime(199012210000), Datetime(201112090000), KQuery::WEEK);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), total - 1);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.endPos(), total - 1);
    CHECK_EQ(kdata.lastPos(), total - 2);
    CHECK_EQ(kdata[0], KRecord(Datetime(199012210000), 96.0500, 109.1300, 95.7900, 109.1300,
                               59.4000, 1485.0000));
    CHECK_EQ(kdata[kdata.size() - 1], KRecord(Datetime(201112020000), 2383.8930, 2423.5590,
                                              2319.4400, 2360.6640, 32821965.5000, 336317856.0000));

    /** @arg 指定起始时间为第一条记录，截至日期为最后一条记录的后一天 */
    query = KQueryByDate(Datetime(199012210000), Datetime(201112100000), KQuery::WEEK);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), total);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.endPos(), total);
    CHECK_EQ(kdata.lastPos(), total - 1);
    CHECK_EQ(kdata[0], KRecord(Datetime(199012210000), 96.0500, 109.1300, 95.7900, 109.1300,
                               59.4000, 1485.0000));
    CHECK_EQ(kdata[kdata.size() - 1], KRecord(Datetime(201112090000), 2363.1110, 2363.1270,
                                              2310.1550, 2325.9050, 9126681.1000, 98132048.0000));

    /** @arg 中间任意一段日期 */
    stock = sm.getStock("sh000001");
    query = KQueryByDate(Datetime(201108150000), Datetime(201111070000), KQuery::WEEK);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 11);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 1043);
    CHECK_EQ(kdata.endPos(), 1054);
    CHECK_EQ(kdata.lastPos(), 1053);
    CHECK_EQ(kdata[0], KRecord(Datetime(201108190000), 2598.1380, 2636.3600, 2513.7540, 2534.3580,
                               40330572.2000, 385608932.0000));
    CHECK_EQ(kdata[kdata.size() - 1], KRecord(Datetime(201111040000), 2470.2540, 2536.7790,
                                              2433.7150, 2528.2940, 50520578.9000, 516983184.0000));

    /** @arg 起始日期等于结束日期 */
    stock = sm.getStock("sh000001");
    query = KQueryByDate(Datetime(199012170000), Datetime(199012170000), KQuery::WEEK);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 0);
    CHECK_EQ(kdata.empty(), true);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.endPos(), 0);
    CHECK_EQ(kdata.lastPos(), 0);

    query = KQueryByDate(Datetime(199104220000), Datetime(199104220000), KQuery::WEEK);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 0);
    CHECK_EQ(kdata.empty(), true);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.endPos(), 0);
    CHECK_EQ(kdata.lastPos(), 0);

    query = KQueryByDate(Datetime(199507170000), Datetime(199507170000), KQuery::WEEK);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 0);
    CHECK_EQ(kdata.empty(), true);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.endPos(), 0);
    CHECK_EQ(kdata.lastPos(), 0);

    query = KQueryByDate(Datetime(201112050000), Datetime(201112050000), KQuery::WEEK);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 0);
    CHECK_EQ(kdata.empty(), true);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.endPos(), 0);
    CHECK_EQ(kdata.lastPos(), 0);

    /** @arg 结束日期等于起始日期之后的第一条记录 */
    stock = sm.getStock("sh000001");
    query = KQueryByDate(Datetime(199012210000), Datetime(199012280000), KQuery::WEEK);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 1);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 0);
    CHECK_EQ(kdata.endPos(), 1);
    CHECK_EQ(kdata.lastPos(), 0);
    CHECK_EQ(kdata[0], KRecord(Datetime(199012210000), 96.0500, 109.1300, 95.7900, 109.1300,
                               59.4000, 1485.0000));

    query = KQueryByDate(Datetime(199707250000), Datetime(199708010000), KQuery::WEEK);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 1);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 338);
    CHECK_EQ(kdata.endPos(), 339);
    CHECK_EQ(kdata.lastPos(), 338);
    CHECK_EQ(kdata[0], KRecord(Datetime(199707250000), 1215.8390, 1223.8230, 1158.1360, 1170.8620,
                               2433586.7000, 20739544.0000));

    query = KQueryByDate(Datetime(201112090000), Datetime(201112100000), KQuery::WEEK);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 1);
    CHECK_EQ(kdata.empty(), false);
    CHECK_EQ(kdata.startPos(), 1058);
    CHECK_EQ(kdata.endPos(), 1059);
    CHECK_EQ(kdata.lastPos(), 1058);
    CHECK_EQ(kdata[0], KRecord(Datetime(201112090000), 2363.1110, 2363.1270, 2310.1550, 2325.9050,
                               9126681.1000, 98132048.0000));

    ///===================================
    /// 测试分钟线
    ///===================================
    /** @arg SH000001全部分钟线数据 */
    stock = sm.getStock("sh000001");
    query = KQueryByDate(Datetime::min(), Null<Datetime>(), KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 682823L);
    record = kdata[0];
    expect =
      KRecord(Datetime(200001040931), 1366.58, 1368.692, 1366.579, 1368.692, 4124880, 191158);
    CHECK_EQ(record, expect);

    record = kdata[1];
    expect = KRecord(Datetime(200001040932), 1368.3, 1368.491, 1368.3, 1368.491, 2783200, 106916);
    CHECK_EQ(record, expect);

    record = kdata[kdata.size() - 1];
    expect = KRecord(Datetime(201112061500), 2325.38, 2326.1, 2324.6, 2325.90, 5215150, 566312);
    CHECK_EQ(record, expect);

    /** @arg SH000001分钟线数据, 起始时间为第一条记录日期的前一分钟 */
    stock = sm.getStock("sh000001");
    query = KQueryByDate(Datetime(200001040930), Null<Datetime>(), KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 682823L);
    record = kdata[0];
    expect =
      KRecord(Datetime(200001040931), 1366.58, 1368.692, 1366.579, 1368.692, 4124880, 191158);
    CHECK_EQ(record, expect);

    record = kdata[1];
    expect = KRecord(Datetime(200001040932), 1368.3, 1368.491, 1368.3, 1368.491, 2783200, 106916);
    CHECK_EQ(record, expect);

    record = kdata[kdata.size() - 1];
    expect = KRecord(Datetime(201112061500), 2325.38, 2326.1, 2324.6, 2325.90, 5215150, 566312);
    CHECK_EQ(record, expect);

    /** @arg SH000001分钟线数据, 起始时间等于第一条记录日期 */
    stock = sm.getStock("sh000001");
    query = KQueryByDate(Datetime(200001040931), Null<Datetime>(), KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 682823L);
    record = kdata[0];
    CHECK_EQ(record.datetime, Datetime(200001040931));

    record = kdata[kdata.size() - 1];
    CHECK_EQ(record.datetime, Datetime(201112061500));

    /** @arg SH000001分钟线数据, 起始时间等于第一条记录日期的后一刻 */
    stock = sm.getStock("sh000001");
    query = KQueryByDate(Datetime(200001040932), Null<Datetime>(), KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 682822L);
    record = kdata[0];
    CHECK_EQ(record.datetime, Datetime(200001040932));

    record = kdata[kdata.size() - 1];
    CHECK_EQ(record.datetime, Datetime(201112061500));

    /** @arg SH000001分钟线数据, 指定起始时间为第一条记录，截至日期为最后一条记录的前一刻 */
    stock = sm.getStock("sh000001");
    query = KQueryByDate(Datetime(200001040931), Datetime(201112061459), KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 682821L);
    record = kdata[0];
    CHECK_EQ(record.datetime, Datetime(200001040931));

    record = kdata[kdata.size() - 1];
    CHECK_EQ(record.datetime, Datetime(201112061458));

    /** @arg SH000001分钟线数据, 指定起始时间为第一条记录，截至日期为最后一条记录 */
    stock = sm.getStock("sh000001");
    query = KQueryByDate(Datetime(200001040931), Datetime(201112061500), KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 682822L);
    record = kdata[0];
    CHECK_EQ(record.datetime, Datetime(200001040931));

    record = kdata[kdata.size() - 1];
    CHECK_EQ(record.datetime, Datetime(201112061459));

    /** @arg SH000001分钟线数据, 指定起始时间为第一条记录，截至日期为最后一条记录的后一刻 */
    stock = sm.getStock("sh000001");
    query = KQueryByDate(Datetime(200001040931), Datetime(201112061501), KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 682823L);
    record = kdata[0];
    CHECK_EQ(record.datetime, Datetime(200001040931));

    record = kdata[kdata.size() - 1];
    CHECK_EQ(record.datetime, Datetime(201112061500));

    /** @arg SH000001分钟线数据, 中间任意一段日期 */
    stock = sm.getStock("sh000001");
    query = KQueryByDate(Datetime(200303051451), Datetime(200303051455), KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 4);
    record = kdata[0];
    CHECK_EQ(record.datetime, Datetime(200303051451));

    record = kdata[kdata.size() - 1];
    CHECK_EQ(record.datetime, Datetime(200303051454));

    /** @arg SH000001分钟线数据, 起始日期等于结束日期 */
    stock = sm.getStock("sh000001");
    query = KQueryByDate(Datetime(200001040931), Datetime(200001040931), KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 0);
    query = KQueryByDate(Datetime(200601041446), Datetime(200601041446), KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 0);
    query = KQueryByDate(Datetime(200801300955), Datetime(200601041446), KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 0);
    query = KQueryByDate(Datetime(201112061500), Datetime(201112061500), KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 0);

    /** @arg SH000001分钟线数据, 结束日期等于起始日期的后一刻，且数据存在 */
    stock = sm.getStock("sh000001");
    query = KQueryByDate(Datetime(200001040931), Datetime(200001040932), KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 1);
    record = kdata[0];
    CHECK_EQ(record.datetime, Datetime(200001040931));

    query = KQueryByDate(Datetime(200601041446), Datetime(200601041447), KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 1);
    record = kdata[0];
    CHECK_EQ(record.datetime, Datetime(200601041446));

    query = KQueryByDate(Datetime(201112061500), Datetime(201112061501), KQuery::MIN);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata.size(), 1);
    record = kdata[0];
    CHECK_EQ(record.datetime, Datetime(201112061500));
}

/** @par 检测点 */
TEST_CASE("test_getKData_recover") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KQuery query;
    KData kdata;

    /** @arg 前向复权*/
    query = KQuery(0, Null<int64_t>(), KQuery::DAY, KQuery::FORWARD);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata[2710],
             KRecord(Datetime(201106030000), 10.02, 10.14, 10.0, 10.09, 38726.1, 384820));
    HKU_INFO("{}", kdata[2710]);
    CHECK_EQ(kdata[2709],
             KRecord(Datetime(201106020000), 10.34, 10.38, 9.93, 10.04, 103909.3, 780543.0));
    CHECK_EQ(kdata[2554],
             KRecord(Datetime(201010140000), 11.04, 11.42, 10.91, 10.95, 322428.8, 2195006));
    CHECK_EQ(kdata[2548], KRecord(Datetime(201009290000), 9.26, 9.64, 9.20, 9.48, 99719.8, 799165));
    CHECK_EQ(kdata[2547], KRecord(Datetime(201009280000), 9.82, 9.82, 9.55, 9.55, 81241.5, 639882));

    /** @arg 后向复权*/
    query = KQuery(0, Null<int64_t>(), KQuery::DAY, KQuery::BACKWARD);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata[0], KRecord(Datetime(199911100000), 29.5, 29.8, 27.0, 27.75, 485910.2, 1740850));
    CHECK_EQ(kdata[151],
             KRecord(Datetime(200007050000), 23.25, 23.47, 23.15, 23.22, 3298.8, 14218));
    CHECK_EQ(kdata[152],
             KRecord(Datetime(200007060000), 23.30, 23.42, 23.16, 23.23, 3049.5, 13200));
    CHECK_EQ(kdata[657],
             KRecord(Datetime(200208210000), 18.35, 18.75, 18.18, 18.55, 36409.8, 197640));
    CHECK_EQ(kdata[658],
             KRecord(Datetime(200208220000), 18.77, 18.89, 18.62, 18.81, 13101.3, 106872));

    /** @arg 前向等比复权*/
    query = KQuery(0, Null<int64_t>(), KQuery::DAY, KQuery::EQUAL_FORWARD);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata[2710],
             KRecord(Datetime(201106030000), 10.02, 10.14, 10.0, 10.09, 38726.1, 384820));
    CHECK_EQ(kdata[2709],
             KRecord(Datetime(201106020000), 10.33, 10.37, 9.93, 10.04, 103909.3, 780543.0));
    CHECK_EQ(kdata[2554],
             KRecord(Datetime(201010140000), 11.03, 11.40, 10.90, 10.94, 322428.8, 2195006));
    CHECK_EQ(kdata[2548], KRecord(Datetime(201009290000), 9.27, 9.64, 9.21, 9.49, 99719.8, 799165));
    CHECK_EQ(kdata[2547], KRecord(Datetime(201009280000), 9.82, 9.82, 9.55, 9.56, 81241.5, 639882));

    /** @arg 等比后向复权*/
    query = KQuery(0, Null<int64_t>(), KQuery::DAY, KQuery::EQUAL_BACKWARD);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata[0], KRecord(Datetime(199911100000), 29.5, 29.8, 27.0, 27.75, 485910.2, 1740850));
    CHECK_EQ(kdata[151],
             KRecord(Datetime(200007050000), 23.25, 23.47, 23.15, 23.22, 3298.8, 14218));
    CHECK_EQ(kdata[152],
             KRecord(Datetime(200007060000), 23.30, 23.42, 23.16, 23.23, 3049.5, 13200));
    CHECK_EQ(kdata[657],
             KRecord(Datetime(200208210000), 18.32, 18.72, 18.15, 18.52, 36409.8, 197640));
    CHECK_EQ(kdata[658],
             KRecord(Datetime(200208220000), 18.74, 18.86, 18.59, 18.79, 13101.3, 106872));
}

/** @par 检测点 */
TEST_CASE("test_getKRecord_By_Date") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KQuery query;
    KData kdata;
    KRecord result;

    /** @arg kdata为空 */
    result = kdata.getKRecord(Datetime(200101010000));
    CHECK_EQ(result, Null<KRecord>());

    /** @arg 日线*/
    query = KQuery(1, 10, KQuery::DAY);
    kdata = stock.getKData(query);
    result = kdata.getKRecord(Datetime(199911100000));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(kdata[0].datetime);
    CHECK_EQ(result, kdata[0]);

    result = kdata.getKRecord(kdata[1].datetime);
    CHECK_EQ(result, kdata[1]);

    result = kdata.getKRecord(Datetime(199911130000));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(kdata[7].datetime);
    CHECK_EQ(result, kdata[7]);

    result = kdata.getKRecord(kdata[8].datetime);
    CHECK_EQ(result, kdata[8]);

    result = kdata.getKRecord(Datetime(199911240000));
    CHECK_EQ(result, Null<KRecord>());

    /** @arg 周线*/
    query = KQuery(1, 10, KQuery::WEEK);
    kdata = stock.getKData(query);
    result = kdata.getKRecord(Datetime(199911070000));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(Datetime(199911080000));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(kdata[0].datetime);
    CHECK_EQ(result, kdata[0]);

    result = kdata.getKRecord(kdata[1].datetime);
    CHECK_EQ(result, kdata[1]);

    result = kdata.getKRecord(Datetime(199101200000));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(kdata[7].datetime);
    CHECK_EQ(result, kdata[7]);

    result = kdata.getKRecord(kdata[8].datetime);
    CHECK_EQ(result, kdata[8]);

    result = kdata.getKRecord(Datetime(199002190000));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(Datetime(199002250000));
    CHECK_EQ(result, Null<KRecord>());

    /** @arg 月线*/
    query = KQuery(1, 10, KQuery::MONTH);
    kdata = stock.getKData(query);
    result = kdata.getKRecord(Datetime(199012010000));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(Datetime(199012310000));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(kdata[0].datetime);
    CHECK_EQ(result, kdata[0]);

    result = kdata.getKRecord(kdata[1].datetime);
    CHECK_EQ(result, kdata[1]);

    result = kdata.getKRecord(Datetime(199103020000));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(kdata[7].datetime);
    CHECK_EQ(result, kdata[7]);

    result = kdata.getKRecord(kdata[8].datetime);
    CHECK_EQ(result, kdata[8]);

    result = kdata.getKRecord(Datetime(199109020000));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(Datetime(199110010000));
    CHECK_EQ(result, Null<KRecord>());

    /** @arg 季线*/
    query = KQuery(1, 10, KQuery::QUARTER);
    kdata = stock.getKData(query);
    result = kdata.getKRecord(Datetime(199909300000));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(Datetime(199910010000));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(kdata[0].datetime);
    CHECK_EQ(result, kdata[0]);

    result = kdata.getKRecord(kdata[1].datetime);
    CHECK_EQ(result, kdata[1]);

    result = kdata.getKRecord(Datetime(200012010000));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(kdata[7].datetime);
    CHECK_EQ(result, kdata[7]);

    result = kdata.getKRecord(kdata[8].datetime);
    CHECK_EQ(result, kdata[8]);

    result = kdata.getKRecord(Datetime(200204010000));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(Datetime(200205010000));
    CHECK_EQ(result, Null<KRecord>());

    /** @arg 半年线*/
    query = KQuery(1, 10, KQuery::HALFYEAR);
    kdata = stock.getKData(query);
    result = kdata.getKRecord(Datetime(199906300000));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(Datetime(199907010000));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(kdata[0].datetime);
    CHECK_EQ(result, kdata[0]);

    result = kdata.getKRecord(kdata[1].datetime);
    CHECK_EQ(result, kdata[1]);

    result = kdata.getKRecord(Datetime(200209010000));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(kdata[7].datetime);
    CHECK_EQ(result, kdata[7]);

    result = kdata.getKRecord(kdata[8].datetime);
    CHECK_EQ(result, kdata[8]);

    result = kdata.getKRecord(Datetime(200407010000));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(Datetime(200408010000));
    CHECK_EQ(result, Null<KRecord>());

    /** @arg 年线*/
    query = KQuery(1, 10, KQuery::YEAR);
    kdata = stock.getKData(query);
    result = kdata.getKRecord(Datetime(199801010000));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(Datetime(199901010000));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(kdata[0].datetime);
    CHECK_EQ(result, kdata[0]);

    result = kdata.getKRecord(kdata[1].datetime);
    CHECK_EQ(result, kdata[1]);

    result = kdata.getKRecord(Datetime(200209010000));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(kdata[7].datetime);
    CHECK_EQ(result, kdata[7]);

    result = kdata.getKRecord(kdata[8].datetime);
    CHECK_EQ(result, kdata[8]);

    result = kdata.getKRecord(Datetime(200901010000));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(Datetime(200901020000));
    CHECK_EQ(result, Null<KRecord>());

    /** @arg 分钟线*/
    query = KQuery(1, 10, KQuery::MIN);
    kdata = stock.getKData(query);
    result = kdata.getKRecord(Datetime(200001030931));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(Datetime(200001040931));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(kdata[0].datetime);
    CHECK_EQ(result, kdata[0]);

    result = kdata.getKRecord(kdata[1].datetime);
    CHECK_EQ(result, kdata[1]);

    result = kdata.getKRecord(kdata[7].datetime);
    CHECK_EQ(result, kdata[7]);

    result = kdata.getKRecord(kdata[8].datetime);
    CHECK_EQ(result, kdata[8]);

    result = kdata.getKRecord(Datetime(200001040941));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(Datetime(200001040942));
    CHECK_EQ(result, Null<KRecord>());

    /** @arg 5分钟线*/
    query = KQuery(1, 10, KQuery::MIN5);
    kdata = stock.getKData(query);
    result = kdata.getKRecord(Datetime(200001030935));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(Datetime(200001040935));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(kdata[0].datetime);
    CHECK_EQ(result, kdata[0]);

    result = kdata.getKRecord(kdata[1].datetime);
    CHECK_EQ(result, kdata[1]);

    result = kdata.getKRecord(Datetime(200001041011));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(kdata[7].datetime);
    CHECK_EQ(result, kdata[7]);

    result = kdata.getKRecord(kdata[8].datetime);
    CHECK_EQ(result, kdata[8]);

    result = kdata.getKRecord(Datetime(200001041025));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(Datetime(200001041030));
    CHECK_EQ(result, Null<KRecord>());

    /** @arg 15分钟线*/
    query = KQuery(1, 10, KQuery::MIN15);
    kdata = stock.getKData(query);
    result = kdata.getKRecord(Datetime(200001030945));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(Datetime(200001040945));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(kdata[0].datetime);
    CHECK_EQ(result, kdata[0]);

    result = kdata.getKRecord(kdata[1].datetime);
    CHECK_EQ(result, kdata[1]);

    result = kdata.getKRecord(Datetime(200001041116));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(kdata[7].datetime);
    CHECK_EQ(result, kdata[7]);

    result = kdata.getKRecord(kdata[8].datetime);
    CHECK_EQ(result, kdata[8]);

    result = kdata.getKRecord(Datetime(200001041345));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(Datetime(200001041400));
    CHECK_EQ(result, Null<KRecord>());

    /** @arg 30分钟线*/
    query = KQuery(1, 10, KQuery::MIN30);
    kdata = stock.getKData(query);
    result = kdata.getKRecord(Datetime(200001031000));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(Datetime(200001041000));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(kdata[0].datetime);
    CHECK_EQ(result, kdata[0]);

    result = kdata.getKRecord(kdata[1].datetime);
    CHECK_EQ(result, kdata[1]);

    result = kdata.getKRecord(Datetime(200001041116));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(kdata[7].datetime);
    CHECK_EQ(result, kdata[7]);

    result = kdata.getKRecord(kdata[8].datetime);
    CHECK_EQ(result, kdata[8]);

    result = kdata.getKRecord(Datetime(200001051100));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(Datetime(200001051100));
    CHECK_EQ(result, Null<KRecord>());

    /** @arg 60分钟线*/
    query = KQuery(1, 10, KQuery::MIN60);
    kdata = stock.getKData(query);
    result = kdata.getKRecord(Datetime(200001031030));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(Datetime(200001041030));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(kdata[0].datetime);
    CHECK_EQ(result, kdata[0]);

    result = kdata.getKRecord(kdata[1].datetime);
    CHECK_EQ(result, kdata[1]);

    result = kdata.getKRecord(Datetime(200001041116));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(kdata[7].datetime);
    CHECK_EQ(result, kdata[7]);

    result = kdata.getKRecord(kdata[8].datetime);
    CHECK_EQ(result, kdata[8]);

    result = kdata.getKRecord(Datetime(200001061400));
    CHECK_EQ(result, Null<KRecord>());

    result = kdata.getKRecord(Datetime(200001061400));
    CHECK_EQ(result, Null<KRecord>());
}

/** @} */
