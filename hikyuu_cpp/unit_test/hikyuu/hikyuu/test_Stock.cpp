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

#define MEMORY_CHECK                                           \
    {                                                          \
        Stock mem_stock = sm["sh000001"];                      \
        CHECK_EQ(mem_stock.isBuffer(KQuery::DAY), true);       \
        CHECK_EQ(mem_stock.isBuffer(KQuery::WEEK), false);     \
        CHECK_EQ(mem_stock.isBuffer(KQuery::MONTH), false);    \
        CHECK_EQ(mem_stock.isBuffer(KQuery::QUARTER), false);  \
        CHECK_EQ(mem_stock.isBuffer(KQuery::HALFYEAR), false); \
        CHECK_EQ(mem_stock.isBuffer(KQuery::YEAR), false);     \
        CHECK_EQ(mem_stock.isBuffer(KQuery::MIN), false);      \
        CHECK_EQ(mem_stock.isBuffer(KQuery::MIN5), false);     \
        CHECK_EQ(mem_stock.isBuffer(KQuery::MIN15), false);    \
        CHECK_EQ(mem_stock.isBuffer(KQuery::MIN30), false);    \
        CHECK_EQ(mem_stock.isBuffer(KQuery::MIN60), false);    \
    }

/**
 * @defgroup test_hikyuu_Stock test_hikyuu_Stock
 * @ingroup test_hikyuu_base_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_Stock_Relational_comparison") {
    StockManager& sm = StockManager::instance();
    Stock s1;

    /** @arg 和Null<Stock>()进行相等比较 */
    CHECK_EQ(s1, Null<Stock>());

    /** @arg 和Null<Stock>()进行不等比较 */
    s1 = sm.getStock("sh000001");
    CHECK_NE(s1, Null<Stock>());

    /** @arg 相等比较 */
    Stock s2 = sm.getStock("000001.sh");
    CHECK_EQ(s1, s2);

    /** @arg 不等比较 */
    s2 = sm.getStock("sz000001");
    CHECK_NE(s1, s2);

    MEMORY_CHECK;
}

/** @par 检测点 */
TEST_CASE("test_Stock_getWeight") {
    StockManager& sm = StockManager::instance();

    Stock stock = sm.getStock("sz000001");
    StockWeight weight;

    /** @arg 查询全部权息信息 */
    StockWeightList weightList = stock.getWeight();
    CHECK_EQ(weightList.size(), 23);
    weight = weightList.front();
    CHECK_EQ(weight.datetime(), Datetime(199104030000L));
    CHECK_EQ(weight.countAsGift(), 0.0);
    CHECK_EQ(weight.countForSell(), 0.0);
    CHECK_EQ(weight.priceForSell(), 0.0);
    CHECK_EQ(weight.bonus(), 0.0);
    CHECK_EQ(weight.increasement(), 0.0);
    CHECK_EQ(weight.totalCount(), 150);
    CHECK_EQ(weight.freeCount(), 68);

    weight = weightList.back();
    CHECK_EQ(weight.datetime(), Datetime(201108050000L));
    CHECK_EQ(weight.countAsGift(), 0.0);
    CHECK_EQ(weight.countForSell(), 0.0);
    CHECK_EQ(weight.priceForSell(), 0.0);
    CHECK_EQ(weight.bonus(), 0.0);
    CHECK_EQ(weight.increasement(), 0.0);
    CHECK_EQ(weight.totalCount(), 512335);
    CHECK_EQ(weight.freeCount(), 310536);

    /** @arg 查询指定日期范围内的权息信息，指定的起始日期和结束日期没有刚好对应的权息记录 */
    weightList = stock.getWeight(Datetime(199501010000), Datetime(199701010000));
    CHECK_EQ(weightList.size(), 2);
    weight = weightList.front();
    CHECK_EQ(weight.datetime(), Datetime(199509250000));
    CHECK_EQ(weight.countAsGift(), 2.0);
    CHECK_EQ(weight.countForSell(), 0.0);
    CHECK_EQ(weight.priceForSell(), 0.0);
    CHECK_EQ(weight.bonus(), 3.0);
    CHECK_EQ(weight.increasement(), 0.0);
    CHECK_EQ(weight.totalCount(), 51728);
    CHECK_EQ(weight.freeCount(), 35721);

    weight = weightList.back();
    CHECK_EQ(weight.datetime(), Datetime(199605270000));
    CHECK_EQ(weight.countAsGift(), 5.0);
    CHECK_EQ(weight.countForSell(), 0.0);
    CHECK_EQ(weight.priceForSell(), 0.0);
    CHECK_EQ(weight.bonus(), 0.0);
    CHECK_EQ(weight.increasement(), 5.0);
    CHECK_EQ(weight.totalCount(), 103456);
    CHECK_EQ(weight.freeCount(), 71393);

    /** @arg 查询指定日期范围内的权息信息，指定的起始日期刚好存在对应权息记录，而结束日期没有 */
    weightList = stock.getWeight(Datetime(199509250000), Datetime(199701010000));
    CHECK_EQ(weightList.size(), 2);
    weight = weightList.front();
    CHECK_EQ(weight.datetime(), Datetime(199509250000));
    CHECK_EQ(weight.countAsGift(), 2.0);
    CHECK_EQ(weight.countForSell(), 0.0);
    CHECK_EQ(weight.priceForSell(), 0.0);
    CHECK_EQ(weight.bonus(), 3.0);
    CHECK_EQ(weight.increasement(), 0.0);
    CHECK_EQ(weight.totalCount(), 51728);
    CHECK_EQ(weight.freeCount(), 35721);

    weight = weightList.back();
    CHECK_EQ(weight.datetime(), Datetime(199605270000));
    CHECK_EQ(weight.countAsGift(), 5.0);
    CHECK_EQ(weight.countForSell(), 0.0);
    CHECK_EQ(weight.priceForSell(), 0.0);
    CHECK_EQ(weight.bonus(), 0.0);
    CHECK_EQ(weight.increasement(), 5.0);
    CHECK_EQ(weight.totalCount(), 103456);
    CHECK_EQ(weight.freeCount(), 71393);

    /** @arg 查询指定日期范围内的权息信息，指定的结束日期刚好存在对应权息记录，而起始日期没有 */
    weightList = stock.getWeight(Datetime(199501010000), Datetime(199605270000));
    CHECK_EQ(weightList.size(), 1);
    weight = weightList.front();
    CHECK_EQ(weight.datetime(), Datetime(199509250000));
    CHECK_EQ(weight.countAsGift(), 2.0);
    CHECK_EQ(weight.countForSell(), 0.0);
    CHECK_EQ(weight.priceForSell(), 0.0);
    CHECK_EQ(weight.bonus(), 3.0);
    CHECK_EQ(weight.increasement(), 0.0);
    CHECK_EQ(weight.totalCount(), 51728);
    CHECK_EQ(weight.freeCount(), 35721);

    /** @arg 查询指定日期范围内的权息信息，结束日期为Null<Datetime>() */
    weightList = stock.getWeight(Datetime(201101010000), Null<Datetime>());
    CHECK_EQ(weightList.size(), 1);
    weight = weightList.front();
    CHECK_EQ(weight.datetime(), Datetime(201108050000));
    CHECK_EQ(weight.countAsGift(), 0.0);
    CHECK_EQ(weight.countForSell(), 0.0);
    CHECK_EQ(weight.priceForSell(), 0.0);
    CHECK_EQ(weight.bonus(), 0.0);
    CHECK_EQ(weight.increasement(), 0.0);
    CHECK_EQ(weight.totalCount(), 512335);
    CHECK_EQ(weight.freeCount(), 310536);

    /** @arg 查询指定日期范围内的权息信息，只指定起始日期，结束日期为默认值 */
    weightList = stock.getWeight(Datetime(201101010000));
    CHECK_EQ(weightList.size(), 1);
    weight = weightList.front();
    CHECK_EQ(weight.datetime(), Datetime(201108050000));
    CHECK_EQ(weight.countAsGift(), 0.0);
    CHECK_EQ(weight.countForSell(), 0.0);
    CHECK_EQ(weight.priceForSell(), 0.0);
    CHECK_EQ(weight.bonus(), 0.0);
    CHECK_EQ(weight.increasement(), 0.0);
    CHECK_EQ(weight.totalCount(), 512335);
    CHECK_EQ(weight.freeCount(), 310536);

    MEMORY_CHECK;
}

/** @par 检测点 */
TEST_CASE("test_Stock_getCount") {
    StockManager& sm = StockManager::instance();

    /** @arg 查询sh000001的日线数量*/
    Stock stock = sm.getStock("sh000001");
    CHECK_EQ(stock.getCount(KQuery::DAY), 5121);

    /** @arg 查询sz000001的日线数量 */
    stock = sm.getStock("Sz000001");
    CHECK_EQ(stock.getCount(KQuery::DAY), 4937);

    /** @arg 查询sh000001的周线数量*/
    stock = sm.getStock("sh000001");
    CHECK_EQ(stock.getCount(KQuery::WEEK), 1059);

    /** @arg 查询sz000001的周线数量 */
    stock = sm.getStock("Sz000001");
    CHECK_EQ(stock.getCount(KQuery::WEEK), 1037);

    /** @arg 查询sh000001的月线数量*/
    stock = sm.getStock("sh000001");
    CHECK_EQ(stock.getCount(KQuery::MONTH), 253);

    /** @arg 查询sz000001的月线数量 */
    stock = sm.getStock("Sz000001");
    CHECK_EQ(stock.getCount(KQuery::MONTH), 250);

    /** @arg 查询sh000001的季线数量*/
    stock = sm.getStock("sh000001");
    CHECK_EQ(stock.getCount(KQuery::QUARTER), 85);

    /** @arg 查询sz000001的季线数量 */
    stock = sm.getStock("Sz000001");
    CHECK_EQ(stock.getCount(KQuery::QUARTER), 84);

    /** @arg 查询sh000001的半年线数量*/
    stock = sm.getStock("sh000001");
    CHECK_EQ(stock.getCount(KQuery::HALFYEAR), 43);

    /** @arg 查询sz000001的半年线数量 */
    stock = sm.getStock("Sz000001");
    CHECK_EQ(stock.getCount(KQuery::HALFYEAR), 42);

    /** @arg 查询sh000001的年线数量*/
    stock = sm.getStock("sh000001");
    CHECK_EQ(stock.getCount(KQuery::YEAR), 22);

    /** @arg 查询sz000001的年线数量 */
    stock = sm.getStock("Sz000001");
    CHECK_EQ(stock.getCount(KQuery::YEAR), 21);

    /** @arg 查询sh000001的分钟线数量 */
    stock = sm.getStock("sh000001");
    CHECK_EQ(stock.getCount(KQuery::MIN), 682823L);

    /** @arg 查询sz000001的分钟线数量 */
    stock = sm.getStock("Sz000001");
    CHECK_EQ(stock.getCount(KQuery::MIN), 641091L);

    /** @arg 查询sh000001的5分钟线数量 */
    stock = sm.getStock("sh000001");
    CHECK_EQ(stock.getCount(KQuery::MIN5), 133980L);

    /** @arg 查询sz000001的5分钟线数量 */
    stock = sm.getStock("Sz000001");
    CHECK_EQ(stock.getCount(KQuery::MIN5), 126381L);

    /** @arg 查询sh000001的15分钟线数量 */
    stock = sm.getStock("sh000001");
    CHECK_EQ(stock.getCount(KQuery::MIN15), 44750L);

    /** @arg 查询sz000001的15分钟线数量 */
    stock = sm.getStock("Sz000001");
    CHECK_EQ(stock.getCount(KQuery::MIN15), 42195L);

    /** @arg 查询sh000001的30分钟线数量 */
    stock = sm.getStock("sh000001");
    CHECK_EQ(stock.getCount(KQuery::MIN30), 22380L);

    /** @arg 查询sz000001的30分钟线数量 */
    stock = sm.getStock("Sz000001");
    CHECK_EQ(stock.getCount(KQuery::MIN30), 21101L);

    /** @arg 查询sh000001的60分钟线数量 */
    stock = sm.getStock("sh000001");
    CHECK_EQ(stock.getCount(KQuery::MIN60), 11193L);

    /** @arg 查询sz000001的60分钟线数量 */
    stock = sm.getStock("Sz000001");
    CHECK_EQ(stock.getCount(KQuery::MIN60), 10553L);

    MEMORY_CHECK;
}

