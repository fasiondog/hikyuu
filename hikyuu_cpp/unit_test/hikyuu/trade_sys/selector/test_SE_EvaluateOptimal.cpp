/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2025-10-10
 *      Author: fasiondog
 */

#include "../../test_config.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include <hikyuu/trade_sys/selector/crt/SE_Optimal.h>
#include <hikyuu/trade_sys/selector/imp/optimal/OptimalSelectorBase.h>
#include "../system/create_test_sys.h"

using namespace hku;

/**
 * @defgroup test_Selector_optimal test_Selector_optimal
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_SE_EvaluateOptimal") {
    auto se = std::dynamic_pointer_cast<OptimalSelectorBase>(
      SE_EvaluateOptimal([](const SYSPtr& sys, const Datetime& date) { return 42.0; }));
    CHECK_EQ(se->name(), "SE_EvaluateOptimal");

    auto sys = create_test_sys(2, 3);

    /** @arg evaluate returns the value of the evaluate function */
    CHECK_EQ(se->evaluate(sys, Datetime()), doctest::Approx(42.0));

    /** @arg the clone keeps the evaluate function, otherwise evaluate returns Null */
    auto cloned = std::dynamic_pointer_cast<OptimalSelectorBase>(se->clone());
    CHECK_UNARY(cloned);
    CHECK_EQ(cloned->name(), "SE_EvaluateOptimal");
    CHECK_EQ(cloned->evaluate(sys, Datetime()), doctest::Approx(42.0));
}

/** @} */
