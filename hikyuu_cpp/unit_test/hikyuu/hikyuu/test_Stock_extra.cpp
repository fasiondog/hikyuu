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

using namespace hku;

static bool pluginValid() {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<HkuExtraPluginInterface>(HKU_PLUGIN_HKU_EXTRA);
    return plugin != nullptr;
}

/**
 * @defgroup test_hikyuu_Stock_extra test_hikyuu_Stock_extra
 * @ingroup test_hikyuu_base_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_Stock_extra_ktype") {
    HKU_IF_RETURN(!pluginValid(), void());

    /** @arg 检查扩展K线类型对应的分钟数 */
    CHECK_EQ(KQuery::getKTypeInMin(KQuery::DAY) * 3, KQuery::getKTypeInMin(KQuery::DAY3));

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
    // HKU_INFO("{}", kday3[100]);
    CHECK_EQ(kday3[100], stk.getKRecord(Datetime(199203050000), KQuery::DAY3));
}

/** @} */
