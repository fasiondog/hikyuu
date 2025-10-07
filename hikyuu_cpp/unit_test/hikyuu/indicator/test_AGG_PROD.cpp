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

using namespace hku;

/**
 * @defgroup test_indicator_AGG_PROD test_indicator_AGG_PROD
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

// 低精度模式下, float 会溢出
#if !HKU_USE_LOW_PRECISION

static bool pluginValid() {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<ExtendIndicatorsPluginInterface>(HKU_PLUGIN_EXTEND_INDICATOR);
    return plugin && isValidLicense();
}

/** @par 检测点 */
TEST_CASE("test_AGG_PROD") {
    HKU_IF_RETURN(!pluginValid(), void());

    auto k = getKData("sz000001", KQueryByDate(Datetime(20111115)));
    auto mink =
      getKData("sz000001", KQueryByDate(Datetime(20111115), Null<Datetime>(), KQuery::MIN));

    /** @arg 单日分钟线聚合 */
    auto ind = AGG_PROD(CLOSE(), KQuery::MIN);
    auto result = ind(k);
    CHECK_EQ(result.size(), k.size());
    CHECK_EQ(result.name(), "AGG_PROD");
    CHECK_EQ(result.discard(), 0);

    auto mink2 =
      getKData("sz000001", KQueryByDate(Datetime(20111115), Datetime(20111116), KQuery::MIN));
    double sum = 1.0;
    for (auto& kr : mink2) {
        sum *= kr.closePrice;
    }
    CHECK_EQ(result[0], doctest::Approx(sum));

    mink2 = getKData("sz000001", KQueryByDate(Datetime(20111205), Datetime(20111206), KQuery::MIN));
    sum = 1.0;
    for (auto& kr : mink2) {
        sum *= kr.closePrice;
    }
    CHECK_EQ(result.back(), doctest::Approx(sum));

    mink2 = getKData("sz000001", KQueryByDate(Datetime(20111118), Datetime(20111119), KQuery::MIN));
    sum = 1.0;
    for (auto& kr : mink2) {
        sum *= kr.closePrice;
    }
    CHECK_EQ(result[3], doctest::Approx(sum));
}
#endif

/** @} */
