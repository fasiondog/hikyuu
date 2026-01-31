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
#include <hikyuu/plugin/interface/plugins.h>
#include <hikyuu/plugin/device.h>
#include "../plugin_valid.h"

using namespace hku;

/**
 * @defgroup test_hikyuu_Stock_extra test_hikyuu_Stock_extra
 * @ingroup test_hikyuu_base_suite
 * @{
 */

static bool _pluginValid() {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<HkuExtraPluginInterface>(HKU_PLUGIN_HKU_EXTRA);
    return plugin && isValidLicense();
}

/** @par 检测点 */
TEST_CASE("test_Stock_extra_ktype") {
    HKU_INFO("0");
    HKU_IF_RETURN(!_pluginValid(), void());
    HKU_INFO("1");

    /** @arg 检查扩展K线类型对应的分钟数 */
    CHECK_EQ(KQuery::getKTypeInMin(KQuery::DAY) * 3, KQuery::getKTypeInMin(KQuery::DAY3));

    HKU_INFO("2");

    /** @arg 检测 nbar 类别的扩展K线数据 */
    auto stk = getStock("sh000001");
    auto kday1 = stk.getKData(KQuery(0));
    auto kday3 = stk.getKData(KQuery(0, Null<int64_t>(), KQuery::DAY3));
    REQUIRE(kday1.size() % 3 == 0);
    CHECK_EQ(kday1.size(), kday3.size() * 3);
    CHECK_EQ(kday3.size(), stk.getCount(KQuery::DAY3));

    CHECK_EQ(kday3[0].datetime, kday1[2].datetime);
    CHECK_EQ(kday3[0].openPrice, kday1[0].openPrice);
    CHECK_EQ(kday3[0].closePrice, kday1[2].closePrice);
    price_t high = kday1[0].highPrice;
    price_t low = std::numeric_limits<price_t>::max();
    price_t sum_amount = 0;
    price_t sum_volume = 0;
    for (size_t i = 0; i < 3; i++) {
        if (kday1[i].highPrice > high) {
            high = kday1[i].highPrice;
        }
        if (kday1[i].lowPrice < low) {
            low = kday1[i].lowPrice;
        }
        sum_amount += kday1[i].transAmount;
        sum_volume += kday1[i].transCount;
    }
    CHECK_EQ(kday3[0].highPrice, high);
    CHECK_EQ(kday3[0].lowPrice, low);
    CHECK_EQ(kday3[0].transAmount, doctest::Approx(sum_amount).epsilon(0.001));
    CHECK_EQ(kday3[0].transCount, doctest::Approx(sum_volume).epsilon(0.001));

    CHECK_EQ(kday3[0], stk.getKRecord(0, KQuery::DAY3));
    CHECK_EQ(kday3[100], stk.getKRecord(100, KQuery::DAY3));
    CHECK_EQ(kday3[100], stk.getKRecord(Datetime(199203050000), KQuery::DAY3));

    CHECK_EQ(stk.getMarketValue(Datetime(199203050000), KQuery::DAY),
             stk.getKRecord(Datetime(199203050000), KQuery::DAY).closePrice);
    CHECK_EQ(stk.getMarketValue(Datetime(199203050000), KQuery::DAY3), kday3[100].closePrice);
    CHECK_EQ(stk.getMarketValue(Datetime(199203040000), KQuery::DAY3), kday3[99].closePrice);

    size_t startix = 0, endix = 0;
    bool success = stk.getIndexRange(
      KQueryByDate(Datetime(199203050000), Null<Datetime>(), KQuery::DAY3), startix, endix);
    CHECK_UNARY(success);
    CHECK_EQ(startix, 100);
    CHECK_EQ(endix, 1707);
    success = stk.getIndexRange(
      KQueryByDate(Datetime(199203050000), Datetime(199204160000), KQuery::DAY3), startix, endix);
    CHECK_UNARY(success);
    CHECK_EQ(startix, 100);
    CHECK_EQ(endix, 110);

    kday3 = stk.getKData(KQueryByIndex(100, 120, KQuery::DAY3));
    auto kday3_2 =
      stk.getKData(KQueryByDate(Datetime(199203050000), Datetime(199205270001), KQuery::DAY3));
    CHECK_EQ(kday3.size(), kday3_2.size());
    for (size_t i = 0; i < kday3.size(); i++) {
        CHECK_EQ(kday3[i], kday3_2[i]);
    }

    /** @arg 检测周期转换类别的扩展K线数据 */
    auto kmin1 = stk.getKData(KQuery(0, Null<int64_t>(), KQuery::MIN));
    auto kmin3 = stk.getKData(KQuery(0, Null<int64_t>(), KQuery::MIN3));
    CHECK_EQ(kmin3.size(), stk.getCount(KQuery::MIN3));

    REQUIRE(kmin1[0].datetime == Datetime(200001040931));
    CHECK_EQ(kmin3[0].datetime, kmin1[2].datetime);
    CHECK_EQ(kmin3[0].openPrice, kmin1[0].openPrice);
    CHECK_EQ(kmin3[0].closePrice, kmin1[2].closePrice);
    high = kmin1[0].highPrice;
    low = kmin1[0].lowPrice;
    sum_amount = kmin1[0].transAmount;
    sum_volume = kmin1[0].transCount;
    for (size_t i = 1; i < 3; i++) {
        high = std::max(high, kmin1[i].highPrice);
        low = std::min(low, kmin1[i].lowPrice);
        sum_amount += kmin1[i].transAmount;
        sum_volume += kmin1[i].transCount;
    }
    CHECK_EQ(kmin3[0].highPrice, high);
    CHECK_EQ(kmin3[0].lowPrice, low);
    CHECK_EQ(kmin3[0].transAmount, doctest::Approx(sum_amount).epsilon(0.001));
    CHECK_EQ(kmin3[0].transCount, doctest::Approx(sum_volume).epsilon(0.001));

    CHECK_EQ(kmin3[0], stk.getKRecord(0, KQuery::MIN3));
    CHECK_EQ(kmin3[100], stk.getKRecord(100, KQuery::MIN3));
    CHECK_EQ(kmin3[100], stk.getKRecord(Datetime(200001051033), KQuery::MIN3));

    CHECK_EQ(stk.getMarketValue(Datetime(200001040931), KQuery::MIN),
             stk.getKRecord(Datetime(200001040931), KQuery::MIN).closePrice);
    CHECK_EQ(stk.getMarketValue(Datetime(200001051033), KQuery::MIN3), kmin3[100].closePrice);
    CHECK_EQ(stk.getMarketValue(Datetime(200001051032), KQuery::MIN3), kmin3[99].closePrice);

    success = stk.getIndexRange(
      KQueryByDate(Datetime(200001051033), Null<Datetime>(), KQuery::MIN3), startix, endix);
    CHECK_UNARY(success);
    CHECK_EQ(startix, 100);
    CHECK_EQ(endix, 228099);
    success = stk.getIndexRange(
      KQueryByDate(Datetime(200001051033), Datetime(200001051103), KQuery::MIN3), startix, endix);
    CHECK_UNARY(success);
    CHECK_EQ(startix, 100);
    CHECK_EQ(endix, 110);

    kmin3 = stk.getKData(KQueryByIndex(100, 120, KQuery::MIN3));
    auto kmin3_2 =
      stk.getKData(KQueryByDate(Datetime(200001051033), Datetime(200001051131), KQuery::MIN3));
    CHECK_EQ(kmin3.size(), kmin3_2.size());
    for (size_t i = 0; i < kmin3.size(); i++) {
        CHECK_EQ(kmin3[i], kmin3_2[i]);
    }
}

/** @} */