/** @par 检测点 */
TEST_CASE("test_Stock_getKRecord") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KRecord record, expect;

    /** @arg 获取第一条日线记录 */
    record = stock.getKRecord(0);
    expect = KRecord(Datetime(199012190000), 96.05, 99.98, 95.79, 99.98, 49.4, 1260);
    CHECK_EQ(record, expect);

    /** @arg 获取中间随机一条日线记录 */
    record = stock.getKRecord(10);
    expect = KRecord(Datetime(199101030000), 128.840, 130.14, 128.84, 130.140, 9.3, 141);
    CHECK_EQ(record, expect);

    /** @arg 获取最后一条日线记录 */
    size_t total = stock.getCount();
    record = stock.getKRecord(total - 1);
    expect =
      KRecord(Datetime(201112060000), 2326.66, 2331.892, 2310.155, 2325.905, 4262559.5, 45917078);
    CHECK_EQ(record, expect);

    /** @arg 获取第一条周线记录 */
    record = stock.getKRecord(0, KQuery::WEEK);
    expect = KRecord(Datetime(199012210000), 96.05, 109.130, 95.790, 109.130, 59.4, 1485);
    CHECK_EQ(record, expect);

    /** @arg 获取中间随机一条周线记录 */
    record = stock.getKRecord(10, KQuery::WEEK);
    expect = KRecord(Datetime(199103010000), 134.37, 134.87, 132.47, 132.53, 827.1, 11500);
    CHECK_EQ(record, expect);

    /** @arg 获取最后一条周线记录 */
    total = stock.getCount(KQuery::WEEK);
    record = stock.getKRecord(total - 1, KQuery::WEEK);
    expect =
      KRecord(Datetime(201112090000), 2363.111, 2363.127, 2310.155, 2325.905, 9126681.1, 98132048);
    CHECK_EQ(record, expect);

    /** @arg 获取第一条月线记录 */
    record = stock.getKRecord(0, KQuery::MONTH);
    expect = KRecord(Datetime(199012310000), 96.05, 127.610, 95.790, 127.610, 93.6, 1884);
    CHECK_EQ(record, expect);

    /** @arg 获取中间随机一条月线记录 */
    record = stock.getKRecord(10, KQuery::MONTH);
    expect = KRecord(Datetime(199110310000), 181.550, 218.600, 179.800, 218.600, 14953.6, 223443);
    CHECK_EQ(record, expect);

    /** @arg 获取最后一条月线记录 */
    total = stock.getCount(KQuery::MONTH);
    record = stock.getKRecord(total - 1, KQuery::MONTH);
    expect = KRecord(Datetime(201112310000), 2392.485, 2423.559, 2310.155, 2325.905, 24378748.5,
                     256650941);
    CHECK_EQ(record, expect);

    /** @arg 获取第一条季线记录 */
    record = stock.getKRecord(0, KQuery::QUARTER);
    expect = KRecord(Datetime(199012310000), 96.05, 127.610, 95.790, 127.610, 93.6, 1884);
    CHECK_EQ(record, expect);

    /** @arg 获取中间随机一条季线记录 */
    record = stock.getKRecord(10, KQuery::QUARTER);
    expect =
      KRecord(Datetime(199306300000), 921.590, 1392.620, 859.48, 1007.050, 5181883.5, 27631999);
    CHECK_EQ(record, expect);

    /** @arg 获取最后一条季线记录 */
    total = stock.getCount(KQuery::QUARTER);
    record = stock.getKRecord(total - 1, KQuery::QUARTER);
    expect = KRecord(Datetime(201112310000), 2363.078, 2536.779, 2307.146, 2325.905, 298278573.1,
                     3100147936);
    CHECK_EQ(record, expect);

    /** @arg 获取第一条半年线记录 */
    record = stock.getKRecord(0, KQuery::HALFYEAR);
    expect = KRecord(Datetime(199012310000), 96.05, 127.610, 95.790, 127.610, 93.6, 1884);
    CHECK_EQ(record, expect);

    /** @arg 获取中间随机一条半年线记录 */
    record = stock.getKRecord(10, KQuery::HALFYEAR);
    expect =
      KRecord(Datetime(199512310000), 623.140, 792.540, 552.840, 555.290, 18764625.6, 351276329);
    CHECK_EQ(record, expect);

    /** @arg 获取最后一条半年线记录 */
    total = stock.getCount(KQuery::HALFYEAR);
    record = stock.getKRecord(total - 1, KQuery::HALFYEAR);
    expect = KRecord(Datetime(201112310000), 2767.834, 2826.959, 2307.146, 2325.905, 842271699.6,
                     8187717694);
    CHECK_EQ(record, expect);

    /** @arg 获取第一条年线记录 */
    record = stock.getKRecord(0, KQuery::YEAR);
    expect = KRecord(Datetime(199012310000), 96.05, 127.61, 95.79, 127.61, 93.6, 1884);
    CHECK_EQ(record, expect);

    /** @arg 获取中间随机一条年线记录 */
    record = stock.getKRecord(10, KQuery::YEAR);
    expect = KRecord(Datetime(200012310000), 1368.693, 2125.724, 1361.214, 2073.476, 326945832.5,
                     3433225623);
    CHECK_EQ(record, expect);

    /** @arg 获取最后一条年线记录 */
    total = stock.getCount(KQuery::YEAR);
    record = stock.getKRecord(total - 1, KQuery::YEAR);
    expect = KRecord(Datetime(201112310000), 2825.329, 3067.456, 2307.146, 2325.905, 2308389935.3,
                     21513978512);
    CHECK_EQ(record, expect);

    /** @arg 获取第一条分钟线记录 */
    record = stock.getKRecord(0, KQuery::MIN);
    expect =
      KRecord(Datetime("2000-1-4 9:31"), 1366.580, 1368.692, 1366.579, 1368.692, 4124880, 191158);
    CHECK_EQ(record, expect);

    /** @arg 获取中间随机一条分钟线记录 */
    record = stock.getKRecord(10, KQuery::MIN);
    expect =
      KRecord(Datetime(200001040941), 1367.389, 1367.389, 1366.578, 1366.578, 1214910.4, 31811);
    CHECK_EQ(record, expect);

    /** @arg 获取最后一条分钟线记录 */
    total = stock.getCount(KQuery::MIN);
    record = stock.getKRecord(total - 1, KQuery::MIN);
    expect =
      KRecord(Datetime("2011-12-06 15:00"), 2325.380, 2326.1, 2324.6, 2325.9, 5215150, 566312);
    CHECK_EQ(record, expect);

    /** @arg 获取第一条5分钟线记录 */
    record = stock.getKRecord(0, KQuery::MIN5);
    expect =
      KRecord(Datetime(200001040935), 1366.580, 1369.680, 1366.579, 1369.187, 106611.5, 438171);
    CHECK_EQ(record, expect);

    /** @arg 获取中间随机一条5分钟线记录 */
    record = stock.getKRecord(10, KQuery::MIN5);
    expect = KRecord(Datetime(200001041025), 1366.291, 1366.582, 1366.144, 1366.526, 89332, 118007);
    CHECK_EQ(record, expect);

    /** @arg 获取最后一条5分钟线记录 */
    total = stock.getCount(KQuery::MIN5);
    record = stock.getKRecord(total - 1, KQuery::MIN5);
    expect =
      KRecord(Datetime(201112061500), 2323.059, 2325.931, 2323.059, 2325.905, 176347.1, 1873675);
    CHECK_EQ(record, expect);

    /** @arg 获取第一条15分钟线记录 */
    record = stock.getKRecord(0, KQuery::MIN15);
    expect =
      KRecord(Datetime(200001040945), 1366.580, 1369.680, 1364.645, 1364.763, 250875.9, 912990);
    CHECK_EQ(record, expect);

    /** @arg 获取中间随机一条15分钟线记录 */
    record = stock.getKRecord(10, KQuery::MIN15);
    expect =
      KRecord(Datetime(200001041345), 1399.325, 1403.996, 1399.325, 1403.056, 441489.9, 547552);
    CHECK_EQ(record, expect);

    /** @arg 获取最后一条15分钟线记录 */
    total = stock.getCount(KQuery::MIN15);
    record = stock.getKRecord(total - 1, KQuery::MIN15);
    expect =
      KRecord(Datetime(201112061500), 2323.068, 2325.931, 2320.819, 2325.905, 402339.3, 4060591);
    CHECK_EQ(record, expect);

    /** @arg 获取第一条30分钟线记录 */
    record = stock.getKRecord(0, KQuery::MIN30);
    expect =
      KRecord(Datetime(200001041000), 1366.580, 1369.68, 1361.459, 1361.459, 455153, 1370442);
    CHECK_EQ(record, expect);

    /** @arg 获取中间随机一条30分钟线记录 */
    record = stock.getKRecord(10, KQuery::MIN30);
    expect =
      KRecord(Datetime(200001051100), 1421.179, 1425.868, 1421.179, 1423.859, 1109219.5, 1282851);
    CHECK_EQ(record, expect);

    /** @arg 获取最后一条30分钟线记录 */
    total = stock.getCount(KQuery::MIN30);
    record = stock.getKRecord(total - 1, KQuery::MIN30);
    expect =
      KRecord(Datetime(201112061500), 2327.919, 2327.919, 2320.819, 2325.905, 660297.5, 6415054);
    CHECK_EQ(record, expect);

    /** @arg 获取第一条60分钟线记录 */
    record = stock.getKRecord(0, KQuery::MIN60);
    expect =
      KRecord(Datetime(200001041030), 1366.580, 1369.68, 1361.295, 1367.683, 1041787.5, 2370736);
    CHECK_EQ(record, expect);

    /** @arg 获取中间随机一条60分钟线记录 */
    record = stock.getKRecord(10, KQuery::MIN60);
    expect =
      KRecord(Datetime(200001061400), 1411.490, 1436.53, 1411.49, 1436.53, 2432679.2, 3016536);
    CHECK_EQ(record, expect);

    /** @arg 获取最后一条60分钟线记录 */
    total = stock.getCount(KQuery::MIN60);
    record = stock.getKRecord(total - 1, KQuery::MIN60);
    expect =
      KRecord(Datetime(201112061500), 2326.036, 2331.378, 2320.819, 2325.905, 1332210, 11886356);
    CHECK_EQ(record, expect);

    MEMORY_CHECK;
}

