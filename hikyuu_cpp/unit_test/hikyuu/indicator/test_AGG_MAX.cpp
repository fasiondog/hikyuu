/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-09-04
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/plugin/interface/plugins.h>
#include <hikyuu/plugin/extind.h>
#include <hikyuu/plugin/device.h>
#include "plugin_valid.h"

using namespace hku;

/**
 * @defgroup test_indicator_AGG_MAX test_indicator_AGG_MAX
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_AGG_MAX") {
    HKU_IF_RETURN(!pluginValid(), void());

    auto k = getKData("sh000001", KQueryByDate(Datetime(20111115)));
    auto mink =
      getKData("sh000001", KQueryByDate(Datetime(20111115), Null<Datetime>(), KQuery::MIN));

    /** @arg 单日分钟线聚合 */
    auto ind = AGG_MAX(HIGH(), KQuery::MIN);
    auto result = ind(k);
    CHECK_EQ(result.size(), k.size());
    CHECK_EQ(result.name(), "AGG_MAX");
    CHECK_EQ(result.discard(), 0);

    auto mink2 =
      getKData("sh000001", KQueryByDate(Datetime(20111115), Datetime(20111116), KQuery::MIN));
    price_t max_value = std::numeric_limits<price_t>::min();
    for (auto& kr : mink2) {
        if (kr.highPrice > max_value) {
            max_value = kr.highPrice;
        }
    }
    CHECK_EQ(result[0], doctest::Approx(max_value));

    mink2 = getKData("sh000001", KQueryByDate(Datetime(20111206), Datetime(20111207), KQuery::MIN));
    max_value = std::numeric_limits<price_t>::min();
    for (auto& kr : mink2) {
        if (kr.highPrice > max_value) {
            max_value = kr.highPrice;
        }
    }
    CHECK_EQ(result.back(), doctest::Approx(max_value));

    mink2 = getKData("sh000001", KQueryByDate(Datetime(20111118), Datetime(20111119), KQuery::MIN));
    max_value = std::numeric_limits<price_t>::min();
    for (auto& kr : mink2) {
        if (kr.highPrice > max_value) {
            max_value = kr.highPrice;
        }
    }
    CHECK_EQ(result[3], doctest::Approx(max_value));
}

/** @} */
