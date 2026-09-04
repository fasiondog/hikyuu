/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-16
 *      Author: fasiondog
 *
 *  聚合系统 MultiSystem 回归测试。
 *  覆盖范围：
 *    1. MultiSystem 结构：运行模式、clone 保真、组合判定、循环引用检测（直接/嵌套）、
 *       任意深度嵌套、模式 B 额度回写（checkin/checkout）；
 *    2. 聚合分配引擎（MoneyManagerBase L1/L2/L3，供 MultiSystem::allocate 调用，无子类 override）：
 *       - L1 系统级分配：等权 / weight-list 固定权重（迁移 AF_FixedWeight/FixedWeightList）
 *                        / 模式 B 真实额度（含 fixed-amount 迁移 AF_FixedAmount）；
 *       - L2 行为级换算：模式 A 按 assets_ratio 换算（设计 8.3 完整语义透传）/ fixed-amount
 *                        / SELL·CLEAR 全平 / 超配转减仓 / 模式 B 透传裁剪；
 *       - L3 组合风控：max-single-position 集中度裁剪 / 模式 B 跳过。
 *  @note F1（_toSuggestions 三比重填充）为 MultiSystem 私有成员，其产出的 assets_ratio
 *        经 L2 消费路径（test_MoneyManager_L2_modeA_by_ratio）间接验证；端到端集成
 *        依赖运行数据与信号触发，归属 unit-test 数据用例。
 */

#include "../../test_config.h"
#include <unordered_map>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/system/imp/MultiSystem.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_Nothing.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include "create_test_sys.h"

using namespace hku;

/**
 * @defgroup test_MultiSystem test_MultiSystem
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

// ============================================================================
// MultiSystem 结构
// ============================================================================

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

/** @par 检测点：组合判定与子系统列表 */
TEST_CASE("test_MultiSystem_isComposite") {
    MultiSystem ms;
    /** @arg 聚合系统 isComposite 恒为真 */
    CHECK(ms.isComposite());
    /** @arg 初始无子系统 */
    CHECK_EQ(ms.getSubSystemList().size(), 0);
    /** @arg 单证券系统 isComposite 为假 */
    auto sys = create_test_sys(3, 5);
    CHECK(!sys->isComposite());
}

/** @par 检测点：add 的直接循环引用与重复检测 */
TEST_CASE("test_MultiSystem_add") {
    auto ms = std::make_shared<MultiSystem>("ms");
    auto sys1 = create_test_sys(3, 5);
    /** @arg 正常添加子系统 */
    ms->add(sys1);
    CHECK_EQ(ms->getSystemList().size(), 1);
    /** @arg 拒绝空指针 */
    ms->add(SystemPtr());
    CHECK_EQ(ms->getSystemList().size(), 1);
    /** @arg 拒绝重复添加同一实例 */
    ms->add(sys1);
    CHECK_EQ(ms->getSystemList().size(), 1);
    /** @arg 拒绝添加自身（直接循环引用） */
    ms->add(ms);
    CHECK_EQ(ms->getSystemList().size(), 1);
}

/** @par 检测点：嵌套聚合的间接循环引用检测 */
TEST_CASE("test_MultiSystem_add_cycle_nested") {
    auto outer = std::make_shared<MultiSystem>("outer");
    auto inner = std::make_shared<MultiSystem>("inner");
    auto leaf = create_test_sys(3, 5);
    inner->add(leaf);
    /** @arg inner 作为 outer 子系统（正常嵌套） */
    outer->add(inner);
    CHECK_EQ(outer->getSystemList().size(), 1);
    /** @arg 拒绝间接环：outer 已含 inner，inner 不能再含 outer */
    inner->add(outer);
    CHECK_EQ(inner->getSystemList().size(), 1);  // inner 仍只有 leaf
}

