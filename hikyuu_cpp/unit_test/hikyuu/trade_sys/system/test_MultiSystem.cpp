/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-16
 *      Author: fasiondog
 *
 *  聚合系统 MultiSystem 回归测试。
 *  重点守护递归组合重构中 _clone 丢失运行模式（m_mode）等成员的缺陷：
 *  clone 后须保留 A/B 模式、调仓周期、sell_at_not_selected 与子系统（深拷贝）。
 */

#include "../../test_config.h"
#include <hikyuu/trade_sys/system/imp/MultiSystem.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include "create_test_sys.h"

using namespace hku;

/**
 * @defgroup test_MultiSystem test_MultiSystem
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点：运行模式语义（默认 A；B/b 归一化为 B；非法值回退 A） */
TEST_CASE("test_MultiSystem_mode") {
    MultiSystem ms;
    CHECK_EQ(ms.getMode(), "A");

    ms.setMode("B");
    CHECK_EQ(ms.getMode(), "B");

    ms.setMode("b");  // 小写亦识别为 B
    CHECK_EQ(ms.getMode(), "B");

    ms.setMode("X");  // 非法值归一化为默认 A
    CHECK_EQ(ms.getMode(), "A");
}

/** @par 检测点：clone 保留运行模式与聚合参数（守护 _clone 丢失 m_mode/m_sell_at_not_selected 的缺陷） */
TEST_CASE("test_MultiSystem_clone_preserve_mode") {
    auto ms = std::make_shared<MultiSystem>("outer");
    ms->add(create_test_sys(3, 5));
    ms->setMode("B");
    ms->setAdjustCycle(5);
    ms->setSellAtNotSelected(false);
    ms->setSubInitCash(50000.0);
    REQUIRE_EQ(ms->getMode(), "B");

    auto cloned = std::dynamic_pointer_cast<MultiSystem>(ms->clone());
    REQUIRE(cloned != nullptr);

    CHECK_EQ(cloned->name(), "outer");
    CHECK_EQ(cloned->getMode(), "B");                 // clone 保留运行模式（缺陷修复点）
    CHECK_EQ(cloned->getAdjustCycle(), 5);            // clone 保留调仓周期
    CHECK_EQ(cloned->getSellAtNotSelected(), false);  // clone 保留 sell_at_not_selected（缺陷修复点）
    CHECK_EQ(cloned->getSubInitCash(), doctest::Approx(50000.0));
    REQUIRE_EQ(cloned->getSystemList().size(), 1);    // 子系统被克隆
    CHECK(cloned->getSystemList()[0] != ms->getSystemList()[0]);  // 深拷贝：与源不同实例
}

/** @} */
