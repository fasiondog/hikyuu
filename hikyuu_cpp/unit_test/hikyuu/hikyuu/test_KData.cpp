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

#define CHECK_KDATA_EQUAL(kdata1, kdata2)        \
    CHECK_EQ(kdata1, kdata2);                    \
    CHECK_EQ(kdata1.size(), kdata2.size());      \
    for (size_t i = 0; i < kdata1.size(); i++) { \
        CHECK_EQ(kdata1[i], kdata2[i]);          \
    }

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
    CHECK_EQ(k1, null_k);
    CHECK_UNARY(k1.empty());

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
             KRecord(Datetime(201106020000), 10.34, 10.38, 9.93, 10.04, 6028196.72, 600418.0));
    CHECK_EQ(kdata[2554],
             KRecord(Datetime(201010140000), 11.04, 11.42, 10.91, 10.95, 18488702.7, 1688466.0));
    CHECK_EQ(kdata[2548],
             KRecord(Datetime(201009290000), 9.26, 9.64, 9.20, 9.48, 5827754.16, 614742.0));
    CHECK_EQ(kdata[2547],
             KRecord(Datetime(201009280000), 9.82, 9.82, 9.55, 9.55, 4700672.35, 492217.0));

    /** @arg 后向复权*/
    query = KQuery(0, Null<int64_t>(), KQuery::DAY, KQuery::BACKWARD);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata[0], KRecord(Datetime(199911100000), 29.5, 29.8, 27.0, 27.75, 485910.2, 1740850));
    CHECK_EQ(kdata[151],
             KRecord(Datetime(200007050000), 23.25, 23.47, 23.15, 23.22, 3298.8, 14218));
    CHECK_EQ(kdata[152],
             KRecord(Datetime(200007060000), 23.30, 23.42, 23.16, 23.23, 306636., 13200.0));
    CHECK_EQ(kdata[657],
             KRecord(Datetime(200208210000), 18.35, 18.75, 18.18, 18.55, 3666222., 197640.0));
    CHECK_EQ(kdata[658],
             KRecord(Datetime(200208220000), 18.77, 18.89, 18.62, 18.82, 1340887.36, 71248.));

    /** @arg 前向等比复权*/
    query = KQuery(0, Null<int64_t>(), KQuery::DAY, KQuery::EQUAL_FORWARD);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata[2710],
             KRecord(Datetime(201106030000), 10.02, 10.14, 10.0, 10.09, 38726.1, 384820));
    CHECK_EQ(kdata[2709],
             KRecord(Datetime(201106020000), 10.33, 10.37, 9.93, 10.04, 6028196.72, 600418.0));
    CHECK_EQ(kdata[2554],
             KRecord(Datetime(201010140000), 11.03, 11.40, 10.90, 10.94, 18471818.04, 1688466.0));
    CHECK_EQ(kdata[2548],
             KRecord(Datetime(201009290000), 9.27, 9.64, 9.21, 9.49, 5833901.58, 614742.0));
    CHECK_EQ(kdata[2547],
             KRecord(Datetime(201009280000), 9.82, 9.82, 9.55, 9.56, 4705594.52, 492217.00));

    /** @arg 等比后向复权*/
    query = KQuery(0, Null<int64_t>(), KQuery::DAY, KQuery::EQUAL_BACKWARD);
    kdata = stock.getKData(query);
    CHECK_EQ(kdata[0], KRecord(Datetime(199911100000), 29.5, 29.8, 27.0, 27.75, 485910.2, 1740850));
    CHECK_EQ(kdata[151],
             KRecord(Datetime(200007050000), 23.25, 23.47, 23.15, 23.22, 3298.8, 14218));
    CHECK_EQ(kdata[152],
             KRecord(Datetime(200007060000), 23.30, 23.42, 23.16, 23.23, 306636.0, 13200.0));
    CHECK_EQ(kdata[657],
             KRecord(Datetime(200208210000), 18.32, 18.72, 18.15, 18.52, 3660292.8, 197640.0));
    CHECK_EQ(kdata[658],
             KRecord(Datetime(200208220000), 18.74, 18.86, 18.59, 18.79, 3012187.32, 160308.0));
}