/** @par 检测点：任意深度嵌套结构（outer → inner → leaf） */
TEST_CASE("test_MultiSystem_nesting") {
    auto outer = std::make_shared<MultiSystem>("outer");
    auto inner = std::make_shared<MultiSystem>("inner");
    inner->add(create_test_sys(3, 5));
    outer->add(inner);
    /** @arg 外层直接子系统为 inner */
    REQUIRE_EQ(outer->getSubSystemList().size(), 1);
    CHECK(outer->getSubSystemList()[0] == inner);
    /** @arg 直接子系统本身是组合（可递归） */
    CHECK(outer->getSubSystemList()[0]->isComposite());
    /** @arg 内层持有叶子子系统 */
    CHECK_EQ(inner->getSubSystemList().size(), 1);
    CHECK(!inner->getSubSystemList()[0]->isComposite());
}

/** @par 检测点：模式 B 额度回写（checkin/checkout 调整子系统虚拟账户总资产） */
TEST_CASE("test_MultiSystem_setSubSystemQuota") {
    auto ms = std::make_shared<MultiSystem>("ms");
    auto sub = create_test_sys(3, 5);  // 自带 crtTM，初始资金 100000
    ms->add(sub);
    Datetime date(200001010000LL);
    REQUIRE_EQ(sub->getTM()->getFunds(date, KQuery::DAY).total_assets(), doctest::Approx(100000.0));

    /** @arg 额度增加：checkin 差额，总资产升至 quota */
    ms->setSubSystemQuota(sub, date, 150000.0);
    CHECK_EQ(sub->getTM()->getFunds(date, KQuery::DAY).total_assets(), doctest::Approx(150000.0));

    /** @arg 额度减少：checkout 差额，总资产降至 quota */
    ms->setSubSystemQuota(sub, date, 80000.0);
    CHECK_EQ(sub->getTM()->getFunds(date, KQuery::DAY).total_assets(), doctest::Approx(80000.0));

    /** @arg quota<=0 被拒绝（总资产不变） */
    ms->setSubSystemQuota(sub, date, 0.0);
    CHECK_EQ(sub->getTM()->getFunds(date, KQuery::DAY).total_assets(), doctest::Approx(80000.0));

    /** @arg 空子系统被拒绝（不崩溃） */
    ms->setSubSystemQuota(SystemPtr(), date, 100000.0);
    CHECK_EQ(sub->getTM()->getFunds(date, KQuery::DAY).total_assets(), doctest::Approx(80000.0));
}

// ============================================================================
// 聚合分配引擎 L1：系统级分配（名义权重 / 真实额度）
// ============================================================================

/** @par 检测点：L1 默认等权分配（迁移 AF_EqualWeight） */
TEST_CASE("test_MoneyManager_L1_equal_weight") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto mm = MM_Nothing();
    mm->setMode("A");
    Datetime date(200001010000LL);

    /** @arg 空 contexts 返回空权重表（边界：0 个子系统） */
    {
        SubSystemContextList empty;
        auto w = mm->_allocateSystemWeight(date, tm, empty, KQuery());
        CHECK_EQ(w.size(), 0);
    }

    /** @arg 2 个子系统等权 1/N */
    auto sys1 = create_test_sys(3, 5);
    auto sys2 = create_test_sys(5, 10);
    SubSystemContextList contexts(2);
    contexts[0].sys = sys1;
    contexts[1].sys = sys2;
    auto w = mm->_allocateSystemWeight(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(0.5));
    CHECK_EQ(w[sys2], doctest::Approx(0.5));

    /** @arg 模式 A 不写 quota（quota 仅模式 B 产出） */
    CHECK_EQ(contexts[0].quota, doctest::Approx(0.0));
    CHECK_EQ(contexts[1].quota, doctest::Approx(0.0));
}