/** @par 检测点 */
TEST_CASE("test_Stock_getIndexRange") {
    StockManager& sm = StockManager::instance();
    Stock stock;
    KQuery query;
    bool success;
    size_t out_start, out_end;
    stock = sm.getStock("sh000001");

    /** @arg 索引方式全部日线/周线/月/季/半年/年/1分钟/5分钟/15分钟/30分钟/60分钟 */
    query = KQuery();
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 5121);

    query = KQuery(0, Null<int64_t>(), KQuery::WEEK);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 1059);

    query = KQuery(0, Null<int64_t>(), KQuery::MONTH);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 253);

    query = KQuery(0, Null<int64_t>(), KQuery::QUARTER);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 85);

    query = KQuery(0, Null<int64_t>(), KQuery::HALFYEAR);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 43);

    query = KQuery(0, Null<int64_t>(), KQuery::YEAR);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 22);

    query = KQuery(0, Null<int64_t>(), KQuery::MIN);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 682823L);

    query = KQuery(0, Null<int64_t>(), KQuery::MIN5);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 133980L);

    query = KQuery(0, Null<int64_t>(), KQuery::MIN15);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 44750);

    query = KQuery(0, Null<int64_t>(), KQuery::MIN30);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 22380L);

    query = KQuery(0, Null<int64_t>(), KQuery::MIN60);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 11193);

    /** @arg 日期方式全部日线/周线/月/季/半年/年/1分钟/5分钟/15分钟/30分钟/60分钟 */
    query = KQueryByDate();
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 5121);

    query = KQueryByDate(Datetime::min(), Null<Datetime>(), KQuery::WEEK);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 1059);

    query = KQueryByDate(Datetime::min(), Null<Datetime>(), KQuery::MONTH);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 253);

    query = KQueryByDate(Datetime::min(), Null<Datetime>(), KQuery::QUARTER);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 85);

    query = KQueryByDate(Datetime::min(), Null<Datetime>(), KQuery::HALFYEAR);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 43);

    query = KQueryByDate(Datetime::min(), Null<Datetime>(), KQuery::YEAR);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 22);

    query = KQueryByDate(Datetime::min(), Null<Datetime>(), KQuery::MIN);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 682823L);

    query = KQueryByDate(Datetime::min(), Null<Datetime>(), KQuery::MIN5);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 133980L);

    query = KQueryByDate(Datetime::min(), Null<Datetime>(), KQuery::MIN15);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 44750);

    query = KQueryByDate(Datetime::min(), Null<Datetime>(), KQuery::MIN30);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 22380L);

    query = KQueryByDate(Datetime::min(), Null<Datetime>(), KQuery::MIN60);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 11193);

    /** @arg 索引方式 start>end 全部日线/周线/月/季/半年/年/1分钟/5分钟/15分钟/30分钟/60分钟 */
    query = KQuery(168, 165);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQuery(168, 165, KQuery::WEEK);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQuery(168, 165, KQuery::MONTH);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQuery(168, 165, KQuery::QUARTER);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQuery(168, 165, KQuery::HALFYEAR);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQuery(168, 165, KQuery::YEAR);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQuery(168, 165, KQuery::MIN);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQuery(168, 165, KQuery::MIN5);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQuery(168, 165, KQuery::MIN15);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQuery(168, 165, KQuery::MIN30);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQuery(168, 165, KQuery::MIN60);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    /** @arg 日期方式 start>end 全部日线/周线/月/季/半年/年/1分钟/5分钟/15分钟/30分钟/60分钟 */
    query = KQueryByDate(Datetime(200001010900), Datetime(200001010000));
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQueryByDate(Datetime(200001010900), Datetime(200001010000), KQuery::WEEK);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQueryByDate(Datetime(200001010900), Datetime(200001010000), KQuery::MONTH);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQueryByDate(Datetime(200001010900), Datetime(200001010000), KQuery::QUARTER);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQueryByDate(Datetime(200001010900), Datetime(200001010000), KQuery::HALFYEAR);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQueryByDate(Datetime(200001010900), Datetime(200001010000), KQuery::YEAR);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQueryByDate(Datetime(200001010900), Datetime(200001010000), KQuery::MIN);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQueryByDate(Datetime(200001010900), Datetime(200001010000), KQuery::MIN5);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQueryByDate(Datetime(200001010900), Datetime(200001010000), KQuery::MIN15);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQueryByDate(Datetime(200001010900), Datetime(200001010000), KQuery::MIN30);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQueryByDate(Datetime(200001010900), Datetime(200001010000), KQuery::MIN60);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    /** @arg 索引方式 start==end 全部日线/周线/月/季/半年/年/1分钟/5分钟/15分钟/30分钟/60分钟 */
    query = KQuery(165, 165);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQuery(165, 165, KQuery::WEEK);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQuery(165, 165, KQuery::MONTH);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQuery(165, 165, KQuery::QUARTER);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQuery(165, 165, KQuery::HALFYEAR);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQuery(165, 165, KQuery::YEAR);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQuery(165, 165, KQuery::MIN);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQuery(165, 165, KQuery::MIN5);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQuery(165, 165, KQuery::MIN15);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQuery(165, 165, KQuery::MIN30);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQuery(165, 165, KQuery::MIN60);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    /** @arg 日期方式 start==end 全部日线/周线/月/季/半年/年/1分钟/5分钟/15分钟/30分钟/60分钟 */
    query = KQueryByDate(Datetime(200001010000), Datetime(200001010000));
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQueryByDate(Datetime(200001010000), Datetime(200001010000), KQuery::WEEK);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQueryByDate(Datetime(200001010000), Datetime(200001010000), KQuery::MONTH);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQueryByDate(Datetime(200001010000), Datetime(200001010000), KQuery::QUARTER);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQueryByDate(Datetime(200001010000), Datetime(200001010000), KQuery::HALFYEAR);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQueryByDate(Datetime(200001010000), Datetime(200001010000), KQuery::YEAR);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQueryByDate(Datetime(200001010000), Datetime(200001010000), KQuery::MIN);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQueryByDate(Datetime(200001010000), Datetime(200001010000), KQuery::MIN5);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQueryByDate(Datetime(200001010000), Datetime(200001010000), KQuery::MIN15);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQueryByDate(Datetime(200001010000), Datetime(200001010000), KQuery::MIN30);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQueryByDate(Datetime(200001010000), Datetime(200001010000), KQuery::MIN60);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    /** @arg 索引方式选定正常范围日线/周线/月/季/半年/年/1分钟/5分钟/15分钟/30分钟/60分钟 */
    query = KQuery(1, 10);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 1);
    CHECK_EQ(out_end, 10);

    query = KQuery(1, 10, KQuery::WEEK);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 1);
    CHECK_EQ(out_end, 10);

    query = KQuery(1, 10, KQuery::MONTH);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 1);
    CHECK_EQ(out_end, 10);

    query = KQuery(1, 10, KQuery::QUARTER);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 1);
    CHECK_EQ(out_end, 10);

    query = KQuery(1, 10, KQuery::HALFYEAR);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 1);
    CHECK_EQ(out_end, 10);

    query = KQuery(1, 10, KQuery::YEAR);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 1);
    CHECK_EQ(out_end, 10);

    query = KQuery(1, 10, KQuery::MIN);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 1);
    CHECK_EQ(out_end, 10);

    query = KQuery(1, 10, KQuery::MIN5);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 1);
    CHECK_EQ(out_end, 10);

    query = KQuery(1, 10, KQuery::MIN15);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 1);
    CHECK_EQ(out_end, 10);

    query = KQuery(1, 10, KQuery::MIN30);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 1);
    CHECK_EQ(out_end, 10);

    query = KQuery(1, 10, KQuery::MIN60);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 1);
    CHECK_EQ(out_end, 10);

    /** @arg
     * 日期方式选定正常范围，且开始/结束日期在数据中刚好有对应的数据。日线/周线/月/季/半年/年/1分钟/5分钟/15分钟/30分钟/60分钟
     */
    query = KQueryByDate(Datetime(199012200000), Datetime(199101030000));
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 1);
    CHECK_EQ(out_end, 10);

    query = KQueryByDate(Datetime(199102250000), Datetime(199104290000), KQuery::WEEK);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 10);
    CHECK_EQ(out_end, 19);

    query = KQueryByDate(Datetime(199108010000), Datetime(199201010000), KQuery::MONTH);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 8);
    CHECK_EQ(out_end, 13);

    query = KQueryByDate(Datetime(199201010000), Datetime(199310010000), KQuery::QUARTER);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 5);
    CHECK_EQ(out_end, 12);

    query = KQueryByDate(Datetime(199401010000), Datetime(199801010000), KQuery::HALFYEAR);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 7);
    CHECK_EQ(out_end, 15);

    query = KQueryByDate(Datetime(199601010000), Datetime(200001010000), KQuery::YEAR);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 6);
    CHECK_EQ(out_end, 10);

    query = KQueryByDate(Datetime(200602171016), Datetime(200602171026), KQuery::MIN);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 343796L);
    CHECK_EQ(out_end, 343806L);

    query = KQueryByDate(Datetime(200001041025), Datetime(200001041100), KQuery::MIN5);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 10);
    CHECK_EQ(out_end, 17);

    query = KQueryByDate(Datetime(200001041415), Datetime(200001050945), KQuery::MIN15);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 12);
    CHECK_EQ(out_end, 16);

    query = KQueryByDate(Datetime(200001041330), Datetime(200001051500), KQuery::MIN30);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 4);
    CHECK_EQ(out_end, 15);

    query = KQueryByDate(Datetime(200001061400), Datetime(200001101130), KQuery::MIN60);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 10);
    CHECK_EQ(out_end, 17);

    /** @arg 日期方式查询，当开始/结束日期在数据中没有刚好相等的情况 */
    query = KQueryByDate(Datetime(199012220000), Datetime(199101050000));
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 3);
    CHECK_EQ(out_end, 12);

    query = KQueryByDate(Datetime(199102200000), Datetime(199104230000), KQuery::WEEK);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 9);
    CHECK_EQ(out_end, 18);

    query = KQueryByDate(Datetime(199107020000), Datetime(199112300000), KQuery::MONTH);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 7);
    CHECK_EQ(out_end, 12);

    query = KQueryByDate(Datetime(199112210000), Datetime(199308010000), KQuery::QUARTER);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 4);
    CHECK_EQ(out_end, 11);

    query = KQueryByDate(Datetime(199311010000), Datetime(199711010000), KQuery::HALFYEAR);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 6);
    CHECK_EQ(out_end, 14);

    query = KQueryByDate(Datetime(199502010000), Datetime(199912010000), KQuery::YEAR);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 5);
    CHECK_EQ(out_end, 9);

    query = KQueryByDate(Datetime(200602171016), Datetime(200602171026), KQuery::MIN);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 343796L);
    CHECK_EQ(out_end, 343806L);

    query = KQueryByDate(Datetime(200001041025), Datetime(200001041100), KQuery::MIN5);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 10);
    CHECK_EQ(out_end, 17);

    query = KQueryByDate(Datetime(200001041415), Datetime(200001050945), KQuery::MIN15);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 12);
    CHECK_EQ(out_end, 16);

    query = KQueryByDate(Datetime(200001041330), Datetime(200001051500), KQuery::MIN30);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 4);
    CHECK_EQ(out_end, 15);

    query = KQueryByDate(Datetime(200001061400), Datetime(200001101130), KQuery::MIN60);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 10);
    CHECK_EQ(out_end, 17);

    /** @arg 索引方式查询，测试索引为负值 */
    query = KQuery(-1, 0);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQuery(0, -1);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 5120);

    query = KQuery(-1, -1);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQuery(-1, -2);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, false);
    CHECK_EQ(out_start, 0);
    CHECK_EQ(out_end, 0);

    query = KQuery(-2, -1);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 5119);
    CHECK_EQ(out_end, 5120);

    query = KQuery(-5120, -1);
    success = stock.getIndexRange(query, out_start, out_end);
    CHECK_EQ(success, true);
    CHECK_EQ(out_start, 1);
    CHECK_EQ(out_end, 5120);

    MEMORY_CHECK;
}

