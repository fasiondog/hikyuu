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

static bool pluginValid() {
    auto& sm = StockManager::instance();
    auto* plugin = sm.getPlugin<ExtendIndicatorsPluginInterface>(HKU_PLUGIN_EXTEND_INDICATOR);
    return plugin && isValidLicense();
}

/**
 * @defgroup test_indicator_AGG_COUNT test_indicator_AGG_COUNT
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_AGG_COUNT") {
    HKU_IF_RETURN(!pluginValid(), void());

    auto k = getKData("sh000001", KQueryByDate(Datetime(20111115)));
    auto mink =
      getKData("sh000001", KQueryByDate(Datetime(20111115), Null<Datetime>(), KQuery::MIN));

    auto ind = AGG_COUNT(CLOSE(), KQuery::MIN)(k);
    CHECK_EQ(ind.name(), "AGG_COUNT");
    CHECK_EQ(ind.size(), k.size());
    CHECK_EQ(ind.discard(), 0);
    double sum = 0.0;
    for (const auto& v : ind) {
        sum += v;
    }
    CHECK_EQ(sum, mink.size());
}

/** @} */