/** @par 检测点：L1 weight-list 固定权重（迁移 AF_FixedWeight / AF_FixedWeightList） */
TEST_CASE("test_MoneyManager_L1_weight_list") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto mm = MM_Nothing();
    mm->setMode("A");
    Datetime date(200001010000LL);
    auto sys1 = create_test_sys(3, 5);
    auto sys2 = create_test_sys(5, 10);
    SubSystemContextList contexts(2);
    contexts[0].sys = sys1;
    contexts[1].sys = sys2;

    /** @arg 权重和为 1，直接采用 */
    mm->setParam<string>("weight-list", "0.6,0.4");
    auto w = mm->_allocateSystemWeight(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(0.6));
    CHECK_EQ(w[sys2], doctest::Approx(0.4));

    /** @arg 权重和不为 1，自动归一化（3:7 → 0.3/0.7） */
    mm->setParam<string>("weight-list", "3,7");
    w = mm->_allocateSystemWeight(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(0.3));
    CHECK_EQ(w[sys2], doctest::Approx(0.7));

    /** @arg 数量与子系统不符（3 项 vs 2 子系统），回退等权 */
    mm->setParam<string>("weight-list", "0.5,0.3,0.2");
    w = mm->_allocateSystemWeight(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(0.5));
    CHECK_EQ(w[sys2], doctest::Approx(0.5));

    /** @arg 负权重按 0 处理后归一化（0.5,-0.5 → 1.0/0.0） */
    mm->setParam<string>("weight-list", "0.5,-0.5");
    w = mm->_allocateSystemWeight(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(1.0));
    CHECK_EQ(w[sys2], doctest::Approx(0.0));

    /** @arg 全部非法项（解析和为 0），回退等权 */
    mm->setParam<string>("weight-list", "abc,def");
    w = mm->_allocateSystemWeight(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(0.5));
    CHECK_EQ(w[sys2], doctest::Approx(0.5));

    /** @arg 空串回退等权 */
    mm->setParam<string>("weight-list", "");
    w = mm->_allocateSystemWeight(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(0.5));
    CHECK_EQ(w[sys2], doctest::Approx(0.5));
}

/** @par 检测点：L1 模式 B 真实额度（等权 / weight-list / fixed-amount） */
TEST_CASE("test_MoneyManager_L1_modeB_quota") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto mm = MM_Nothing();
    mm->setMode("B");
    Datetime date(200001010000LL);
    auto sys1 = create_test_sys(3, 5);
    auto sys2 = create_test_sys(5, 10);

    /** @arg 单子系统等权：额度 = 权重(1.0) × 父总资产(100000) */
    SubSystemContextList contexts(1);
    contexts[0].sys = sys1;
    mm->_allocateSystemWeight(date, tm, contexts, KQuery());
    CHECK_EQ(contexts[0].quota, doctest::Approx(100000.0));

    /** @arg fixed-amount>0：额度 = 固定金额（迁移 AF_FixedAmount） */
    mm->setParam<double>("fixed-amount", 30000.0);
    mm->_allocateSystemWeight(date, tm, contexts, KQuery());
    CHECK_EQ(contexts[0].quota, doctest::Approx(30000.0));

    /** @arg weight-list 影响额度分配（0.7/0.3 → 70000/30000） */
    mm->setParam<double>("fixed-amount", 0.0);
    mm->setParam<string>("weight-list", "0.7,0.3");
    SubSystemContextList ctx2(2);
    ctx2[0].sys = sys1;
    ctx2[1].sys = sys2;
    mm->_allocateSystemWeight(date, tm, ctx2, KQuery());
    CHECK_EQ(ctx2[0].quota, doctest::Approx(70000.0));
    CHECK_EQ(ctx2[1].quota, doctest::Approx(30000.0));
}

// ============================================================================
// 聚合分配引擎 L2：行为级换算（模式 A 按比重 / 模式 B 透传）
// ============================================================================