/** @par 检测点 */
TEST_CASE("test_Stock_getKRecordList") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KRecordList result;
    KRecord record, expect;
    size_t total;

    MEMORY_CHECK;

    ///=====================
    /// 测试日线
    ///=====================
    /** @arg 日线 start, 0, end, total */
    total = stock.getCount(KQuery::DAY);
    result = stock.getKRecordList(KQuery(0, total, KQuery::DAY));
    CHECK_EQ(result.size(), total);
    CHECK_EQ(result[0],
             KRecord(Datetime(199012190000), 96.050, 99.980, 95.790, 99.980, 49.4, 1260));
    CHECK_EQ(result[10],
             KRecord(Datetime(199101030000), 128.840, 130.140, 128.840, 130.140, 9.3, 141));
    CHECK_EQ(result[5120], KRecord(Datetime(201112060000), 2326.660, 2331.892, 2310.155, 2325.905,
                                   4262559.5, 45917078));

    /** @arg 日线 start, 0, end, total + 1 */
    result = stock.getKRecordList(KQuery(0, total + 1, KQuery::DAY));
    CHECK_EQ(result.size(), total);
    CHECK_EQ(result[0],
             KRecord(Datetime(199012190000), 96.050, 99.980, 95.790, 99.980, 49.4, 1260));
    CHECK_EQ(result[10],
             KRecord(Datetime(199101030000), 128.840, 130.140, 128.840, 130.140, 9.3, 141));
    CHECK_EQ(result[5120], KRecord(Datetime(201112060000), 2326.660, 2331.892, 2310.155, 2325.905,
                                   4262559.5, 45917078));

    /** @arg 日线 start, 1, end, 10 */
    result = stock.getKRecordList(KQuery(1, 10, KQuery::DAY));
    CHECK_EQ(result.size(), 9);
    CHECK_EQ(result[0],
             KRecord(Datetime(199012200000), 104.300, 104.390, 99.980, 104.390, 8.4, 197));
    CHECK_EQ(result[1],
             KRecord(Datetime(199012210000), 109.070, 109.130, 103.730, 109.130, 1.6, 28));
    CHECK_EQ(result[8],
             KRecord(Datetime(199101020000), 127.610, 128.840, 127.610, 128.840, 5.9, 91));

    /** @arg 日线 start, end, 0 */
    result = stock.getKRecordList(KQuery(0, 0, KQuery::DAY));
    CHECK_EQ(result.size(), 0);

    /** @arg 日线 start, end, total */
    result = stock.getKRecordList(KQuery(total, total, KQuery::DAY));
    CHECK_EQ(result.size(), 0);

    /** @arg 日线 start > end */
    result = stock.getKRecordList(KQuery(2, 1, KQuery::DAY));
    CHECK_EQ(result.size(), 0);

    /** @arg 日线 start, 0, end, 1 */
    result = stock.getKRecordList(KQuery(0, 1, KQuery::DAY));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0],
             KRecord(Datetime(199012190000), 96.050, 99.980, 95.790, 99.980, 49.4, 1260));

    /** @arg 日线 start, total - 1, end, total */
    result = stock.getKRecordList(KQuery(total - 1, total, KQuery::DAY));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], KRecord(Datetime(201112060000), 2326.660, 2331.892, 2310.155, 2325.905,
                                4262559.5, 45917078));

    /** @arg 日线 start, total - 1, end, total + 1 */
    result = stock.getKRecordList(KQuery(total - 1, total + 1, KQuery::DAY));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], KRecord(Datetime(201112060000), 2326.660, 2331.892, 2310.155, 2325.905,
                                4262559.5, 45917078));

    ///=====================
    /// 测试周线
    ///=====================
    /** @arg 周线 start, 0, end, total */
    total = stock.getCount(KQuery::WEEK);
    result = stock.getKRecordList(KQuery(0, total, KQuery::WEEK));
    CHECK_EQ(result.size(), total);
    CHECK_EQ(result[0], KRecord(Datetime(199012210000), 96.05, 109.13, 95.79, 109.130, 59.4, 1485));
    CHECK_EQ(result[1],
             KRecord(Datetime(199012280000), 113.57, 126.45, 109.13, 126.450, 28.2, 321));
    CHECK_EQ(result[1058], KRecord(Datetime(201112090000), 2363.111, 2363.127, 2310.155, 2325.905,
                                   9126681.1, 98132048));

    /** @arg 周线 start, 0, end, total + 1 */
    result = stock.getKRecordList(KQuery(0, total + 1, KQuery::WEEK));
    CHECK_EQ(result.size(), total);
    CHECK_EQ(result[0], KRecord(Datetime(199012210000), 96.05, 109.13, 95.79, 109.130, 59.4, 1485));
    CHECK_EQ(result[1],
             KRecord(Datetime(199012280000), 113.57, 126.45, 109.13, 126.450, 28.2, 321));
    CHECK_EQ(result[1058], KRecord(Datetime(201112090000), 2363.111, 2363.127, 2310.155, 2325.905,
                                   9126681.1, 98132048));

    /** @arg 周线 start, 1, end, 10 */
    result = stock.getKRecordList(KQuery(1, 10, KQuery::WEEK));
    CHECK_EQ(result.size(), 9);
    CHECK_EQ(result[0],
             KRecord(Datetime(199012280000), 113.57, 126.45, 109.13, 126.450, 28.2, 321));
    CHECK_EQ(result[1],
             KRecord(Datetime(199101040000), 126.56, 131.44, 126.48, 131.440, 47.3, 730));
    CHECK_EQ(result[8],
             KRecord(Datetime(199102220000), 133.12, 134.87, 133.08, 134.870, 551, 9271));

    /** @arg 周线 start, end, 0 */
    result = stock.getKRecordList(KQuery(0, 0, KQuery::WEEK));
    CHECK_EQ(result.size(), 0);

    /** @arg 周线 start, end, total */
    result = stock.getKRecordList(KQuery(total, total, KQuery::WEEK));
    CHECK_EQ(result.size(), 0);

    /** @arg 周线 start > end */
    result = stock.getKRecordList(KQuery(2, 1, KQuery::WEEK));
    CHECK_EQ(result.size(), 0);

    /** @arg 周线 start, 0, end, 1 */
    result = stock.getKRecordList(KQuery(0, 1, KQuery::WEEK));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], KRecord(Datetime(199012210000), 96.05, 109.13, 95.79, 109.130, 59.4, 1485));

    /** @arg 周线 start, total - 1, end, total */
    result = stock.getKRecordList(KQuery(total - 1, total, KQuery::WEEK));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], KRecord(Datetime(201112090000), 2363.111, 2363.127, 2310.155, 2325.905,
                                9126681.1, 98132048));

    /** @arg 周线 start, total - 1, end, total + 1 */
    result = stock.getKRecordList(KQuery(total - 1, total + 1, KQuery::WEEK));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], KRecord(Datetime(201112090000), 2363.111, 2363.127, 2310.155, 2325.905,
                                9126681.1, 98132048));

    ///=====================
    /// 测试月线
    ///=====================
    /** @arg 月线 start, 0, end, total */
    total = stock.getCount(KQuery::MONTH);
    result = stock.getKRecordList(KQuery(0, total, KQuery::MONTH));
    CHECK_EQ(result.size(), total);
    CHECK_EQ(result[0], KRecord(Datetime(199012310000), 96.05, 127.61, 95.79, 127.610, 93.6, 1884));
    CHECK_EQ(result[1],
             KRecord(Datetime(199101310000), 127.61, 135.19, 127.61, 129.970, 3637.1, 67197));
    CHECK_EQ(result[252], KRecord(Datetime(201112310000), 2392.485, 2423.559, 2310.155, 2325.905,
                                  24378748.5, 256650941));

    /** @arg 月线 start, 0, end, total + 1 */
    result = stock.getKRecordList(KQuery(0, total + 1, KQuery::MONTH));
    CHECK_EQ(result.size(), total);
    CHECK_EQ(result[0], KRecord(Datetime(199012310000), 96.05, 127.61, 95.79, 127.610, 93.6, 1884));
    CHECK_EQ(result[1],
             KRecord(Datetime(199101310000), 127.61, 135.19, 127.61, 129.970, 3637.1, 67197));
    CHECK_EQ(result[252], KRecord(Datetime(201112310000), 2392.485, 2423.559, 2310.155, 2325.905,
                                  24378748.5, 256650941));

    /** @arg 月线 start, 1, end, 10 */
    result = stock.getKRecordList(KQuery(1, 10, KQuery::MONTH));
    CHECK_EQ(result.size(), 9);
    CHECK_EQ(result[0],
             KRecord(Datetime(199101310000), 127.61, 135.19, 127.61, 129.970, 3637.1, 67197));
    CHECK_EQ(result[1],
             KRecord(Datetime(199102280000), 129.5, 134.87, 128.06, 133.010, 3027.3, 50982));
    CHECK_EQ(result[8],
             KRecord(Datetime(199109300000), 180.08, 191.18, 178.43, 180.920, 19410.4, 286133));

    /** @arg 月线 start, end, 0 */
    result = stock.getKRecordList(KQuery(0, 0, KQuery::MONTH));
    CHECK_EQ(result.size(), 0);

    /** @arg 月线 start, end, total */
    result = stock.getKRecordList(KQuery(total, total, KQuery::MONTH));
    CHECK_EQ(result.size(), 0);

    /** @arg 月线 start > end */
    result = stock.getKRecordList(KQuery(2, 1, KQuery::MONTH));
    CHECK_EQ(result.size(), 0);

    /** @arg 月线 start, 0, end, 1 */
    result = stock.getKRecordList(KQuery(0, 1, KQuery::MONTH));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], KRecord(Datetime(199012310000), 96.05, 127.61, 95.79, 127.610, 93.6, 1884));

    /** @arg 月线 start, total - 1, end, total */
    result = stock.getKRecordList(KQuery(total - 1, total, KQuery::MONTH));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], KRecord(Datetime(201112310000), 2392.485, 2423.559, 2310.155, 2325.905,
                                24378748.5, 256650941));

    /** @arg 月线 start, total - 1, end, total + 1 */
    result = stock.getKRecordList(KQuery(total - 1, total + 1, KQuery::MONTH));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], KRecord(Datetime(201112310000), 2392.485, 2423.559, 2310.155, 2325.905,
                                24378748.5, 256650941));

    ///=====================
    /// 测试季线
    ///=====================
    /** @arg 季线 start, 0, end, total */
    total = stock.getCount(KQuery::QUARTER);
    result = stock.getKRecordList(KQuery(0, total, KQuery::QUARTER));
    CHECK_EQ(result.size(), total);
    CHECK_EQ(result[0], KRecord(Datetime(199012310000), 96.05, 127.61, 95.79, 127.610, 93.6, 1884));
    CHECK_EQ(result[1],
             KRecord(Datetime(199103310000), 127.61, 135.19, 120.11, 120.190, 8389.7, 142707));
    CHECK_EQ(result[84], KRecord(Datetime(201112310000), 2363.078, 2536.779, 2307.146, 2325.905,
                                 298278573.1, 3100147936));

    /** @arg 季线 start, 0, end, total + 1 */
    result = stock.getKRecordList(KQuery(0, total + 1, KQuery::QUARTER));
    CHECK_EQ(result.size(), total);
    CHECK_EQ(result[0], KRecord(Datetime(199012310000), 96.05, 127.61, 95.79, 127.610, 93.6, 1884));
    CHECK_EQ(result[1],
             KRecord(Datetime(199103310000), 127.61, 135.19, 120.11, 120.190, 8389.7, 142707));
    CHECK_EQ(result[84], KRecord(Datetime(201112310000), 2363.078, 2536.779, 2307.146, 2325.905,
                                 298278573.1, 3100147936));

    /** @arg 季线 start, 1, end, 10 */
    result = stock.getKRecordList(KQuery(1, 10, KQuery::QUARTER));
    CHECK_EQ(result.size(), 9);
    CHECK_EQ(result[0],
             KRecord(Datetime(199103310000), 127.61, 135.19, 120.11, 120.190, 8389.7, 142707));
    CHECK_EQ(result[1],
             KRecord(Datetime(199106300000), 120.69, 137.56, 104.96, 137.560, 12095.6, 222753));
    CHECK_EQ(result[8], KRecord(Datetime(199303310000), 784.13, 1558.95, 777.16, 925.910, 5003866.3,
                                26182794));

    /** @arg 季线 start, end, 0 */
    result = stock.getKRecordList(KQuery(0, 0, KQuery::QUARTER));
    CHECK_EQ(result.size(), 0);

    /** @arg 季线 start, end, total */
    result = stock.getKRecordList(KQuery(total, total, KQuery::QUARTER));
    CHECK_EQ(result.size(), 0);

    /** @arg 季线 start > end */
    result = stock.getKRecordList(KQuery(2, 1, KQuery::QUARTER));
    CHECK_EQ(result.size(), 0);

    /** @arg 季线 start, 0, end, 1 */
    result = stock.getKRecordList(KQuery(0, 1, KQuery::QUARTER));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], KRecord(Datetime(199012310000), 96.05, 127.61, 95.79, 127.610, 93.6, 1884));

    /** @arg 季线 start, total - 1, end, total */
    result = stock.getKRecordList(KQuery(total - 1, total, KQuery::QUARTER));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], KRecord(Datetime(201112310000), 2363.078, 2536.779, 2307.146, 2325.905,
                                298278573.1, 3100147936));

    /** @arg 季线 start, total - 1, end, total + 1 */
    result = stock.getKRecordList(KQuery(total - 1, total + 1, KQuery::QUARTER));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], KRecord(Datetime(201112310000), 2363.078, 2536.779, 2307.146, 2325.905,
                                298278573.1, 3100147936));

    ///=====================
    /// 测试半年线
    ///=====================
    /** @arg 半年线 start, 0, end, total */
    total = stock.getCount(KQuery::HALFYEAR);
    result = stock.getKRecordList(KQuery(0, total, KQuery::HALFYEAR));
    CHECK_EQ(result.size(), total);
    CHECK_EQ(result[0], KRecord(Datetime(199012310000), 96.05, 127.61, 95.79, 127.610, 93.6, 1884));
    CHECK_EQ(result[1],
             KRecord(Datetime(199106300000), 127.61, 137.56, 104.96, 137.560, 20485.3, 365460));
    CHECK_EQ(result[42], KRecord(Datetime(201112310000), 2767.834, 2826.959, 2307.146, 2325.905,
                                 842271699.6, 8187717694));

    /** @arg 半年线 start, 0, end, total + 1 */
    result = stock.getKRecordList(KQuery(0, total + 1, KQuery::HALFYEAR));
    CHECK_EQ(result.size(), total);
    CHECK_EQ(result[0], KRecord(Datetime(199012310000), 96.05, 127.61, 95.79, 127.610, 93.6, 1884));
    CHECK_EQ(result[1],
             KRecord(Datetime(199106300000), 127.61, 137.56, 104.96, 137.560, 20485.3, 365460));
    CHECK_EQ(result[42], KRecord(Datetime(201112310000), 2767.834, 2826.959, 2307.146, 2325.905,
                                 842271699.6, 8187717694));

    /** @arg 半年线 start, 1, end, 10 */
    result = stock.getKRecordList(KQuery(1, 10, KQuery::HALFYEAR));
    CHECK_EQ(result.size(), 9);
    CHECK_EQ(result[0],
             KRecord(Datetime(199106300000), 127.61, 137.56, 104.96, 137.560, 20485.3, 365460));
    CHECK_EQ(result[1],
             KRecord(Datetime(199112310000), 136.64, 292.75, 131.87, 292.750, 60078.8, 890458));
    CHECK_EQ(result[8], KRecord(Datetime(199506300000), 637.72, 926.41, 524.43, 630.580, 12294596.7,
                                164388191));

    /** @arg 半年线 start, end, 0 */
    result = stock.getKRecordList(KQuery(0, 0, KQuery::HALFYEAR));
    CHECK_EQ(result.size(), 0);

    /** @arg 半年线 start, end, total */
    result = stock.getKRecordList(KQuery(total, total, KQuery::HALFYEAR));
    CHECK_EQ(result.size(), 0);

    /** @arg 半年线 start > end */
    result = stock.getKRecordList(KQuery(2, 1, KQuery::HALFYEAR));
    CHECK_EQ(result.size(), 0);

    /** @arg 半年线 start, 0, end, 1 */
    result = stock.getKRecordList(KQuery(0, 1, KQuery::HALFYEAR));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], KRecord(Datetime(199012310000), 96.05, 127.61, 95.79, 127.610, 93.6, 1884));

    /** @arg 半年线 start, total - 1, end, total */
    result = stock.getKRecordList(KQuery(total - 1, total, KQuery::HALFYEAR));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], KRecord(Datetime(201112310000), 2767.834, 2826.959, 2307.146, 2325.905,
                                842271699.6, 8187717694));

    /** @arg 半年线 start, total - 1, end, total + 1 */
    result = stock.getKRecordList(KQuery(total - 1, total + 1, KQuery::HALFYEAR));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], KRecord(Datetime(201112310000), 2767.834, 2826.959, 2307.146, 2325.905,
                                842271699.6, 8187717694));

    ///=====================
    /// 测试年线
    ///=====================
    /** @arg 年线 start, 0, end, total */
    total = stock.getCount(KQuery::YEAR);
    result = stock.getKRecordList(KQuery(0, total, KQuery::YEAR));
    CHECK_EQ(result.size(), total);
    CHECK_EQ(result[0], KRecord(Datetime(199012310000), 96.05, 127.61, 95.79, 127.610, 93.6, 1884));
    CHECK_EQ(result[1],
             KRecord(Datetime(199112310000), 127.61, 292.75, 104.96, 292.750, 80564.1, 1255918));
    CHECK_EQ(result[21], KRecord(Datetime(201112310000), 2825.329, 3067.456, 2307.146, 2325.905,
                                 2308389935.3, 21513978512));

    /** @arg 年线 start, 0, end, total + 1 */
    result = stock.getKRecordList(KQuery(0, total + 1, KQuery::YEAR));
    CHECK_EQ(result.size(), total);
    CHECK_EQ(result[0], KRecord(Datetime(199012310000), 96.05, 127.61, 95.79, 127.610, 93.6, 1884));
    CHECK_EQ(result[1],
             KRecord(Datetime(199112310000), 127.61, 292.75, 104.96, 292.750, 80564.1, 1255918));
    CHECK_EQ(result[21], KRecord(Datetime(201112310000), 2825.329, 3067.456, 2307.146, 2325.905,
                                 2308389935.3, 21513978512));

    /** @arg 年线 start, 1, end, 10 */
    result = stock.getKRecordList(KQuery(1, 10, KQuery::YEAR));
    CHECK_EQ(result.size(), 9);
    CHECK_EQ(result[0],
             KRecord(Datetime(199112310000), 127.61, 292.75, 104.96, 292.750, 80564.1, 1255918));
    CHECK_EQ(result[1], KRecord(Datetime(199212310000), 293.74, 1429.01, 292.76, 780.390, 2497452.9,
                                18170898));
    CHECK_EQ(result[8], KRecord(Datetime(199912310000), 1144.888, 1756.184, 1047.833, 1366.580,
                                181932093.8, 2287638100));

    /** @arg 年线 start, end, 0 */
    result = stock.getKRecordList(KQuery(0, 0, KQuery::YEAR));
    CHECK_EQ(result.size(), 0);

    /** @arg 年线 start, end, total */
    result = stock.getKRecordList(KQuery(total, total, KQuery::YEAR));
    CHECK_EQ(result.size(), 0);

    /** @arg 年线 start > end */
    result = stock.getKRecordList(KQuery(2, 1, KQuery::YEAR));
    CHECK_EQ(result.size(), 0);

    /** @arg 年线 start, 0, end, 1 */
    result = stock.getKRecordList(KQuery(0, 1, KQuery::YEAR));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], KRecord(Datetime(199012310000), 96.05, 127.61, 95.79, 127.610, 93.6, 1884));

    /** @arg 年线 start, total - 1, end, total */
    result = stock.getKRecordList(KQuery(total - 1, total, KQuery::YEAR));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], KRecord(Datetime(201112310000), 2825.329, 3067.456, 2307.146, 2325.905,
                                2308389935.3, 21513978512));

    /** @arg 年线 start, total - 1, end, total + 1 */
    result = stock.getKRecordList(KQuery(total - 1, total + 1, KQuery::YEAR));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], KRecord(Datetime(201112310000), 2825.329, 3067.456, 2307.146, 2325.905,
                                2308389935.3, 21513978512));

    ///=====================
    /// 测试分钟线
    ///=====================
    /** @arg 分钟线 start, 0, end, total */
    total = stock.getCount(KQuery::MIN);
    result = stock.getKRecordList(KQuery(0, total, KQuery::MIN));
    CHECK_EQ(result.size(), total);
    CHECK_EQ(result[0], KRecord(Datetime(200001040931), 1366.58, 1368.692, 1366.579, 1368.692,
                                4124880, 191158));
    CHECK_EQ(result[1],
             KRecord(Datetime(200001040932), 1368.3, 1368.491, 1368.3, 1368.491, 2783200, 106916));
    CHECK_EQ(result[682822],
             KRecord(Datetime(201112061500), 2325.38, 2326.1, 2324.6, 2325.900, 5215150, 566312));

    /** @arg 分钟线 start, 0, end, total + 1 */
    result = stock.getKRecordList(KQuery(0, total + 1, KQuery::MIN));
    CHECK_EQ(result.size(), total);
    CHECK_EQ(result[0], KRecord(Datetime(200001040931), 1366.58, 1368.692, 1366.579, 1368.692,
                                4124880, 191158));
    CHECK_EQ(result[1],
             KRecord(Datetime(200001040932), 1368.3, 1368.491, 1368.3, 1368.491, 2783200, 106916));
    CHECK_EQ(result[682822],
             KRecord(Datetime(201112061500), 2325.38, 2326.1, 2324.6, 2325.900, 5215150, 566312));

    /** @arg 分钟线 start, 1, end, 10 */
    result = stock.getKRecordList(KQuery(1, 10, KQuery::MIN));
    CHECK_EQ(result.size(), 9);
    CHECK_EQ(result[0],
             KRecord(Datetime(200001040932), 1368.3, 1368.491, 1368.3, 1368.491, 2783200, 106916));
    CHECK_EQ(result[1], KRecord(Datetime(200001040933), 1368.859, 1369.474, 1368.859, 1369.308,
                                1060260, 37789));
    CHECK_EQ(result[8], KRecord(Datetime(200001040940), 1367.651, 1367.662, 1367.389, 1367.389,
                                1335200, 41924));

    /** @arg 分钟线 start, end, 0 */
    result = stock.getKRecordList(KQuery(0, 0, KQuery::MIN));
    CHECK_EQ(result.size(), 0);

    /** @arg 分钟线 start, end, total */
    result = stock.getKRecordList(KQuery(total, total, KQuery::MIN));
    CHECK_EQ(result.size(), 0);

    /** @arg 分钟线 start > end */
    result = stock.getKRecordList(KQuery(2, 1, KQuery::MIN));
    CHECK_EQ(result.size(), 0);

    /** @arg 分钟线 start, 0, end, 1 */
    result = stock.getKRecordList(KQuery(0, 1, KQuery::MIN));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], KRecord(Datetime(200001040931), 1366.58, 1368.692, 1366.579, 1368.692,
                                4124880, 191158));

    /** @arg 分钟线 start, total - 1, end, total */
    result = stock.getKRecordList(KQuery(total - 1, total, KQuery::MIN));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0],
             KRecord(Datetime(201112061500), 2325.38, 2326.1, 2324.6, 2325.900, 5215150, 566312));

    /** @arg 分钟线 start, total - 1, end, total + 1 */
    result = stock.getKRecordList(KQuery(total - 1, total + 1, KQuery::MIN));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0],
             KRecord(Datetime(201112061500), 2325.38, 2326.1, 2324.6, 2325.900, 5215150, 566312));

    ///=====================
    /// 测试5分钟线
    ///=====================
    /** @arg 5分钟线 start, 0, end, total */
    total = stock.getCount(KQuery::MIN5);
    result = stock.getKRecordList(KQuery(0, total, KQuery::MIN5));
    CHECK_EQ(result.size(), total);
    CHECK_EQ(result[0], KRecord(Datetime(200001040935), 1366.58, 1369.68, 1366.579, 1369.187,
                                106611.5, 438171));
    CHECK_EQ(result[1], KRecord(Datetime(200001040940), 1369.187, 1369.371, 1367.389, 1367.389,
                                70687.3, 251473));
    CHECK_EQ(result[133979], KRecord(Datetime(201112061500), 2323.059, 2325.931, 2323.059, 2325.905,
                                     176347.1, 1873675));

    /** @arg 5分钟线 start, 0, end, total + 1 */
    result = stock.getKRecordList(KQuery(0, total + 1, KQuery::MIN5));
    CHECK_EQ(result.size(), total);
    CHECK_EQ(result[0], KRecord(Datetime(200001040935), 1366.58, 1369.68, 1366.579, 1369.187,
                                106611.5, 438171));
    CHECK_EQ(result[1], KRecord(Datetime(200001040940), 1369.187, 1369.371, 1367.389, 1367.389,
                                70687.3, 251473));
    CHECK_EQ(result[133979], KRecord(Datetime(201112061500), 2323.059, 2325.931, 2323.059, 2325.905,
                                     176347.1, 1873675));

    /** @arg 5分钟线 start, 1, end, 10 */
    result = stock.getKRecordList(KQuery(1, 10, KQuery::MIN5));
    CHECK_EQ(result.size(), 9);
    CHECK_EQ(result[0], KRecord(Datetime(200001040940), 1369.187, 1369.371, 1367.389, 1367.389,
                                70687.3, 251473));
    CHECK_EQ(result[1], KRecord(Datetime(200001040945), 1367.389, 1367.389, 1364.645, 1364.763,
                                73577.1, 223346));
    CHECK_EQ(result[8], KRecord(Datetime(200001041020), 1366.043, 1366.458, 1365.946, 1366.458,
                                111601.7, 205190));

    /** @arg 5分钟线 start, end, 0 */
    result = stock.getKRecordList(KQuery(0, 0, KQuery::MIN5));
    CHECK_EQ(result.size(), 0);

    /** @arg 5分钟线 start, end, total */
    result = stock.getKRecordList(KQuery(total, total, KQuery::MIN5));
    CHECK_EQ(result.size(), 0);

    /** @arg 5分钟线 start > end */
    result = stock.getKRecordList(KQuery(2, 1, KQuery::MIN5));
    CHECK_EQ(result.size(), 0);

    /** @arg 5分钟线 start, 0, end, 1 */
    result = stock.getKRecordList(KQuery(0, 1, KQuery::MIN5));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], KRecord(Datetime(200001040935), 1366.58, 1369.68, 1366.579, 1369.187,
                                106611.5, 438171));

    /** @arg 5分钟线 start, total - 1, end, total */
    result = stock.getKRecordList(KQuery(total - 1, total, KQuery::MIN5));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], KRecord(Datetime(201112061500), 2323.059, 2325.931, 2323.059, 2325.905,
                                176347.1, 1873675));

    /** @arg 5分钟线 start, total - 1, end, total + 1 */
    result = stock.getKRecordList(KQuery(total - 1, total + 1, KQuery::MIN5));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], KRecord(Datetime(201112061500), 2323.059, 2325.931, 2323.059, 2325.905,
                                176347.1, 1873675));

    ///=====================
    /// 测试15分钟线
    ///=====================
    /** @arg 15分钟线 start, 0, end, total */
    total = stock.getCount(KQuery::MIN15);
    result = stock.getKRecordList(KQuery(0, total, KQuery::MIN15));
    CHECK_EQ(result.size(), total);
    CHECK_EQ(result[0], KRecord(Datetime(200001040945), 1366.58, 1369.68, 1364.645, 1364.763,
                                250875.9, 912990));
    CHECK_EQ(result[1], KRecord(Datetime(200001041000), 1364.763, 1364.763, 1361.459, 1361.459,
                                204277.1, 457452));
    CHECK_EQ(result[44749], KRecord(Datetime(201112061500), 2323.068, 2325.931, 2320.819, 2325.905,
                                    402339.3, 4060591));

    /** @arg 15分钟线 start, 0, end, total + 1 */
    result = stock.getKRecordList(KQuery(0, total + 1, KQuery::MIN15));
    CHECK_EQ(result.size(), total);
    CHECK_EQ(result[0], KRecord(Datetime(200001040945), 1366.58, 1369.68, 1364.645, 1364.763,
                                250875.9, 912990));
    CHECK_EQ(result[1], KRecord(Datetime(200001041000), 1364.763, 1364.763, 1361.459, 1361.459,
                                204277.1, 457452));
    CHECK_EQ(result[44749], KRecord(Datetime(201112061500), 2323.068, 2325.931, 2320.819, 2325.905,
                                    402339.3, 4060591));

    /** @arg 15分钟线 start, 1, end, 10 */
    result = stock.getKRecordList(KQuery(1, 10, KQuery::MIN15));
    CHECK_EQ(result.size(), 9);
    CHECK_EQ(result[0], KRecord(Datetime(200001041000), 1364.763, 1364.763, 1361.459, 1361.459,
                                204277.1, 457452));
    CHECK_EQ(result[1], KRecord(Datetime(200001041015), 1361.558, 1366.093, 1361.295, 1365.927,
                                307531.8, 548313));
    CHECK_EQ(result[8], KRecord(Datetime(200001041330), 1399.56, 1399.664, 1396.679, 1399.325,
                                407799.9, 554965));

    /** @arg 15分钟线 start, end, 0 */
    result = stock.getKRecordList(KQuery(0, 0, KQuery::MIN15));
    CHECK_EQ(result.size(), 0);

    /** @arg 15分钟线 start, end, total */
    result = stock.getKRecordList(KQuery(total, total, KQuery::MIN15));
    CHECK_EQ(result.size(), 0);

    /** @arg 15分钟线 start > end */
    result = stock.getKRecordList(KQuery(2, 1, KQuery::MIN15));
    CHECK_EQ(result.size(), 0);

    /** @arg 15分钟线 start, 0, end, 1 */
    result = stock.getKRecordList(KQuery(0, 1, KQuery::MIN15));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], KRecord(Datetime(200001040945), 1366.58, 1369.68, 1364.645, 1364.763,
                                250875.9, 912990));

    /** @arg 15分钟线 start, total - 1, end, total */
    result = stock.getKRecordList(KQuery(total - 1, total, KQuery::MIN15));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], KRecord(Datetime(201112061500), 2323.068, 2325.931, 2320.819, 2325.905,
                                402339.3, 4060591));

    /** @arg 15分钟线 start, total - 1, end, total + 1 */
    result = stock.getKRecordList(KQuery(total - 1, total + 1, KQuery::MIN15));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], KRecord(Datetime(201112061500), 2323.068, 2325.931, 2320.819, 2325.905,
                                402339.3, 4060591));

    ///=====================
    /// 测试30分钟线
    ///=====================
    /** @arg 30分钟线 start, 0, end, total */
    total = stock.getCount(KQuery::MIN30);
    result = stock.getKRecordList(KQuery(0, total, KQuery::MIN30));
    CHECK_EQ(result.size(), total);
    CHECK_EQ(result[0], KRecord(Datetime(200001041000), 1366.58, 1369.68, 1361.459, 1361.459,
                                455153, 1370442));
    CHECK_EQ(result[1], KRecord(Datetime(200001041030), 1361.558, 1367.683, 1361.295, 1367.683,
                                586634.5, 1000294));
    CHECK_EQ(result[22379], KRecord(Datetime(201112061500), 2327.919, 2327.919, 2320.819, 2325.905,
                                    660297.5, 6415054));

    /** @arg 30分钟线 start, 0, end, total + 1 */
    result = stock.getKRecordList(KQuery(0, total + 1, KQuery::MIN30));
    CHECK_EQ(result.size(), total);
    CHECK_EQ(result[0], KRecord(Datetime(200001041000), 1366.58, 1369.68, 1361.459, 1361.459,
                                455153, 1370442));
    CHECK_EQ(result[1], KRecord(Datetime(200001041030), 1361.558, 1367.683, 1361.295, 1367.683,
                                586634.5, 1000294));
    CHECK_EQ(result[22379], KRecord(Datetime(201112061500), 2327.919, 2327.919, 2320.819, 2325.905,
                                    660297.5, 6415054));

    /** @arg 30分钟线 start, 1, end, 10 */
    result = stock.getKRecordList(KQuery(1, 10, KQuery::MIN30));
    CHECK_EQ(result.size(), 9);
    CHECK_EQ(result[0], KRecord(Datetime(200001041030), 1361.558, 1367.683, 1361.295, 1367.683,
                                586634.5, 1000294));
    CHECK_EQ(result[1], KRecord(Datetime(200001041100), 1367.667, 1378.543, 1367.6, 1377.098,
                                622250.5, 1248350));
    CHECK_EQ(result[8], KRecord(Datetime(200001051030), 1409.486, 1421.612, 1409.486, 1421.139,
                                1397089.8, 1777404));

    /** @arg 30分钟线 start, end, 0 */
    result = stock.getKRecordList(KQuery(0, 0, KQuery::MIN30));
    CHECK_EQ(result.size(), 0);

    /** @arg 30分钟线 start, end, total */
    result = stock.getKRecordList(KQuery(total, total, KQuery::MIN30));
    CHECK_EQ(result.size(), 0);

    /** @arg 30分钟线 start > end */
    result = stock.getKRecordList(KQuery(2, 1, KQuery::MIN30));
    CHECK_EQ(result.size(), 0);

    /** @arg 30分钟线 start, 0, end, 1 */
    result = stock.getKRecordList(KQuery(0, 1, KQuery::MIN30));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], KRecord(Datetime(200001041000), 1366.58, 1369.68, 1361.459, 1361.459,
                                455153, 1370442));

    /** @arg 30分钟线 start, total - 1, end, total */
    result = stock.getKRecordList(KQuery(total - 1, total, KQuery::MIN30));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], KRecord(Datetime(201112061500), 2327.919, 2327.919, 2320.819, 2325.905,
                                660297.5, 6415054));

    /** @arg 30分钟线 start, total - 1, end, total + 1 */
    result = stock.getKRecordList(KQuery(total - 1, total + 1, KQuery::MIN30));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], KRecord(Datetime(201112061500), 2327.919, 2327.919, 2320.819, 2325.905,
                                660297.5, 6415054));

    ///=====================
    /// 测试60分钟线
    ///=====================
    /** @arg 60分钟线 start, 0, end, total */
    total = stock.getCount(KQuery::MIN60);
    result = stock.getKRecordList(KQuery(0, total, KQuery::MIN60));
    CHECK_EQ(result.size(), total);
    CHECK_EQ(result[0], KRecord(Datetime(200001041030), 1366.58, 1369.68, 1361.295, 1367.683,
                                1041787.5, 2370736));
    CHECK_EQ(result[1], KRecord(Datetime(200001041130), 1367.667, 1387.853, 1367.6, 1387.853,
                                1411191.4, 2172479));
    CHECK_EQ(result[11192], KRecord(Datetime(201112061500), 2326.036, 2331.378, 2320.819, 2325.905,
                                    1332210, 11886356));

    /** @arg 60分钟线 start, 0, end, total + 1 */
    result = stock.getKRecordList(KQuery(0, total + 1, KQuery::MIN60));
    CHECK_EQ(result.size(), total);
    CHECK_EQ(result[0], KRecord(Datetime(200001041030), 1366.58, 1369.68, 1361.295, 1367.683,
                                1041787.5, 2370736));
    CHECK_EQ(result[1], KRecord(Datetime(200001041130), 1367.667, 1387.853, 1367.6, 1387.853,
                                1411191.4, 2172479));
    CHECK_EQ(result[11192], KRecord(Datetime(201112061500), 2326.036, 2331.378, 2320.819, 2325.905,
                                    1332210, 11886356));

    /** @arg 60分钟线 start, 1, end, 10 */
    result = stock.getKRecordList(KQuery(1, 10, KQuery::MIN60));
    CHECK_EQ(result.size(), 9);
    CHECK_EQ(result[0], KRecord(Datetime(200001041130), 1367.667, 1387.853, 1367.6, 1387.853,
                                1411191.4, 2172479));
    CHECK_EQ(result[1], KRecord(Datetime(200001041400), 1389.996, 1403.996, 1389.97, 1400.685,
                                1740709.6, 2348753));
    CHECK_EQ(result[8], KRecord(Datetime(200001061130), 1420.098, 1420.098, 1410.706, 1411.387,
                                1291509.5, 1378193));

    /** @arg 60分钟线 start, end, 0 */
    result = stock.getKRecordList(KQuery(0, 0, KQuery::MIN60));
    CHECK_EQ(result.size(), 0);

    /** @arg 60分钟线 start, end, total */
    result = stock.getKRecordList(KQuery(total, total, KQuery::MIN60));
    CHECK_EQ(result.size(), 0);

    /** @arg 60分钟线 start > end */
    result = stock.getKRecordList(KQuery(2, 1, KQuery::MIN60));
    CHECK_EQ(result.size(), 0);

    /** @arg 60分钟线 start, 0, end, 1 */
    result = stock.getKRecordList(KQuery(0, 1, KQuery::MIN60));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], KRecord(Datetime(200001041030), 1366.58, 1369.68, 1361.295, 1367.683,
                                1041787.5, 2370736));

    /** @arg 60分钟线 start, total - 1, end, total */
    result = stock.getKRecordList(KQuery(total - 1, total, KQuery::MIN60));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], KRecord(Datetime(201112061500), 2326.036, 2331.378, 2320.819, 2325.905,
                                1332210, 11886356));

    /** @arg 60分钟线 start, total - 1, end, total + 1 */
    result = stock.getKRecordList(KQuery(total - 1, total + 1, KQuery::MIN60));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], KRecord(Datetime(201112061500), 2326.036, 2331.378, 2320.819, 2325.905,
                                1332210, 11886356));

    MEMORY_CHECK;
}

