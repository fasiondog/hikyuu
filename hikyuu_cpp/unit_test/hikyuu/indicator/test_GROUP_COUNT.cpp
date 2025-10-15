/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-15
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
 * @defgroup test_indicator_GROUP_COUNT test_indicator_GROUP_COUNT
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_GROUP_COUNT") {
    HKU_IF_RETURN(!pluginValid(), void());

    auto stk = getStock("sh000001");
    auto k = stk.getKData(KQueryByDate(Datetime(20111115)));
    auto mink = stk.getKData(KQueryByDate(Datetime(20111115), Datetime(20111120), KQuery::MIN));

    /** @arg 分钟线按日分组 */
    auto ind = GROUP_COUNT(CLOSE(), KQuery::DAY);
    auto result = ind(mink);
    CHECK_EQ(result.name(), "GROUP_COUNT");
    CHECK_EQ(result.size(), mink.size());
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(
      result.back(),
      stk.getKData(KQueryByDate(Datetime(20111118), Datetime(20111119), KQuery::MIN)).size());

    /** @arg 日线按日分组 */
    result = ind(k);
    CHECK_EQ(result.name(), "GROUP_COUNT");
    CHECK_EQ(result.size(), k.size());
    CHECK_EQ(result.discard(), 0);
    for (size_t i = 0; i < result.size(); ++i) {
        CHECK_EQ(result[i], 1);
    }

    /** @arg 日线按周分组 */
    auto wk = stk.getKData(KQueryByDate(Datetime(20111115), Null<Datetime>(), KQuery::WEEK));
    result = ind(wk);
    CHECK_EQ(result.name(), "GROUP_COUNT");
    CHECK_EQ(result.size(), wk.size());
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 1);
    CHECK_EQ(result[1], 1);
    CHECK_EQ(result[2], 1);
}

/** @} */