/** @par 检测点：L2 模式 A 按 assets_ratio 换算（设计 8.3 完整语义透传，F2） */
TEST_CASE("test_MoneyManager_L2_modeA_by_ratio") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);  // 未建仓，total_assets=100000（纯现金，可预测）
    auto mm = MM_Nothing();
    mm->setMode("A");
    Datetime date(200001010000LL);
    Stock stk = getStock("sz000001");
    REQUIRE(!stk.isNull());
    auto sys = create_test_sys(3, 5);
    std::unordered_map<SYSPtr, double> weights{{sys, 1.0}};

    /** @arg assets_ratio>0：目标市值 = 权重(1.0) × 比重(0.5) × 父总资产(100000) = 50000 → 5000 股 */
    TradeSuggestion s;
    s.stock = stk;
    s.sys = sys;
    s.type = SuggestionType::BUY;
    s.plan_price = 10.0;
    s.number = 100;  // 原始 number 应被改写
    s.assets_ratio = 0.5;
    TradeSuggestionList suggestions{s};
    mm->_allocateSuggestions(date, tm, suggestions, weights, KQuery());
    CHECK(suggestions[0].type == SuggestionType::BUY);
    CHECK_EQ(suggestions[0].number, doctest::Approx(5000.0));

    /** @arg assets_ratio<=0：回退满仓（ratio=1）等权到仓 → 100000/10 = 10000 股 */
    s.assets_ratio = 0.0;
    suggestions[0] = s;
    mm->_allocateSuggestions(date, tm, suggestions, weights, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(10000.0));

    /** @arg plan_price<=0：数量置 0（防除零边界） */
    s.plan_price = 0.0;
    s.assets_ratio = 0.5;
    suggestions[0] = s;
    mm->_allocateSuggestions(date, tm, suggestions, weights, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(0.0));
}

/** @par 检测点：L2 模式 A 超配转减仓（delta<0 → SELL，F2 再平衡分支） */
TEST_CASE("test_MoneyManager_L2_overweight_to_sell") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto mm = MM_Nothing();
    mm->setMode("A");
    // 用 fixed-amount 固定目标市值，规避建仓后 total_assets 受行情影响的不确定性
    mm->setParam<double>("fixed-amount", 5000.0);
    Datetime date(200001010000LL);
    Stock stk = getStock("sz000001");
    REQUIRE(!stk.isNull());
    auto sys = create_test_sys(3, 5);
    std::unordered_map<SYSPtr, double> weights{{sys, 1.0}};
    tm->buy(date, stk, 10.0, 1000.0);  // 建仓 1000 股（持仓数量确定）
    REQUIRE_EQ(tm->getPosition(date, stk).number, doctest::Approx(1000.0));

    /** @arg 目标(5000/10=500 股) < 当前(1000 股)：转 SELL 减仓 500 股 */
    TradeSuggestion s;
    s.stock = stk;
    s.sys = sys;
    s.type = SuggestionType::BUY;
    s.plan_price = 10.0;
    TradeSuggestionList suggestions{s};
    mm->_allocateSuggestions(date, tm, suggestions, weights, KQuery());
    CHECK(suggestions[0].type == SuggestionType::SELL);
    CHECK_EQ(suggestions[0].number, doctest::Approx(500.0));
}

/** @par 检测点：L2 模式 A SELL/CLEAR 全平当前持仓 */
TEST_CASE("test_MoneyManager_L2_sell_clear") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto mm = MM_Nothing();
    mm->setMode("A");
    Datetime date(200001010000LL);
    Stock stk = getStock("sz000001");
    REQUIRE(!stk.isNull());
    auto sys = create_test_sys(3, 5);
    std::unordered_map<SYSPtr, double> weights{{sys, 1.0}};
    tm->buy(date, stk, 10.0, 1000.0);  // 建仓 1000 股

    /** @arg SELL：number = -current（全平），原始 number 被改写 */
    TradeSuggestion s;
    s.stock = stk;
    s.sys = sys;
    s.type = SuggestionType::SELL;
    s.plan_price = 10.0;
    s.number = 999;
    TradeSuggestionList suggestions{s};
    mm->_allocateSuggestions(date, tm, suggestions, weights, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(-1000.0));

    /** @arg CLEAR：同样全平 */
    s.type = SuggestionType::CLEAR;
    suggestions[0] = s;
    mm->_allocateSuggestions(date, tm, suggestions, weights, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(-1000.0));

    /** @arg 空持仓 SELL：number = -0 = 0（边界） */
    auto tm2 = crtTM(Datetime(200001010000LL), 100000.0);
    s.type = SuggestionType::SELL;
    s.number = 500;
    suggestions[0] = s;
    mm->_allocateSuggestions(date, tm2, suggestions, weights, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(0.0));
}