/** @par 检测点 */
TEST_CASE("test_Stock_getKRecord_By_Date") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KRecord record;

    /// 测试日线
    ///===================================

    /** @arg 日期小于第一条记录 */
    record = stock.getKRecord(Datetime(199012180000), KQuery::DAY);
    CHECK_EQ(record, Null<KRecord>());

    /** @arg 日期等于第一条记录 */
    record = stock.getKRecord(Datetime(199012190000), KQuery::DAY);
    CHECK_EQ(record, KRecord(Datetime(199012190000), 96.0500, 99.9800, 95.7900, 99.9800, 49.4000,
                             1260.0000));

    /** @arg 中间存在数据的随机一条数据 */
    record = stock.getKRecord(Datetime(199502150000), KQuery::DAY);
    CHECK_EQ(record, KRecord(Datetime(199502150000), 547.8300, 548.5900, 542.5000, 542.8400,
                             11889.3000, 232116.0000));

    /** @arg 中间不存在对应数据的随机一条数据 */
    record = stock.getKRecord(Datetime(199502180000), KQuery::DAY);
    CHECK_EQ(record, Null<KRecord>());

    /** @arg 日期等于最后一条记录 */
    record = stock.getKRecord(Datetime(201112060000), KQuery::DAY);
    CHECK_EQ(record, KRecord(Datetime(201112060000), 2326.6600, 2331.8920, 2310.1550, 2325.9050,
                             4262559.5000, 45917078.0000));

    /** @arg 日期大于最后一条记录 */
    record = stock.getKRecord(Datetime(201112070000), KQuery::DAY);
    CHECK_EQ(record, Null<KRecord>());

    ///===================================
    /// 测试周线
    ///===================================

    /** @arg 日期小于第一条记录 */
    record = stock.getKRecord(Datetime(199012160000), KQuery::WEEK);
    CHECK_EQ(record, Null<KRecord>());

    /** @arg 日期等于第一条记录 */
    record = stock.getKRecord(Datetime(199012210000), KQuery::WEEK);
    CHECK_EQ(record, KRecord(Datetime(199012210000), 96.0500, 109.1300, 95.7900, 109.1300, 59.4000,
                             1485.0000));

    /** @arg 中间存在数据的随机一条数据 */
    record = stock.getKRecord(Datetime(199103290000), KQuery::WEEK);
    CHECK_EQ(record, KRecord(Datetime(199103290000), 122.1200, 122.6200, 120.1100, 120.1900,
                             409.0000, 5421.0000));

    /** @arg 中间不存在对应数据的随机一条数据 */
    record = stock.getKRecord(Datetime(199103120000), KQuery::WEEK);
    CHECK_EQ(record, Null<KRecord>());

    /** @arg 日期等于最后一条记录 */
    record = stock.getKRecord(Datetime(201112090000), KQuery::WEEK);
    CHECK_EQ(record, KRecord(Datetime(201112090000), 2363.1110, 2363.1270, 2310.1550, 2325.9050,
                             9126681.1000, 98132048.0000));

    /** @arg 日期大于最后一条记录 */
    record = stock.getKRecord(Datetime(201112100000), KQuery::WEEK);
    CHECK_EQ(record, Null<KRecord>());

    ///===================================
    /// 测试月线
    ///===================================

    /** @arg 日期小于第一条记录 */
    record = stock.getKRecord(Datetime(199011300000), KQuery::MONTH);
    CHECK_EQ(record, Null<KRecord>());

    /** @arg 日期等于第一条记录 */
    record = stock.getKRecord(Datetime(199012310000), KQuery::MONTH);
    CHECK_EQ(record, KRecord(Datetime(199012310000), 96.0500, 127.6100, 95.7900, 127.6100, 93.6000,
                             1884.0000));

    /** @arg 中间存在数据的随机一条数据 */
    record = stock.getKRecord(Datetime(199208310000), KQuery::MONTH);
    CHECK_EQ(record, KRecord(Datetime(199208310000), 1034.7000, 1060.3600, 627.3600, 823.2700,
                             286256.7000, 2040679.0000));

    /** @arg 中间不存在对应数据的随机一条数据 */
    record = stock.getKRecord(Datetime(199103120000), KQuery::MONTH);
    CHECK_EQ(record, Null<KRecord>());

    /** @arg 日期等于最后一条记录 */
    record = stock.getKRecord(Datetime(201112310000), KQuery::MONTH);
    CHECK_EQ(record, KRecord(Datetime(201112310000), 2392.4850, 2423.5590, 2310.1550, 2325.9050,
                             24378748.5000, 256650941.0000));

    /** @arg 日期大于最后一条记录 */
    record = stock.getKRecord(Datetime(201201010000), KQuery::MONTH);
    CHECK_EQ(record, Null<KRecord>());

    ///===================================
    /// 测试15分钟线
    ///===================================

    /** @arg 日期小于第一条记录 */
    record = stock.getKRecord(Datetime(200001040944), KQuery::MIN15);
    CHECK_EQ(record, Null<KRecord>());

    /** @arg 日期等于第一条记录 */
    record = stock.getKRecord(Datetime(200001040945), KQuery::MIN15);
    CHECK_EQ(record, KRecord(Datetime(200001040945), 1366.5800, 1369.6800, 1364.6450, 1364.7630,
                             250875.9000, 912990.0000));

    /** @arg 中间存在数据的随机一条数据 */
    record = stock.getKRecord(Datetime(200001071045), KQuery::MIN15);
    CHECK_EQ(record, KRecord(Datetime(200001071045), 1482.5210, 1490.0070, 1481.9910, 1485.2770,
                             1030800.2000, 1663302.0000));

    /** @arg 中间不存在对应数据的随机一条数据 */
    record = stock.getKRecord(Datetime(200001071044), KQuery::MIN15);
    CHECK_EQ(record, Null<KRecord>());

    /** @arg 日期等于最后一条记录 */
    record = stock.getKRecord(Datetime(201112061500), KQuery::MIN15);
    CHECK_EQ(record, KRecord(Datetime(201112061500), 2323.0680, 2325.9310, 2320.8190, 2325.9050,
                             402339.3000, 4060591.0000));

    /** @arg 日期大于最后一条记录 */
    record = stock.getKRecord(Datetime(201112061501), KQuery::MIN15);
    CHECK_EQ(record, Null<KRecord>());

    MEMORY_CHECK;
}