/** @} */
TEST_CASE("test_getKRecord_By_Date") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KQuery query;
    KData kdata;
    KRecord result;

    /** @arg kdata为空 */
    result = kdata.getKRecord(Datetime(200101010000));
    CHECK_EQ(result, KRecord::NullKRecord);

    /** @arg 日线*/
    query = KQuery(1, 10, KQuery::DAY);
    kdata = stock.getKData(query);
    result = kdata.getKRecord(Datetime(199911100000));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(kdata[0].datetime);
    CHECK_EQ(result, kdata[0]);

    result = kdata.getKRecord(kdata[1].datetime);
    CHECK_EQ(result, kdata[1]);

    result = kdata.getKRecord(Datetime(199911130000));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(kdata[7].datetime);
    CHECK_EQ(result, kdata[7]);

    result = kdata.getKRecord(kdata[8].datetime);
    CHECK_EQ(result, kdata[8]);

    result = kdata.getKRecord(Datetime(199911240000));
    CHECK_EQ(result, KRecord::NullKRecord);

    /** @arg 周线*/
    query = KQuery(1, 10, KQuery::WEEK);
    kdata = stock.getKData(query);
    result = kdata.getKRecord(Datetime(199911070000));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(Datetime(199911080000));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(kdata[0].datetime);
    CHECK_EQ(result, kdata[0]);

    result = kdata.getKRecord(kdata[1].datetime);
    CHECK_EQ(result, kdata[1]);

    result = kdata.getKRecord(Datetime(199101200000));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(kdata[7].datetime);
    CHECK_EQ(result, kdata[7]);

    result = kdata.getKRecord(kdata[8].datetime);
    CHECK_EQ(result, kdata[8]);

    result = kdata.getKRecord(Datetime(199002190000));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(Datetime(199002250000));
    CHECK_EQ(result, KRecord::NullKRecord);

    /** @arg 月线*/
    query = KQuery(1, 10, KQuery::MONTH);
    kdata = stock.getKData(query);
    result = kdata.getKRecord(Datetime(199012010000));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(Datetime(199012310000));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(kdata[0].datetime);
    CHECK_EQ(result, kdata[0]);

    result = kdata.getKRecord(kdata[1].datetime);
    CHECK_EQ(result, kdata[1]);

    result = kdata.getKRecord(Datetime(199103020000));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(kdata[7].datetime);
    CHECK_EQ(result, kdata[7]);

    result = kdata.getKRecord(kdata[8].datetime);
    CHECK_EQ(result, kdata[8]);

    result = kdata.getKRecord(Datetime(199109020000));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(Datetime(199110010000));
    CHECK_EQ(result, KRecord::NullKRecord);

    /** @arg 季线*/
    query = KQuery(1, 10, KQuery::QUARTER);
    kdata = stock.getKData(query);
    result = kdata.getKRecord(Datetime(199909300000));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(Datetime(199910010000));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(kdata[0].datetime);
    CHECK_EQ(result, kdata[0]);

    result = kdata.getKRecord(kdata[1].datetime);
    CHECK_EQ(result, kdata[1]);

    result = kdata.getKRecord(Datetime(200012010000));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(kdata[7].datetime);
    CHECK_EQ(result, kdata[7]);

    result = kdata.getKRecord(kdata[8].datetime);
    CHECK_EQ(result, kdata[8]);

    result = kdata.getKRecord(Datetime(200204010000));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(Datetime(200205010000));
    CHECK_EQ(result, KRecord::NullKRecord);

    /** @arg 半年线*/
    query = KQuery(1, 10, KQuery::HALFYEAR);
    kdata = stock.getKData(query);
    result = kdata.getKRecord(Datetime(199906300000));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(Datetime(199907010000));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(kdata[0].datetime);
    CHECK_EQ(result, kdata[0]);

    result = kdata.getKRecord(kdata[1].datetime);
    CHECK_EQ(result, kdata[1]);

    result = kdata.getKRecord(Datetime(200209010000));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(kdata[7].datetime);
    CHECK_EQ(result, kdata[7]);

    result = kdata.getKRecord(kdata[8].datetime);
    CHECK_EQ(result, kdata[8]);

    result = kdata.getKRecord(Datetime(200407010000));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(Datetime(200408010000));
    CHECK_EQ(result, KRecord::NullKRecord);

    /** @arg 年线*/
    query = KQuery(1, 10, KQuery::YEAR);
    kdata = stock.getKData(query);
    result = kdata.getKRecord(Datetime(199801010000));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(Datetime(199901010000));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(kdata[0].datetime);
    CHECK_EQ(result, kdata[0]);

    result = kdata.getKRecord(kdata[1].datetime);
    CHECK_EQ(result, kdata[1]);

    result = kdata.getKRecord(Datetime(200209010000));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(kdata[7].datetime);
    CHECK_EQ(result, kdata[7]);

    result = kdata.getKRecord(kdata[8].datetime);
    CHECK_EQ(result, kdata[8]);

    result = kdata.getKRecord(Datetime(200901010000));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(Datetime(200901020000));
    CHECK_EQ(result, KRecord::NullKRecord);

    /** @arg 分钟线*/
    query = KQuery(1, 10, KQuery::MIN);
    kdata = stock.getKData(query);
    result = kdata.getKRecord(Datetime(200001030931));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(Datetime(200001040931));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(kdata[0].datetime);
    CHECK_EQ(result, kdata[0]);

    result = kdata.getKRecord(kdata[1].datetime);
    CHECK_EQ(result, kdata[1]);

    result = kdata.getKRecord(kdata[7].datetime);
    CHECK_EQ(result, kdata[7]);

    result = kdata.getKRecord(kdata[8].datetime);
    CHECK_EQ(result, kdata[8]);

    result = kdata.getKRecord(Datetime(200001040941));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(Datetime(200001040942));
    CHECK_EQ(result, KRecord::NullKRecord);

    /** @arg 5分钟线*/
    query = KQuery(1, 10, KQuery::MIN5);
    kdata = stock.getKData(query);
    result = kdata.getKRecord(Datetime(200001030935));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(Datetime(200001040935));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(kdata[0].datetime);
    CHECK_EQ(result, kdata[0]);

    result = kdata.getKRecord(kdata[1].datetime);
    CHECK_EQ(result, kdata[1]);

    result = kdata.getKRecord(Datetime(200001041011));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(kdata[7].datetime);
    CHECK_EQ(result, kdata[7]);

    result = kdata.getKRecord(kdata[8].datetime);
    CHECK_EQ(result, kdata[8]);

    result = kdata.getKRecord(Datetime(200001041025));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(Datetime(200001041030));
    CHECK_EQ(result, KRecord::NullKRecord);

    /** @arg 15分钟线*/
    query = KQuery(1, 10, KQuery::MIN15);
    kdata = stock.getKData(query);
    result = kdata.getKRecord(Datetime(200001030945));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(Datetime(200001040945));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(kdata[0].datetime);
    CHECK_EQ(result, kdata[0]);

    result = kdata.getKRecord(kdata[1].datetime);
    CHECK_EQ(result, kdata[1]);

    result = kdata.getKRecord(Datetime(200001041116));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(kdata[7].datetime);
    CHECK_EQ(result, kdata[7]);

    result = kdata.getKRecord(kdata[8].datetime);
    CHECK_EQ(result, kdata[8]);

    result = kdata.getKRecord(Datetime(200001041345));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(Datetime(200001041400));
    CHECK_EQ(result, KRecord::NullKRecord);

    /** @arg 30分钟线*/
    query = KQuery(1, 10, KQuery::MIN30);
    kdata = stock.getKData(query);
    result = kdata.getKRecord(Datetime(200001031000));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(Datetime(200001041000));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(kdata[0].datetime);
    CHECK_EQ(result, kdata[0]);

    result = kdata.getKRecord(kdata[1].datetime);
    CHECK_EQ(result, kdata[1]);

    result = kdata.getKRecord(Datetime(200001041116));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(kdata[7].datetime);
    CHECK_EQ(result, kdata[7]);

    result = kdata.getKRecord(kdata[8].datetime);
    CHECK_EQ(result, kdata[8]);

    result = kdata.getKRecord(Datetime(200001051100));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(Datetime(200001051100));
    CHECK_EQ(result, KRecord::NullKRecord);

    /** @arg 60分钟线*/
    query = KQuery(1, 10, KQuery::MIN60);
    kdata = stock.getKData(query);
    result = kdata.getKRecord(Datetime(200001031030));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(Datetime(200001041030));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(kdata[0].datetime);
    CHECK_EQ(result, kdata[0]);

    result = kdata.getKRecord(kdata[1].datetime);
    CHECK_EQ(result, kdata[1]);

    result = kdata.getKRecord(Datetime(200001041116));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(kdata[7].datetime);
    CHECK_EQ(result, kdata[7]);

    result = kdata.getKRecord(kdata[8].datetime);
    CHECK_EQ(result, kdata[8]);

    result = kdata.getKRecord(Datetime(200001061400));
    CHECK_EQ(result, KRecord::NullKRecord);

    result = kdata.getKRecord(Datetime(200001061400));
    CHECK_EQ(result, KRecord::NullKRecord);
}

