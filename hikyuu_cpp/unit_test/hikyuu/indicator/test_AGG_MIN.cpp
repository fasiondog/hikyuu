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
#include "../plugin_valid.h"

using namespace hku;

/**
 * @defgroup test_indicator_AGG_MIN test_indicator_AGG_MIN
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_AGG_MIN") {
    HKU_IF_RETURN(!pluginValid(), void());

    auto k = getKData("sh000001", KQueryByDate(Datetime(20111115)));
    auto mink =
      getKData("sh000001", KQueryByDate(Datetime(20111115), Null<Datetime>(), KQuery::MIN));

    /** @arg 单日分钟线聚合 */
    auto ind = AGG_MIN(HIGH(), KQuery::MIN);
    auto result = ind(k);
    CHECK_EQ(result.size(), k.size());
    CHECK_EQ(result.name(), "AGG_MIN");
    CHECK_EQ(result.discard(), 0);

    auto mink2 =
      getKData("sh000001", KQueryByDate(Datetime(20111115), Datetime(20111116), KQuery::MIN));
    price_t min_value = std::numeric_limits<price_t>::max();
    for (auto& kr : mink2) {
        if (kr.highPrice < min_value) {
            min_value = kr.highPrice;
        }
    }
    CHECK_EQ(result[0], doctest::Approx(min_value));

    mink2 = getKData("sh000001", KQueryByDate(Datetime(20111206), Datetime(20111207), KQuery::MIN));
    min_value = std::numeric_limits<price_t>::max();
    for (auto& kr : mink2) {
        if (kr.highPrice < min_value) {
            min_value = kr.highPrice;
        }
    }
    CHECK_EQ(result.back(), doctest::Approx(min_value));

    mink2 = getKData("sh000001", KQueryByDate(Datetime(20111118), Datetime(20111119), KQuery::MIN));
    min_value = std::numeric_limits<price_t>::max();
    for (auto& kr : mink2) {
        if (kr.highPrice < min_value) {
            min_value = kr.highPrice;
        }
    }
    CHECK_EQ(result[3], doctest::Approx(min_value));
}

/** @} */