/** @par 检测点 */
TEST_CASE("test_Stock_getMarketValue") {
    StockManager& sm = StockManager::instance();
    Stock stock;
    price_t result = 0.0;

    /** @arg stock is null */
    result = stock.getMarketValue(Datetime::now(), KQuery::DAY);
    CHECK_EQ(result, 0.0);

    /** @arg 正常日线 */
    stock = sm.getStock("sh600000");
    result = stock.getMarketValue(Datetime::now(), KQuery::DAY);
    CHECK_EQ(result, 8.73);

    result = stock.getMarketValue(Datetime(201112010000), KQuery::DAY);
    CHECK_EQ(result, 8.81);

    result = stock.getMarketValue(Datetime(201112020000), KQuery::DAY);
    CHECK_EQ(result, 8.80);

    result = stock.getMarketValue(Datetime(201112030000), KQuery::DAY);
    CHECK_EQ(result, 8.80);

    result = stock.getMarketValue(Datetime(201112040000), KQuery::DAY);
    CHECK_EQ(result, 8.80);

    result = stock.getMarketValue(Datetime(201112050000), KQuery::DAY);
    CHECK_EQ(result, 8.80);

    result = stock.getMarketValue(Datetime(201112060000), KQuery::DAY);
    CHECK_EQ(result, 8.73);

    /** @arg 正常5分钟线 */
    result = stock.getMarketValue(Datetime::now(), KQuery::MIN5);
    CHECK_EQ(result, 8.73);

    result = stock.getMarketValue(Datetime(201111211344), KQuery::MIN5);
    CHECK_LT(std::fabs(result - 8.70), 0.001);

    result = stock.getMarketValue(Datetime(201111211345), KQuery::MIN5);
    CHECK_EQ(result, 8.71);

    result = stock.getMarketValue(Datetime(201111211346), KQuery::MIN5);
    CHECK_EQ(result, 8.71);

    result = stock.getMarketValue(Datetime(201111211350), KQuery::MIN5);
    CHECK_LT(std::fabs(result - 8.70), 0.001);

    MEMORY_CHECK;
}

