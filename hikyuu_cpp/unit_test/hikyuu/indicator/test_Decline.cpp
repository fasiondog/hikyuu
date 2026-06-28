/*
 * test_Decline.cpp
 *
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created for: 回归 IDDecline::_increment_calculate 运算符反向 bug
 *      Author: woleigegg
 */

#include "../test_config.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/DECLINE.h>
#include <hikyuu/indicator/crt/ADVANCE.h>
#include <hikyuu/indicator/crt/KDATA.h>

using namespace hku;

/**
 * @defgroup test_indicator_Decline test_indicator_Decline
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/**
 * @par 检测点
 * 核心回归: 全量计算 == 增量计算 (复用同一实例连续 setContext 触发 _increment_calculate)。
 *
 * 背景: IDDecline::_increment_calculate 曾因比较运算符从 '<' 误写为 '>' (从 ADVANCE
 * 复制未改符号), 导致增量路径实际统计的是上涨家数。该 bug 仅在 _increment_calculate
 * 暴露, _calculate 全量路径用 '<' 是正确的。
 *
 * 触发机制: 必须复用同一个 Indicator 实例连续调用 setContext —— 第一次 setContext
 * (小窗口) 产生并缓存 m_old_context, 第二次 setContext (大窗口, 尾部延伸) 使
 * can_increment_calculate() 成立, 强制进入 _increment_calculate。若用 operator()/
 * clone() 会丢弃 m_old_context 导致静默回退全量, 测试假性通过。
 *
 * 捕捉逻辑: full 全程用 '<' (正确下跌家数); inc 的 [start_pos, total) 走增量, 修复前
 * 用 '>' (错误, 算的是上涨家数), 与 full 不相等 → CHECK_EQ 失败暴露 bug; 修复后用 '<'
 * 与 full 一致 → 通过。
 */
TEST_CASE("test_Decline_increment_equivalence") {
    StockManager& sm = StockManager::instance();
    // 用个股上下文 (非 INDEX) 满足 supportIncrementCalculate
    Stock stk = sm.getStock("SH600000");
    // KQuery(-20,-10) 与 KQuery(-20) 首根物理对齐, 后者尾部延伸, 满足增量准入
    KData k_full = stk.getKData(KQuery(-20));
    KData k_partial = stk.getKData(KQuery(-20, -10));

    // (1) 全量基准: 独立实例
    Indicator ind_full = DECLINE();
    ind_full.setContext(k_full);

    // (2) 增量测试: 复用同一实例连续 setContext 触发内部状态机
    Indicator ind_inc = DECLINE();
    ind_inc.setContext(k_partial);  // 缓存 m_old_context
    ind_inc.setContext(k_full);     // 尾部延伸 -> 进入 _increment_calculate

    CHECK_EQ(ind_full.size(), ind_inc.size());
    CHECK_EQ(ind_full.discard(), ind_inc.discard());

    for (size_t i = 0; i < ind_full.size(); ++i) {
        if (std::isnan(ind_full[i]) && std::isnan(ind_inc[i])) {
            continue;
        }
        CHECK_EQ(ind_inc[i], doctest::Approx(ind_full[i]).epsilon(0.0001));
    }

    // (3) 方向性硬断言 (双保险): 非横盘行情下 DECLINE ≠ ADVANCE
    // 修复前若 bug 存在, ind_inc 增量结果会等于 ADVANCE; 修复后必不相等
    Indicator ind_adv = ADVANCE();
    ind_adv.setContext(k_full);
    size_t last_idx = ind_full.size() - 1;
    if (!std::isnan(ind_full[last_idx]) && !std::isnan(ind_adv[last_idx]) &&
        ind_full[last_idx] != ind_adv[last_idx]) {
        CHECK_NE(ind_inc[last_idx], ind_adv[last_idx]);
    }
}

/**
 * @par 检测点
 * DECLINE 与 ADVANCE 互为镜像: 同一上下文下两者逐点之和应为常量 (下跌家数 + 上涨家数
 * = 有效家数)。锁定方向语义: DECLINE 统计下跌, ADVANCE 统计上涨, 不可互换。
 */
TEST_CASE("test_Decline_vs_Advance_mirror") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm.getStock("SH600000");
    KData k = stk.getKData(KQuery(-20));

    Indicator decline = DECLINE();
    decline.setContext(k);
    Indicator advance = ADVANCE();
    advance.setContext(k);

    CHECK_EQ(decline.name(), "DECLINE");
    CHECK_EQ(advance.name(), "ADVANCE");
    CHECK_EQ(decline.size(), advance.size());

    size_t discard = std::max(decline.discard(), advance.discard());
    if (discard < decline.size()) {
        price_t total = decline[discard] + advance[discard];
        CHECK_GT(total, 0.0);
        for (size_t i = discard + 1; i < decline.size(); ++i) {
            if (!std::isnan(decline[i]) && !std::isnan(advance[i])) {
                CHECK_EQ(decline[i] + advance[i], doctest::Approx(total).epsilon(0.0001));
            }
        }
    }
}

/**
 * @par 检测点
 * ignore_context 模式: 不依赖上下文, 直接按 query/market/stk_type 全市场统计。
 */
TEST_CASE("test_Decline_ignore_context") {
    Indicator decline = DECLINE(KQuery(-10), "SH", STOCKTYPE_A, true, true);
    CHECK_EQ(decline.name(), "DECLINE");
    CHECK_EQ(decline.empty(), false);
    CHECK_GT(decline.size(), 0);
    size_t d = decline.discard();
    CHECK_LT(d, decline.size());
    CHECK_GE(decline[d], 0.0);
}

/** @} */