/** @par 检测点 */
TEST_CASE("test_KData_getOtherFromSelf") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");

    // 确保stock有效
    REQUIRE_FALSE(stock.isNull());

    /** @arg 测试分钟线数据，不同的查询条件 */
    KQuery query1(0, 10, KQuery::MIN);
    KData kdata1 = stock.getKData(query1);
    REQUIRE_FALSE(kdata1.empty());  // 确保有数据

    // 不测试相同查询，而是测试不同但类似的查询，避免shared_from_this可能带来的问题
    KQuery query_same(0, 10, KQuery::MIN);  // 相同类型的查询，但不是同一个对象
    KData kdata2 = kdata1.getKData(query_same);
    CHECK_KDATA_EQUAL(kdata2, kdata1);

    /** @arg 测试不同的查询类型，应该创建新的实例 */
    KQuery query3(0, 10, KQuery::DAY);
    KData kdata3 = kdata1.getKData(query3);
    CHECK_EQ(kdata3.getStock(), kdata1.getStock());
    CHECK_NE(kdata3.getQuery().kType(), kdata1.getQuery().kType());  // 不同的类型
    CHECK_UNARY(kdata3 != kdata1);

    /** @arg 测试带复权的数据，不同的复权类型，应该创建新的实例 */
    KQuery query4(0, 10, KQuery::MIN, KQuery::BACKWARD);  // 使用MIN类型
    KData kdata4 = kdata1.getKData(query4);
    KData expect = stock.getKData(query4);
    CHECK_NE(kdata4.getQuery().recoverType(), kdata1.getQuery().recoverType());  // 不同的复权类型
    CHECK_KDATA_EQUAL(kdata4, expect);

    /** @arg 测试索引查询 */
    KQuery query5(5, 15, KQuery::MIN);
    KData kdata5 = kdata1.getKData(query5);
    expect = stock.getKData(query5);
    CHECK_NE(kdata5.getQuery(), kdata1.getQuery());  // 不同的查询参数
    CHECK_KDATA_EQUAL(kdata5, expect);

    query5 = KQuery(15, 20, KQuery::MIN, KQuery::FORWARD);
    kdata5 = kdata1.getKData(query5);
    expect = stock.getKData(query5);
    CHECK_KDATA_EQUAL(kdata5, expect);

    query5 = KQuery(3, 5, KQuery::MIN, KQuery::FORWARD);
    kdata5 = kdata1.getKData(query5);
    expect = stock.getKData(query5);
    CHECK_KDATA_EQUAL(kdata5, expect);

    query4 = KQuery(3, 5, KQuery::MIN);
    kdata4 = stock.getKData(query4);
    query5 = KQuery(0, 10, KQuery::MIN);
    kdata5 = kdata4.getKData(query5);
    expect = stock.getKData(query5);
    CHECK_KDATA_EQUAL(kdata5, expect);

    /** @arg 测试日期查询 */
    KQuery query6(Datetime(201001010000), Datetime(201002010000), KQuery::DAY);
    KData kdata6 = stock.getKData(query6);
    KQuery query7(Datetime(201001010000), Datetime(201002010000), KQuery::DAY);  // 相同类型查询
    KData kdata7 = kdata6.getKData(query7);  // 不是完全相同的查询对象
    CHECK_KDATA_EQUAL(kdata7, kdata6);

    /** @arg 测试不同查询类型间的转换 */
    KQuery query8(Datetime(201001010000), Datetime(201002010000), KQuery::WEEK);
    KData kdata8 = kdata6.getKData(query8);
    expect = stock.getKData(query8);
    CHECK_KDATA_EQUAL(kdata8, expect);

    /** @arg 测试分钟级别查询 */
    KQuery query9(Datetime(201112050930), Datetime(201112061030), KQuery::MIN);  // 分钟级别查询
    KData kdata9 = stock.getKData(query9);
    KQuery query10(Datetime(201112060930), Datetime(201112061030), KQuery::MIN);
    KData kdata10 = kdata9.getKData(query10);
    expect = stock.getKData(query10);
    CHECK_UNARY(!kdata10.empty());
    CHECK_KDATA_EQUAL(kdata10, expect);

    query10 = KQueryByDate(Datetime(201112060930), Datetime(201112061530), KQuery::MIN);
    kdata10 = kdata9.getKData(query10);
    expect = stock.getKData(query10);
    CHECK_UNARY(!kdata10.empty());
    CHECK_KDATA_EQUAL(kdata10, expect);

    query10 = KQueryByDate(Datetime(201112010930), Datetime(201112050930), KQuery::MIN);
    kdata10 = kdata9.getKData(query10);
    expect = stock.getKData(query10);
    CHECK_UNARY(!kdata10.empty());
    CHECK_KDATA_EQUAL(kdata10, expect);

    query10 = KQueryByDate(Datetime(201112061131), Datetime(201112061530), KQuery::MIN);
    kdata10 = kdata9.getKData(query10);
    expect = stock.getKData(query10);
    CHECK_UNARY(!kdata10.empty());
    CHECK_KDATA_EQUAL(kdata10, expect);

    query10 =
      KQueryByDate(Datetime(201112060930), Datetime(201112061530), KQuery::MIN, KQuery::BACKWARD);
    kdata10 = kdata9.getKData(query10);
    expect = stock.getKData(query10);
    CHECK_UNARY(!kdata10.empty());
    CHECK_KDATA_EQUAL(kdata10, expect);

    /** @arg 测试分钟到其他类型的转换 */
    KQuery query11(Datetime(201001010930), Datetime(201001011030), KQuery::MIN5);
    KData kdata11 = kdata9.getKData(query11);
    expect = stock.getKData(query11);
    CHECK_KDATA_EQUAL(kdata11, expect);
}

