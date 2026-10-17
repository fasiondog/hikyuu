/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2026-10-18
 *      Author: fasiondog
 */

#include "../test_config.h"

#include <hikyuu/StockManager.h>
#include <hikyuu/analysis/analysis_sys.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_Nothing.h>
#include <hikyuu/trade_sys/signal/crt/SG_Bool.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>

using namespace hku;

/**
 * @defgroup test_hikyuu_analysis_sys test_hikyuu_analysis_sys
 * @ingroup test_hikyuu_analysis_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_findOptimalSystem_invalid_sort_key") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query = KQueryByDate(Datetime(199911100000LL), Datetime(200002250000LL));

    Indicator buy = CLOSE() > OPEN();
    Indicator sell = CLOSE() < 0;
    SystemList sys_list;
    for (size_t i = 0; i < 2; i++) {
        sys_list.emplace_back(
          SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Bool(buy, sell)));
    }

    /** @arg an empty sort_key is rejected instead of silently selecting no system */
    CHECK_THROWS_AS((findOptimalSystem(sys_list, stk, query, "")), hku::exception);

    /** @arg an unknown sort_key is rejected instead of silently selecting no system */
    CHECK_THROWS_AS((findOptimalSystem(sys_list, stk, query, "Not Exist Key")), hku::exception);

    /** @arg a valid sort_key selects a system and returns its statistics value */
    auto result = findOptimalSystem(sys_list, stk, query, "Current Total Assets", 0);
    REQUIRE(result.second);
    CHECK_GT(result.first, 0.0);
}

/** @par Test points */
TEST_CASE("test_findOptimalSystemMulti_invalid_sort_key") {
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query = KQueryByDate(Datetime(199911100000LL), Datetime(200002250000LL));

    Indicator buy = CLOSE() > OPEN();
    Indicator sell = CLOSE() < 0;
    SystemList sys_list;
    for (size_t i = 0; i < 2; i++) {
        sys_list.emplace_back(
          SYS_Simple(crtTM(), MM_Nothing(), EnvironmentPtr(), ConditionPtr(), SG_Bool(buy, sell)));
    }

    /** @arg an empty sort_key is rejected instead of silently selecting no system */
    CHECK_THROWS_AS((findOptimalSystemMulti(sys_list, stk, query, "")), hku::exception);

    /** @arg an unknown sort_key is rejected instead of silently selecting no system */
    CHECK_THROWS_AS((findOptimalSystemMulti(sys_list, stk, query, "Not Exist Key")),
                    hku::exception);

    /** @arg a valid sort_key selects a system and returns its statistics value */
    auto result = findOptimalSystemMulti(sys_list, stk, query, "Current Total Assets", 0);
    REQUIRE(result.second);
    CHECK_GT(result.first, 0.0);
}

/** @} */
