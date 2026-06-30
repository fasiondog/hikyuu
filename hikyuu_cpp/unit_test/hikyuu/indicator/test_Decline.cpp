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
 * DECLINE 与 ADVANCE 互为镜像, 锁定方向语义: DECLINE 统计下跌, ADVANCE 统计上涨。
 * 验证: 在有足够多股票数据的交易日 (D+A >= 10, 排除极少数据对齐的异质日),
 *   存在某日 D != A (非全平盘, < / > 运算符实际区分下跌/上涨)。
 * 注: 修复前 _increment_calculate 用 '>' 会导致 DECLINE 增量结果等于 ADVANCE;
 *   修复后全量与增量均用 '<', DECLINE 与 ADVANCE 在非横盘日必然不同。
 *   部分时点因 ALIGN 日期对齐一端为 NaN (停牌/未上市填充), 仅在双值点验证。
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

    // 在有足够数据的双值点 (D+A>=10) 验证方向区分
    bool found_distinct = false;
    for (size_t i = 0; i < decline.size(); ++i) {
        if (std::isnan(decline[i]) || std::isnan(advance[i])) {
            continue;
        }
        price_t sum = decline[i] + advance[i];
        if (sum >= 10.0) {
            CHECK_GT(sum, 0.0);
            if (decline[i] != advance[i]) {
                found_distinct = true;
            }
        }
    }
    // 真实行情 20 日内必存在非全平盘日 (D != A)
    CHECK_UNARY(found_distinct);
}

/**
 * @par 检测点
 * ignore_context 模式: 不依赖上下文, 直接按 query/market/stk_type 全市场统计。
 * 验证可正常计算且存在有效 (非 NaN) 输出点。
 */
TEST_CASE("test_Decline_ignore_context") {
    Indicator decline = DECLINE(KQuery(-10), "SH", STOCKTYPE_A, true, true);
    CHECK_EQ(decline.name(), "DECLINE");
    CHECK_EQ(decline.empty(), false);
    CHECK_GT(decline.size(), 0);
    // 遍历找首个非 NaN 有效点断言 (discard 点可能因对齐为 NaN)
    bool has_valid = false;
    for (size_t i = 0; i < decline.size(); ++i) {
        if (!std::isnan(decline[i])) {
            CHECK_GE(decline[i], 0.0);
            has_valid = true;
            break;
        }
    }
    CHECK_UNARY(has_valid);
}

/** @} */