/** @par 检测点：L2 fixed-amount 每标的固定金额（迁移 AF_FixedAmount，F4） */
TEST_CASE("test_MoneyManager_L2_fixed_amount") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto mm = MM_Nothing();
    mm->setMode("A");
    mm->setParam<double>("fixed-amount", 20000.0);
    Datetime date(200001010000LL);
    Stock stk = getStock("sz000001");
    REQUIRE(!stk.isNull());
    auto sys = create_test_sys(3, 5);
    std::unordered_map<SYSPtr, double> weights{{sys, 1.0}};

    /** @arg fixed-amount 优先于 assets_ratio：目标市值 = 20000 → 2000 股 */
    TradeSuggestion s;
    s.stock = stk;
    s.sys = sys;
    s.type = SuggestionType::BUY;
    s.plan_price = 10.0;
    s.assets_ratio = 0.9;  // 应被 fixed-amount 覆盖
    TradeSuggestionList suggestions{s};
    mm->_allocateSuggestions(date, tm, suggestions, weights, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(2000.0));
}

/** @par 检测点：L2 模式 B 透传子系统指令（父不换算，仅防御性裁剪 SELL） */
TEST_CASE("test_MoneyManager_L2_modeB_passthrough") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto mm = MM_Nothing();
    mm->setMode("B");
    Datetime date(200001010000LL);
    Stock stk = getStock("sz000001");
    REQUIRE(!stk.isNull());
    auto sys = create_test_sys(3, 5);
    std::unordered_map<SYSPtr, double> weights{{sys, 1.0}};

    /** @arg BUY 透传原始 number（不按父资产换算） */
    TradeSuggestion s;
    s.stock = stk;
    s.sys = sys;
    s.type = SuggestionType::BUY;
    s.plan_price = 10.0;
    s.number = 888;
    TradeSuggestionList suggestions{s};
    mm->_allocateSuggestions(date, tm, suggestions, weights, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(888.0));

    /** @arg SELL 裁剪不超父当前持仓（current=0 → 裁剪为 0） */
    s.type = SuggestionType::SELL;
    s.number = 1000;
    suggestions[0] = s;
    mm->_allocateSuggestions(date, tm, suggestions, weights, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(0.0));
}

// ============================================================================
// 聚合分配引擎 L3：组合风控裁剪
// ============================================================================

/** @par 检测点：L3 集中度风控（max-single-position 裁剪 / 不限制 / 模式 B 跳过） */
TEST_CASE("test_MoneyManager_L3_check_risk") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);  // 未建仓，total_assets=100000
    auto mm = MM_Nothing();
    mm->setMode("A");
    Datetime date(200001010000LL);
    Stock stk = getStock("sz000001");
    REQUIRE(!stk.isNull());
    auto sys = create_test_sys(3, 5);
    TradeSuggestion s;
    s.stock = stk;
    s.sys = sys;
    s.type = SuggestionType::BUY;
    s.plan_price = 10.0;
    s.number = 20000;  // 目标市值 200000

    /** @arg max-single-position=0.5：裁剪至 50000 市值 = 5000 股 */
    mm->setParam<double>("max-single-position", 0.5);
    TradeSuggestionList suggestions{s};
    mm->_checkRisk(date, tm, suggestions, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(5000.0));

    /** @arg max-single-position>=1.0：不限制（数量不变） */
    mm->setParam<double>("max-single-position", 1.0);
    suggestions[0] = s;
    mm->_checkRisk(date, tm, suggestions, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(20000.0));

    /** @arg 模式 B：跳过风控（尊重子管理人自主权，数量不变） */
    mm->setMode("B");
    mm->setParam<double>("max-single-position", 0.5);
    suggestions[0] = s;
    mm->_checkRisk(date, tm, suggestions, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(20000.0));
}

/** @} */
