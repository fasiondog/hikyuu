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
#include "plugin_valid.h"

using namespace hku;

/**
 * @defgroup test_indicator_GROUP_PROD test_indicator_GROUP_PROD
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_GROUP_PROD") {
    HKU_IF_RETURN(!pluginValid(), void());

    auto stk = getStock("sh000001");
    auto mink = stk.getKData(KQueryByDate(Datetime(20111115), Datetime(20111120), KQuery::MIN));

    /** @arg 分钟线按日分组 */
    auto ind = GROUP_PROD(CLOSE(), KQuery::DAY);
    auto result = ind(mink);
    CHECK_EQ(result.name(), "GROUP_PROD");
    CHECK_EQ(result.size(), mink.size());
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], doctest::Approx(mink[0].closePrice));
    CHECK_EQ(result[1], doctest::Approx(mink[0].closePrice * mink[1].closePrice));
    CHECK_EQ(result[720], doctest::Approx(mink[720].closePrice));
    CHECK_EQ(result[721], doctest::Approx(mink[720].closePrice * mink[721].closePrice));
}

/** @} */