/** @par 检测点 */
TEST_CASE("test_Stock_id_map") {
    /** @arg 两个为空的stock */
    Stock x1, x2;
    CHECK_EQ(x1.id(), x2.id());

    /** @arg 两个相同的stock */
    StockManager& sm = StockManager::instance();
    x1 = sm.getStock("sh600001");
    x2 = sm.getStock("sh600001");
    CHECK_EQ(x1.id(), x2.id());

    /** @arg stock作为map的键值 */
    map<Stock, int> dict;
    Stock x3 = sm.getStock("sh600000");
    Stock x4 = x1;
    dict[Stock()] = 0;
    dict[x1] = 1;
    dict[x3] = 3;
    CHECK_NE(dict.count(x1), 0);
    CHECK_NE(dict.count(x2), 0);
    CHECK_NE(dict.count(x3), 0);
    CHECK_EQ(dict[x1], 1);
    CHECK_EQ(dict[x2], 1);
    CHECK_EQ(dict[Stock()], 0);
    CHECK_EQ(dict[x3], 3);
    CHECK_EQ(dict[x4], 1);

    MEMORY_CHECK;
}

/** @par 检测点 */
TEST_CASE("test_Stock_getFinanceInfo") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm.getStock("sh600000");
    Parameter f = stk.getFinanceInfo();

    CHECK_EQ(f.get<string>("market"), "SH");
    CHECK_EQ(f.get<string>("code"), "600000");
    CHECK_EQ(f.get<int>("updated_date"), 20190326);
    CHECK_EQ(f.get<int>("ipo_date"), 19991110);

    CHECK_EQ(f.get<price_t>("liutongguben"), doctest::Approx(28103765000.0).epsilon(0.00001));
    CHECK_EQ(f.get<price_t>("province"), doctest::Approx(16).epsilon(0.00001));
    CHECK_EQ(f.get<price_t>("industry"), doctest::Approx(1).epsilon(0.00001));
    CHECK_EQ(f.get<price_t>("zongguben"), doctest::Approx(29352080000.0).epsilon(0.00001));
    CHECK_EQ(f.get<price_t>("guojiagu"), doctest::Approx(1801990000.0).epsilon(0.00001));
    CHECK_EQ(f.get<price_t>("faqirenfarengu"), doctest::Approx(542580000000.0).epsilon(0.00001));
    CHECK_EQ(f.get<price_t>("farengu"), doctest::Approx(1686190080000.0).epsilon(0.00001));
    CHECK_EQ(f.get<price_t>("bgu"), doctest::Approx(0.0).epsilon(0.00001));
    CHECK_EQ(f.get<price_t>("hgu"), doctest::Approx(0.0).epsilon(0.00001));
    CHECK_EQ(f.get<price_t>("zhigonggu"), doctest::Approx(18500.00023841858).epsilon(0.00001));
    CHECK_EQ(f.get<price_t>("zongzichan"), doctest::Approx(62896056320000.0).epsilon(0.00001));
    CHECK_EQ(f.get<price_t>("liudongzichan"), doctest::Approx(0.0).epsilon(0.00001));
    CHECK_EQ(f.get<price_t>("gudingzichan"), doctest::Approx(264920000000.0).epsilon(0.00001));
    CHECK_EQ(f.get<price_t>("wuxingzichan"), doctest::Approx(99620000000.0).epsilon(0.00001));
    CHECK_EQ(f.get<price_t>("gudongrenshu"), doctest::Approx(190753.0).epsilon(0.00001));
    CHECK_EQ(f.get<price_t>("liudongfuzhai"), doctest::Approx(0.0).epsilon(0.00001));
    CHECK_EQ(f.get<price_t>("changqifuzhai"), doctest::Approx(68180000000.0).epsilon(0.00001));
    CHECK_EQ(f.get<price_t>("zibengongjijin"), doctest::Approx(817600000000.0).epsilon(0.00001));
    CHECK_EQ(f.get<price_t>("jingzichan"), doctest::Approx(4715619840000.0).epsilon(0.00001));
    CHECK_EQ(f.get<price_t>("zhuyingshouru"), doctest::Approx(1715420000000.0).epsilon(0.00001));
    CHECK_EQ(f.get<price_t>("zhuyinglirun"), doctest::Approx(0.0).epsilon(0.00001));
    CHECK_EQ(f.get<price_t>("yingshouzhangkuan"), doctest::Approx(0.0).epsilon(0.00001));
    CHECK_EQ(f.get<price_t>("yingyelirun"), doctest::Approx(653430000000.0).epsilon(0.00001));
    CHECK_EQ(f.get<price_t>("touzishouyi"), doctest::Approx(147650000000.0).epsilon(0.00001));
    CHECK_EQ(f.get<price_t>("jingyingxianjinliu"),
             doctest::Approx(-3383600000000.0).epsilon(0.00001));
    CHECK_EQ(f.get<price_t>("zongxianjinliu"), doctest::Approx(374319920000.0).epsilon(0.00001));
    CHECK_EQ(f.get<price_t>("cunhuo"), doctest::Approx(0.0).epsilon(0.00001));
    CHECK_EQ(f.get<price_t>("lirunzonghe"), doctest::Approx(652840000000.0).epsilon(0.00001));
    CHECK_EQ(f.get<price_t>("shuihoulirun"), doctest::Approx(565150000000.0).epsilon(0.00001));
    CHECK_EQ(f.get<price_t>("jinglirun"), doctest::Approx(559140000000.0).epsilon(0.00001));
    CHECK_EQ(f.get<price_t>("weifenpeilirun"), doctest::Approx(1402080000000.0).epsilon(0.00001));
    CHECK_EQ(f.get<price_t>("meigujingzichan"),
             doctest::Approx(15.050000190734863).epsilon(0.00001));
    CHECK_EQ(f.get<price_t>("baoliu2"), doctest::Approx(12.0).epsilon(0.00001));

    MEMORY_CHECK;
}

/** @} */