/** @par 检测点 - 覆盖_getOtherFromSelfByIndex和_getOtherFromSelfByDate的分支 */
TEST_CASE("test_KData_getOtherFromSelf_subfunctions") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");

    /** @arg 测试_getOtherFromSelfByIndex: 新查询的开始位置小于旧数据的开始位置，应创建新的实例 */
    KQuery query1(10, 20, KQuery::DAY);  // 创建一个基础数据
    KData kdata1 = stock.getKData(query1);
    REQUIRE_FALSE(kdata1.empty());

    KQuery query_smaller_start(5, 15, KQuery::DAY);  // 开始位置更小
    KData kdata2 = kdata1.getKData(query_smaller_start);
    KData expect = stock.getKData(query_smaller_start);
    CHECK_KDATA_EQUAL(kdata2, expect);
    CHECK_EQ(kdata2.getStock(), kdata1.getStock());
    CHECK_NE(kdata2.getQuery(), kdata1.getQuery());  // 应该创建新的实例

    /** @arg 测试_getOtherFromSelfByIndex: 新查询的开始位置在旧数据范围内但结束位置超出 */
    size_t old_size = kdata1.size();
    KQuery query_extend_end(kdata1.startPos() + 2, kdata1.endPos() + 5, KQuery::DAY);
    KData kdata3 = stock.getKData(query_extend_end);   // 重新获取以确保数据完整
    KData kdata4 = kdata3.getKData(query_extend_end);  // 这里会触发_getOtherFromSelfByIndex的分支
    CHECK_EQ(kdata4.getStock(), kdata3.getStock());
    CHECK_EQ(kdata4.getQuery(), query_extend_end);

    /** @arg 测试_getOtherFromSelfByIndex: 新查询完全在旧数据范围内 */
    KQuery query_within(12, 15, KQuery::DAY);
    KData kdata5 = kdata1.getKData(query_within);
    CHECK_EQ(kdata5.getStock(), kdata1.getStock());
    CHECK_EQ(kdata5.getQuery().start(), query_within.start());
    CHECK_EQ(kdata5.getQuery().end(), query_within.end());

    /** @arg 测试_getOtherFromSelfByDate: 新开始日期小于旧数据开始日期 */
    KQuery date_query1(Datetime(201001010000), Datetime(201012010000), KQuery::DAY);
    KData kdata_date1 = stock.getKData(date_query1);
    REQUIRE_FALSE(kdata_date1.empty());

    KQuery date_query_earlier(Datetime(200901010000), Datetime(201011010000),
                              KQuery::DAY);  // 更早的开始日期
    KData kdata_date2 = kdata_date1.getKData(date_query_earlier);
    CHECK_EQ(kdata_date2.getStock(), kdata_date1.getStock());
    CHECK_NE(kdata_date2.getQuery(), kdata_date1.getQuery());  // 应该创建新的实例

    /** @arg 测试_getOtherFromSelfByDate: 新开始日期在旧数据范围内 */
    Datetime mid_date = kdata_date1[kdata_date1.size() / 2].datetime;
    KQuery date_query_within(mid_date, Datetime(201011010000), KQuery::DAY);
    KData kdata_date3 = kdata_date1.getKData(date_query_within);
    CHECK_EQ(kdata_date3.getStock(), kdata_date1.getStock());
    CHECK_EQ(kdata_date3.getQuery().startDatetime(), mid_date);

    /** @arg 测试_getOtherFromSelfByDate: 日期查询跨越现有数据范围 */
    Datetime end_date = kdata_date1.back().datetime;
    Datetime future_date = end_date + TimeDelta(30 * 24 * 3600);  // 30天后，使用TimeDelta
    KQuery date_query_extend(kdata_date1[2].datetime, future_date, KQuery::DAY);
    KData kdata_date4 = kdata_date1.getKData(date_query_extend);
    CHECK_EQ(kdata_date4.getStock(), kdata_date1.getStock());
    CHECK_GE(kdata_date4.size(), kdata_date1.size());  // 新数据应该不少于原数据

    /** @arg 测试索引转日期查询: 从索引查询转为日期查询 */
    KQuery idx_query(0, 100, KQuery::DAY);
    KData kdata_idx = stock.getKData(idx_query);
    KQuery date_from_idx(Datetime(200001010000), Datetime(200005010000), KQuery::DAY);
    KData kdata_date_from_idx = kdata_idx.getKData(date_from_idx);
    CHECK_EQ(kdata_date_from_idx.getStock(), kdata_idx.getStock());
    CHECK_EQ(kdata_date_from_idx.getQuery().kType(), KQuery::DAY);

    /** @arg 测试日期转索引查询: 从日期查询转为索引查询 */
    KQuery date_query_orig(Datetime(200001010000), Datetime(200005010000), KQuery::DAY);
    KData kdata_date_orig = stock.getKData(date_query_orig);
    KQuery idx_from_date(50, 150, KQuery::DAY);
    KData kdata_idx_from_date = kdata_date_orig.getKData(idx_from_date);
    CHECK_EQ(kdata_idx_from_date.getStock(), kdata_date_orig.getStock());
    CHECK_EQ(kdata_idx_from_date.getQuery().kType(), KQuery::DAY);
}

