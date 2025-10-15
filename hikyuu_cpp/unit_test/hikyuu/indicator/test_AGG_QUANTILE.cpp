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
 * @defgroup test_indicator_AGG_QUANTILE test_indicator_AGG_QUANTILE
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_AGG_QUANTILE") {
    HKU_IF_RETURN(!pluginValid(), void());

    auto k = getKData("sh000001", KQueryByDate(Datetime(20111115)));
    auto mink =
      getKData("sh000001", KQueryByDate(Datetime(20111115), Null<Datetime>(), KQuery::MIN));

    /** @arg 单日分钟线聚合 */
    auto ind = AGG_QUANTILE(CLOSE(), KQuery::MIN, false, 1, 0.5);
    auto result = ind(k);
    CHECK_EQ(result.size(), k.size());
    CHECK_EQ(result.name(), "AGG_QUANTILE");
    CHECK_EQ(result.discard(), 0);

    auto mink2 =
      getKData("sh000001", KQueryByDate(Datetime(20111115), Datetime(20111116), KQuery::MIN));
    std::vector<price_t> prices;
    for (auto& kr : mink2) {
        prices.push_back(kr.closePrice);
    }
    std::sort(prices.begin(), prices.end());
    CHECK_EQ(result[0], doctest::Approx((prices[119] + prices[120]) / 2.));
}

/** @} */
