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
 * @defgroup test_indicator_GROUP_MIN test_indicator_GROUP_MIN
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_GROUP_MIN") {
    HKU_IF_RETURN(!pluginValid(), void());

    auto stk = getStock("sh000001");
    auto mink = stk.getKData(KQueryByDate(Datetime(20111115), Datetime(20111120), KQuery::MIN));

    /** @arg 分钟线按日分组 */
    auto ind = GROUP_MIN(CLOSE(), KQuery::DAY);
    auto result = ind(mink);
    CHECK_EQ(result.name(), "GROUP_MIN");
    CHECK_EQ(result.size(), mink.size());
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], mink[0].closePrice);
    REQUIRE(result[1] < mink[1].closePrice);
    CHECK_EQ(result[1], mink[0].closePrice);
    REQUIRE(result[2] < mink[2].closePrice);
    CHECK_EQ(result[2], mink[0].closePrice);
}

/** @} */