/** @par 检测点 */
TEST_CASE("test_KData_getKData") {
    KData k1, k2;

    /** @arg k1 为 Null<KData> */
    k2 = k1.getKData(KQuery::MIN);
    CHECK_EQ(k2, Null<KData>());
    k2 = k1.getKData(KQuery::WEEK);
    CHECK_EQ(k2, Null<KData>());

    /** @arg k1 长度为0 */
    k1 = getKData("sh000001", KQueryByIndex(10000, 100010, KQuery::DAY, KQuery::FORWARD));
    REQUIRE(k1.size() == 0);
    k2 = k1.getKData(KQuery::MIN);
    CHECK_UNARY(k2.empty());
    CHECK_EQ(k1.getStock(), k2.getStock());
    CHECK_EQ(k2.getQuery().kType(), KQuery::MIN);
    CHECK_EQ(k2.getQuery().recoverType(), k1.getQuery().recoverType());

    /** @arg query k1 为日线，query 为索引方式，非闭合 */
    k1 = getKData("sh000001", KQuery(-10));
    REQUIRE(k1.size() > 0);

    k2 = k1.getKData(KQuery::MIN);
    CHECK_EQ(k1.getStock(), k2.getStock());
    CHECK_EQ(k2.getQuery().kType(), KQuery::MIN);
    CHECK_EQ(k2.getQuery().recoverType(), k1.getQuery().recoverType());
    CHECK_EQ(k2.getQuery().startDatetime(), k1[0].datetime);
    REQUIRE(k1[0].datetime == Datetime(20111123000000));
    CHECK_EQ(k2[0].datetime, Datetime(20111123093100));

    k2 = k1.getKData(KQuery::WEEK);
    CHECK_EQ(k1.getStock(), k2.getStock());
    CHECK_EQ(k2.getQuery().kType(), KQuery::WEEK);
    CHECK_EQ(k2.getQuery().recoverType(), k1.getQuery().recoverType());
    CHECK_EQ(k2.getQuery().startDatetime(), Datetime(20111123000000));
    REQUIRE(k1[0].datetime == Datetime(20111123000000));
    CHECK_EQ(k2[0].datetime, Datetime(20111125000000));

    k1 = getKData("sh000001", KQuery(-10, Null<int64_t>(), KQuery::MONTH, KQuery::FORWARD));
    k2 = k1.getKData(KQuery::DAY);
    CHECK_EQ(k1.getStock(), k2.getStock());
    CHECK_EQ(k2.getQuery().kType(), KQuery::DAY);
    CHECK_EQ(k2.getQuery().recoverType(), k1.getQuery().recoverType());
    CHECK_EQ(k2.getQuery().startDatetime(), Datetime(20110331000000));
    CHECK_EQ(k2.getQuery().endDatetime(), Null<Datetime>());
    REQUIRE(k1.front().datetime == Datetime(20110331000000));
    CHECK_EQ(k2.front().datetime, Datetime(20110331000000));
    CHECK_EQ(k2.back().datetime, Datetime(20111206000000));

    k2 = k1.getKData(KQuery::MIN60);
    CHECK_EQ(k1.getStock(), k2.getStock());
    CHECK_EQ(k2.getQuery().kType(), KQuery::MIN60);
    CHECK_EQ(k2.front().datetime, Datetime(20110331103000));
    CHECK_EQ(k2.back().datetime, Datetime(20111206150000));

    /** @arg query k1 为日线，query 为索引方式，闭合 */
    k1 = getKData("sh000001", KQuery(3000, 3010));
    REQUIRE(k1.size() > 0);

    k2 = k1.getKData(KQuery::MIN);
    CHECK_EQ(k1.getStock(), k2.getStock());
    CHECK_EQ(k2.getQuery().kType(), KQuery::MIN);
    CHECK_EQ(k2.getQuery().recoverType(), k1.getQuery().recoverType());
    CHECK_EQ(k2.getQuery().startDatetime(), k1[0].datetime);
    REQUIRE(k1[0].datetime == Datetime(20030318000000));
    CHECK_EQ(k2[0].datetime, Datetime(20030318093100));
    REQUIRE(k1.back().datetime == Datetime(20030331000000));
    CHECK_EQ(k2.back().datetime, Datetime(20030331150000));

    k2 = k1.getKData(KQuery::WEEK);
    CHECK_EQ(k1.getStock(), k2.getStock());
    CHECK_EQ(k2.getQuery().kType(), KQuery::WEEK);
    CHECK_EQ(k2.getQuery().recoverType(), k1.getQuery().recoverType());
    CHECK_EQ(k2.getQuery().startDatetime(), Datetime(20030318000000));
    CHECK_EQ(k2.front().datetime, Datetime(20030321000000));
    CHECK_EQ(k2.back().datetime, Datetime(20030328000000));

    /** @arg query k1 为日线，query 为日期方式，非闭合 */
    k1 = getKData("sh000001", KQuery(Datetime(201101010000)));
    REQUIRE(k1.size() > 0);

    k2 = k1.getKData(KQuery::MIN);
    CHECK_EQ(k1.getStock(), k2.getStock());
    CHECK_EQ(k2.getQuery().kType(), KQuery::MIN);
    CHECK_EQ(k2.getQuery().recoverType(), k1.getQuery().recoverType());
    CHECK_EQ(k2.getQuery().startDatetime(), k1[0].datetime);
    CHECK_EQ(k2.front().datetime, Datetime(20110104093100));
    CHECK_EQ(k2.back().datetime, Datetime(20111206150000));

    k2 = k1.getKData(KQuery::WEEK);
    CHECK_EQ(k1.getStock(), k2.getStock());
    CHECK_EQ(k2.getQuery().kType(), KQuery::WEEK);
    CHECK_EQ(k2.getQuery().recoverType(), k1.getQuery().recoverType());
    CHECK_EQ(k2.front().datetime, Datetime(20110107000000));
    CHECK_EQ(k2.back().datetime, Datetime(20111209000000));

    /** @arg query k1 为日线，query 为日期方式，闭合 */
    k1 = getKData("sh000001", KQuery(Datetime(201101010000), Datetime(201112010000)));
    REQUIRE(k1.size() > 0);

    k2 = k1.getKData(KQuery::MIN);
    CHECK_EQ(k1.getStock(), k2.getStock());
    CHECK_EQ(k2.getQuery().kType(), KQuery::MIN);
    CHECK_EQ(k2.getQuery().recoverType(), k1.getQuery().recoverType());
    CHECK_EQ(k2.front().datetime, Datetime(20110104093100));
    CHECK_EQ(k2.back().datetime, Datetime(20111130150000));

    k2 = k1.getKData(KQuery::WEEK);
    CHECK_EQ(k1.getStock(), k2.getStock());
    CHECK_EQ(k2.getQuery().kType(), KQuery::WEEK);
    CHECK_EQ(k2.getQuery().recoverType(), k1.getQuery().recoverType());
    CHECK_EQ(k2.front().datetime, Datetime(20110107000000));
    CHECK_EQ(k2.back().datetime, Datetime(20111125000000));

    /** @arg 索引方式查询子集 */
    k1 = getKData("sh000001", KQuery(-10));
    k2 = k1.getSubKData(0, 1);
    CHECK_EQ(k2.size(), 1);
    CHECK_EQ(k2[0], k1[0]);

    k2 = k1.getSubKData(0, 10);
    CHECK_EQ(k2.size(), 10);
    for (size_t i = 0; i < k2.size(); i++) {
        CHECK_EQ(k2[i], k1[i]);
    }

    k2 = k1.getSubKData(0, 11);
    CHECK_EQ(k2.size(), 10);
    for (size_t i = 0; i < k2.size(); i++) {
        CHECK_EQ(k2[i], k1[i]);
    }

    k2 = k1.getSubKData(0, Null<int64_t>());
    CHECK_EQ(k2.size(), 10);
    for (size_t i = 0; i < k2.size(); i++) {
        CHECK_EQ(k2[i], k1[i]);
    }

    k2 = k1.getSubKData(10, Null<int64_t>());
    CHECK_EQ(k2.size(), 0);

    k2 = k1.getSubKData(10, 10);
    CHECK_EQ(k2.size(), 0);

    k2 = k1.getSubKData(10, 11);
    CHECK_EQ(k2.size(), 0);

    k2 = k1.getSubKData(9, 10);
    CHECK_EQ(k2.size(), 1);
    CHECK_EQ(k2[0], k1[9]);

    k2 = k1.getSubKData(9, 11);
    CHECK_EQ(k2.size(), 1);
    CHECK_EQ(k2[0], k1[9]);

    k2 = k1.getSubKData(2, 3);
    CHECK_EQ(k2.size(), 1);
    CHECK_EQ(k2[0], k1[2]);

    k2 = k1.getSubKData(3, 6);
    CHECK_EQ(k2.size(), 3);
    for (size_t i = 0; i < k2.size(); i++) {
        CHECK_EQ(k2[i], k1[i + 3]);
    }

    k2 = k1.getSubKData(6, 3);
    CHECK_EQ(k2.size(), 0);

    k2 = k1.getSubKData(-1, 0);
    CHECK_EQ(k2.size(), 0);

    k2 = k1.getSubKData(-1, 10);
    CHECK_EQ(k2.size(), 1);
    CHECK_EQ(k2[0], k1[9]);

    k2 = k1.getSubKData(-1, Null<int64_t>());
    CHECK_EQ(k2.size(), 1);
    CHECK_EQ(k2[0], k1[9]);

    k2 = k1.getSubKData(-10, Null<int64_t>());
    CHECK_EQ(k2.size(), 10);
    for (size_t i = 0; i < k2.size(); i++) {
        CHECK_EQ(k2[i], k1[i]);
    }

    k2 = k1.getSubKData(-1, -2);
    CHECK_EQ(k2.size(), 0);

    k2 = k1.getSubKData(-2, -1);
    CHECK_EQ(k2.size(), 1);
    CHECK_EQ(k2[0], k1[8]);

    k2 = k1.getSubKData(-2, -3);
    CHECK_EQ(k2.size(), 0);

    k2 = k1.getSubKData(-3, -2);
    CHECK_EQ(k2.size(), 1);
    CHECK_EQ(k2[0], k1[7]);

    k2 = k1.getSubKData(-3, -3);
    CHECK_EQ(k2.size(), 0);

    k2 = k1.getSubKData(-7, -3);
    CHECK_EQ(k2.size(), 4);
    for (size_t i = 0; i < k2.size(); i++) {
        CHECK_EQ(k2[i], k1[i + 3]);
    }
}

