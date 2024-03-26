/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-27
 *      Author: fasiondog
 */

#include "../../test_config.h"
#include "hikyuu/trade_sys/allocatefunds/crt/AF_FixedWeight.h"

using namespace hku;

/**
 * @defgroup test_AF_FixedWeight test_AF_FixedWeight
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_AF_FixedWeight") {
    /** @arg 非法参数 */
    CHECK_THROWS_AS(AF_FixedWeight(0.0), std::exception);
    CHECK_THROWS_AS(AF_FixedWeight(-0.1), std::exception);
    CHECK_THROWS_AS(AF_FixedWeight(1.001), std::exception);

    /** @arg 尝试变更非法的公共参数 */
    auto af = AF_FixedWeight(0.1);
    CHECK_THROWS_AS(af->setParam<bool>("auto_adjust_weight", true), std::exception);
}

/** @} */