/** @par 其它覆盖率补充测试 */
TEST_CASE("test_KData_other") {
    KData k1 = getKData("sh000001", 0, 2);
    KData k2 = k1;
    CHECK_KDATA_EQUAL(k1, k2);

    // k1 = k1;
    // CHECK_KDATA_EQUAL(k1, k2);

    size_t count = 0;
    for (const auto& r : k1) {
        count++;
    }
    CHECK_EQ(count, k1.size());

    count = 0;
    KData::const_iterator iter = k1.cbegin();
    CHECK_EQ(iter->datetime, k1[0].datetime);
    for (; iter != k1.cend(); ++iter) {
        count++;
    }
    CHECK_EQ(count, k1.size());

    k2 = getKData("sh000001", k1[0].datetime, k1[1].datetime + Days(1));
    CHECK_EQ(k1.size(), k2.size());
    const KRecord* p1 = k1.data();
    KRecord* p2 = k2.data();
    for (size_t i = 0; i < k1.size(); i++) {
        CHECK_EQ(p1[i], p2[i]);
    }

    k1 = KData();
    CHECK_EQ(k1.getKRecord(0), Null<KRecord>());
    CHECK_EQ(k1.front(), Null<KRecord>());
    CHECK_EQ(k1.back(), Null<KRecord>());
    p1 = k1.data();
    CHECK_EQ(p1, nullptr);
    p2 = k1.data();
    CHECK_EQ(p2, nullptr);
}

/** @} */
