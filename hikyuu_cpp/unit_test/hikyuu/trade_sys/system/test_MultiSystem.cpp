/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-16
 *      Author: fasiondog
 *
 *  聚合系统 MultiSystem 回归测试。
 *  覆盖范围：
 *    1. MultiSystem 结构：运行模式、clone 保真、组合判定、循环引用检测（直接/嵌套）、
 *       任意深度嵌套、模式 B 额度回写（checkin/checkout）、驱动轴模式与固定轴注入、
 *       run(query) 兼容重载（master Portfolio::run(query) 语义）、
 *       外部调仓日表注入（归一化去重 / 覆盖式写入 / 清空）；
 *    2. 组合级资金分配引擎 AllocateFundsBase（AF L1/L2/L3）：
 *       - L1 系统级分配：等权 / weight-list 固定权重（迁移 AF_FixedWeight/FixedWeightList）
 *                        / 模式 B 真实额度（含 fixed-amount 迁移 AF_FixedAmount）；
 *       - L2 行为级换算：模式 A 按 assets_ratio 换算（设计 8.3 完整语义透传）/ fixed-amount
 *                        / SELL·CLEAR 全平 / 超配转减仓 / 模式 B 透传裁剪；
 *       - L3 组合风控：max-single-position 集中度裁剪 / 模式 B 跳过。
 *  @note MM 已限定为单系统形态，组合级分配（L1/L2/L3）整体迁移至 AF（见
 *        docs/design/pf_af_compat/design.md §5）。
 */

#include "../../test_config.h"
#include <unordered_map>
#include <type_traits>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/system/imp/MultiSystem.h>
#include <hikyuu/trade_sys/portfolio/build_in.h>
#include <hikyuu/trade_sys/allocatefunds/build_in.h>
#include <hikyuu/trade_sys/selector/crt/SE_Fixed.h>
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

/** @par 检测点：clone 保留运行模式与聚合参数（守护 _clone 丢失运行模式/m_sell_at_not_selected 的缺陷） */
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

/** @par 检测点：驱动时间轴模式（默认 kdata；calendar 生效；非法值回退）与固定轴注入/清空 */
TEST_CASE("test_MultiSystem_axis_mode") {
    MultiSystem ms("ms");
    /** @arg 默认驱动轴为 kdata（入参 KData 自带日期，保持既有行为） */
    CHECK_EQ(ms.getAxisMode(), "kdata");
    CHECK_EQ(ms.getParam<string>("axis-mode"), "kdata");

    /** @arg 可切换为 calendar（以注入的固定日期表为驱动轴） */
    ms.setAxisMode("calendar");
    CHECK_EQ(ms.getAxisMode(), "calendar");

    /** @arg 非法取值回退 kdata */
    ms.setAxisMode("month");
    CHECK_EQ(ms.getAxisMode(), "kdata");

    /** @arg 固定日期表注入 / 读取 / 清空 */
    CHECK_EQ(ms.getDateAxis().size(), 0);
    ms.setDateAxis({Datetime(20250102), Datetime(20250103)});
    REQUIRE_EQ(ms.getDateAxis().size(), 2);
    CHECK_EQ(ms.getDateAxis()[0], Datetime(20250102));
    CHECK_EQ(ms.getDateAxis()[1], Datetime(20250103));
    ms.clearDateAxis();
    CHECK_EQ(ms.getDateAxis().size(), 0);
}

/** @par 检测点：run(query) 兼容重载（等价 master Portfolio::run(query)，见 design.md §4.5） */
TEST_CASE("test_MultiSystem_run_query") {
    /** @arg 未注入固定时间轴且无子系统时，run(query) 告警返回、不抛出异常 */
    auto ms = std::make_shared<MultiSystem>("ms");
    ms->setTM(crtTM(Datetime(201111010000LL), 100000.0));
    CHECK_NOTHROW(ms->run(KQuery(Datetime(20111101), Datetime(20111230))));
    CHECK_EQ(ms->getSystemList().size(), 0);

    /** @arg 注入固定时间轴时，run(query) 以其为驱动轴，且不改写入参 query 与注入轴 */
    auto ms2 = std::make_shared<MultiSystem>("ms2");
    ms2->setTM(crtTM(Datetime(201111010000LL), 100000.0));
    auto sys = create_test_sys(3, 5);
    Stock stk = getStock("sh600000");
    REQUIRE(!stk.isNull());
    KQuery query(Datetime(20111101), Datetime(20111230));
    sys->setTO(stk.getKData(query));
    ms2->add(sys);
    ms2->setAxisMode("calendar");
    ms2->setDateAxis({Datetime(20111122), Datetime(20111123), Datetime(20111124)});

    ms2->run(query);
    /** @arg 上下文 KData 承载入参 query（子系统计算与价格查询均以此为上下文） */
    CHECK_EQ(ms2->getQuery(), query);
    /** @arg 注入的固定时间轴未被改写 */
    REQUIRE_EQ(ms2->getDateAxis().size(), 3);
    CHECK_EQ(ms2->getDateAxis()[0], Datetime(20111122));
    CHECK_EQ(ms2->getDateAxis()[2], Datetime(20111124));
    /** @arg 子系统运行后其上下文 KData 与入参 query 一致（run(kdata) 语义未破坏） */
    REQUIRE_EQ(ms2->getSystemList().size(), 1);
    CHECK_EQ(ms2->getSystemList()[0]->getQuery(), query);
}

/** @par 检测点：外部调仓日表注入（归一化去重、忽略 Null）/ 覆盖式写入 / clone 保真 / 清空回退 */
TEST_CASE("test_MultiSystem_adjust_dates") {
    auto ms = std::make_shared<MultiSystem>("ms");
    /** @arg 初始为空 → 回退 m_adjust_cycle 的收盘日计数判定 */
    CHECK_EQ(ms->getAdjustDates().size(), 0);

    /** @arg 注入后归一化为当日零点并去重；Null 日期（默认构造）被忽略 */
    ms->setAdjustDates({Datetime(20250106), Datetime(2025, 1, 6, 10, 30), Datetime(20250107),
                        Datetime()});
    CHECK_EQ(ms->getAdjustDates().size(), 2);
    CHECK_UNARY(ms->getAdjustDates().count(Datetime(20250106)) == 1);
    CHECK_UNARY(ms->getAdjustDates().count(Datetime(20250107)) == 1);

    /** @arg 重复注入为覆盖式（不累加） */
    ms->setAdjustDates({Datetime(20250210)});
    CHECK_EQ(ms->getAdjustDates().size(), 1);

    /** @arg clone 保留调仓日表（clone 保真） */
    auto cloned = std::dynamic_pointer_cast<MultiSystem>(ms->clone());
    REQUIRE(cloned != nullptr);
    CHECK_EQ(cloned->getAdjustDates().size(), 1);
    CHECK_UNARY(cloned->getAdjustDates().count(Datetime(20250210)) == 1);

    /** @arg 清空后回退计数判定 */
    ms->clearAdjustDates();
    CHECK_EQ(ms->getAdjustDates().size(), 0);
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
// 组合分配引擎 L1：系统级分配（名义权重 / 真实额度）
// ============================================================================

/** @par 检测点：L1 默认等权分配（AF_EqualWeight） */
TEST_CASE("test_AllocateFunds_L1_equal_weight") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto af = AF_EqualWeight();
    af->setMode("A");
    Datetime date(200001010000LL);

    /** @arg 空 contexts 返回空权重表（边界：0 个子系统） */
    {
        SubSystemContextList empty;
        auto w = af->_allocate(date, tm, empty, KQuery());
        CHECK_EQ(w.size(), 0);
    }

    /** @arg 2 个子系统等权 1/N */
    auto sys1 = create_test_sys(3, 5);
    auto sys2 = create_test_sys(5, 10);
    SubSystemContextList contexts(2);
    contexts[0].sys = sys1;
    contexts[1].sys = sys2;
    auto w = af->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(0.5));
    CHECK_EQ(w[sys2], doctest::Approx(0.5));

    /** @arg 模式 A 不写 quota（quota 仅模式 B 产出） */
    CHECK_EQ(contexts[0].quota, doctest::Approx(0.0));
    CHECK_EQ(contexts[1].quota, doctest::Approx(0.0));
}

/** @par 检测点：L1 weight-list 固定权重（AF 基类参数，归一化语义） */
TEST_CASE("test_AllocateFunds_L1_weight_list") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto af = AF_EqualWeight();
    af->setMode("A");
    Datetime date(200001010000LL);
    auto sys1 = create_test_sys(3, 5);
    auto sys2 = create_test_sys(5, 10);
    SubSystemContextList contexts(2);
    contexts[0].sys = sys1;
    contexts[1].sys = sys2;

    /** @arg 权重和为 1，直接采用 */
    af->setParam<string>("weight-list", "0.6,0.4");
    auto w = af->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(0.6));
    CHECK_EQ(w[sys2], doctest::Approx(0.4));

    /** @arg 权重和不为 1，自动归一化（3:7 → 0.3/0.7） */
    af->setParam<string>("weight-list", "3,7");
    w = af->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(0.3));
    CHECK_EQ(w[sys2], doctest::Approx(0.7));

    /** @arg 数量与子系统不符（3 项 vs 2 子系统），回退等权 */
    af->setParam<string>("weight-list", "0.5,0.3,0.2");
    w = af->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(0.5));
    CHECK_EQ(w[sys2], doctest::Approx(0.5));

    /** @arg 负权重按 0 处理后归一化（0.5,-0.5 → 1.0/0.0） */
    af->setParam<string>("weight-list", "0.5,-0.5");
    w = af->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(1.0));
    CHECK_EQ(w[sys2], doctest::Approx(0.0));

    /** @arg 全部非法项（解析和为 0），回退等权 */
    af->setParam<string>("weight-list", "abc,def");
    w = af->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(0.5));
    CHECK_EQ(w[sys2], doctest::Approx(0.5));

    /** @arg 空串回退等权 */
    af->setParam<string>("weight-list", "");
    w = af->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(0.5));
    CHECK_EQ(w[sys2], doctest::Approx(0.5));
}

/** @par 检测点：L1 模式 B 真实额度（等权 / weight-list / fixed-amount） */
TEST_CASE("test_AllocateFunds_L1_modeB_quota") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto af = AF_EqualWeight();
    af->setMode("B");
    Datetime date(200001010000LL);
    auto sys1 = create_test_sys(3, 5);
    auto sys2 = create_test_sys(5, 10);

    /** @arg 单子系统等权：额度 = 权重(1.0) × 父总资产(100000) */
    SubSystemContextList contexts(1);
    contexts[0].sys = sys1;
    af->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(contexts[0].quota, doctest::Approx(100000.0));

    /** @arg fixed-amount>0：额度 = 固定金额（迁移 AF_FixedAmount） */
    af->setParam<double>("fixed-amount", 30000.0);
    af->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(contexts[0].quota, doctest::Approx(30000.0));

    /** @arg weight-list 影响额度分配（0.7/0.3 → 70000/30000） */
    af->setParam<double>("fixed-amount", 0.0);
    af->setParam<string>("weight-list", "0.7,0.3");
    SubSystemContextList ctx2(2);
    ctx2[0].sys = sys1;
    ctx2[1].sys = sys2;
    af->_allocate(date, tm, ctx2, KQuery());
    CHECK_EQ(ctx2[0].quota, doctest::Approx(70000.0));
    CHECK_EQ(ctx2[1].quota, doctest::Approx(30000.0));
}

/** @par 检测点：L1 固定比例（AF_FixedWeight / AF_FixedWeightList，不归一化） */
TEST_CASE("test_AllocateFunds_L1_fixed_weight") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    Datetime date(200001010000LL);
    auto sys1 = create_test_sys(3, 5);
    auto sys2 = create_test_sys(5, 10);
    SubSystemContextList contexts(2);
    contexts[0].sys = sys1;
    contexts[1].sys = sys2;

    /** @arg AF_FixedWeight(0.3)：每个子系统均返回 0.3，**不归一化**（区别于 weight-list） */
    auto af = AF_FixedWeight(0.3);
    af->setMode("A");
    auto w = af->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(0.3));
    CHECK_EQ(w[sys2], doctest::Approx(0.3));

    /** @arg 对比 weight-list 的归一化语义（0.3,0.3 → 0.5,0.5），确认 AF_FixedWeight 不走归一化 */
    auto af_norm = AF_EqualWeight();
    af_norm->setMode("A");
    af_norm->setParam<string>("weight-list", "0.3,0.3");
    auto w_norm = af_norm->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(w_norm[sys1], doctest::Approx(0.5));
    CHECK_EQ(w_norm[sys2], doctest::Approx(0.5));

    /** @arg 模式 B：额度 = 0.3 × 父总资产(100000) = 30000 */
    af->setMode("B");
    af->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(contexts[0].quota, doctest::Approx(30000.0));
    CHECK_EQ(contexts[1].quota, doctest::Approx(30000.0));

    /** @arg 空 contexts 返回空表（边界：0 个子系统） */
    {
        SubSystemContextList empty;
        auto we = af->_allocate(date, tm, empty, KQuery());
        CHECK_EQ(we.size(), 0);
    }

    /** @arg AF_FixedWeightList 按序取权重，不归一化 */
    auto afl = AF_FixedWeightList({0.4, 0.2});
    afl->setMode("A");
    w = afl->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(0.4));
    CHECK_EQ(w[sys2], doctest::Approx(0.2));

    /** @arg 数量不符（3 项 vs 2 子系统）回退等权 */
    auto afl3 = AF_FixedWeightList({0.4, 0.2, 0.1});
    afl3->setMode("A");
    w = afl3->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(0.5));
    CHECK_EQ(w[sys2], doctest::Approx(0.5));

    /** @arg 空权重列表回退等权（边界） */
    auto afl0 = AF_FixedWeightList(PriceList{});
    afl0->setMode("A");
    w = afl0->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(0.5));
    CHECK_EQ(w[sys2], doctest::Approx(0.5));
}

/** @par 检测点：L1 多因子得分权重（AF_MultiFactor：以 SubSystemContext::score 为权重） */
TEST_CASE("test_AllocateFunds_L1_multifactor") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    Datetime date(200001010000LL);
    auto sys1 = create_test_sys(3, 5);
    auto sys2 = create_test_sys(5, 10);
    SubSystemContextList contexts(2);
    contexts[0].sys = sys1;
    contexts[0].score = 0.7;
    contexts[1].sys = sys2;
    contexts[1].score = 0.3;

    auto af = AF_MultiFactor();
    af->setMode("A");

    /** @arg 以得分直接作为权重（不归一化） */
    auto w = af->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(0.7));
    CHECK_EQ(w[sys2], doctest::Approx(0.3));

    /** @arg 负分按 0 处理 */
    contexts[1].score = -1.0;
    w = af->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(0.7));
    CHECK_EQ(w[sys2], doctest::Approx(0.0));

    /** @arg 全部得分为 0（无 SE/非调仓日）：回退等权，避免零权重无法分配 */
    contexts[0].score = 0.0;
    contexts[1].score = 0.0;
    w = af->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(w[sys1], doctest::Approx(0.5));
    CHECK_EQ(w[sys2], doctest::Approx(0.5));

    /** @arg 模式 B：额度 = 得分 × 父总资产 */
    contexts[0].score = 0.6;
    contexts[1].score = 0.4;
    af->setMode("B");
    af->_allocate(date, tm, contexts, KQuery());
    CHECK_EQ(contexts[0].quota, doctest::Approx(60000.0));
    CHECK_EQ(contexts[1].quota, doctest::Approx(40000.0));

    /** @arg 空 contexts 返回空表（边界） */
    {
        SubSystemContextList empty;
        CHECK_EQ(af->_allocate(date, tm, empty, KQuery()).size(), 0);
    }
}

// ============================================================================
// 组合分配引擎 L2：行为级换算（模式 A 按比重 / 模式 B 透传）
// ============================================================================

/** @par 检测点：L2 模式 A 按 assets_ratio 换算（设计 8.3 完整语义透传，F2） */
TEST_CASE("test_AllocateFunds_L2_modeA_by_ratio") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);  // 未建仓，total_assets=100000（纯现金，可预测）
    auto af = AF_EqualWeight();
    af->setMode("A");
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
    af->_toTargets(date, tm, suggestions, weights, KQuery());
    CHECK(suggestions[0].type == SuggestionType::BUY);
    CHECK_EQ(suggestions[0].number, doctest::Approx(5000.0));

    /** @arg assets_ratio<=0：回退满仓（ratio=1）等权到仓 → 100000/10 = 10000 股 */
    s.assets_ratio = 0.0;
    suggestions[0] = s;
    af->_toTargets(date, tm, suggestions, weights, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(10000.0));

    /** @arg plan_price<=0：数量置 0（防除零边界） */
    s.plan_price = 0.0;
    s.assets_ratio = 0.5;
    suggestions[0] = s;
    af->_toTargets(date, tm, suggestions, weights, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(0.0));
}

/** @par 检测点：L2 模式 A 超配转减仓（delta<0 → SELL，F2 再平衡分支） */
TEST_CASE("test_AllocateFunds_L2_overweight_to_sell") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto af = AF_EqualWeight();
    af->setMode("A");
    // 用 fixed-amount 固定目标市值，规避建仓后 total_assets 受行情影响的不确定性
    af->setParam<double>("fixed-amount", 5000.0);
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
    af->_toTargets(date, tm, suggestions, weights, KQuery());
    CHECK(suggestions[0].type == SuggestionType::SELL);
    CHECK_EQ(suggestions[0].number, doctest::Approx(500.0));
}

/** @par 检测点：L2 模式 A SELL/CLEAR 全平当前持仓 */
TEST_CASE("test_AllocateFunds_L2_sell_clear") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto af = AF_EqualWeight();
    af->setMode("A");
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
    af->_toTargets(date, tm, suggestions, weights, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(-1000.0));

    /** @arg CLEAR：同样全平 */
    s.type = SuggestionType::CLEAR;
    suggestions[0] = s;
    af->_toTargets(date, tm, suggestions, weights, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(-1000.0));

    /** @arg 空持仓 SELL：number = -0 = 0（边界） */
    auto tm2 = crtTM(Datetime(200001010000LL), 100000.0);
    s.type = SuggestionType::SELL;
    s.number = 500;
    suggestions[0] = s;
    af->_toTargets(date, tm2, suggestions, weights, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(0.0));
}

/** @par 检测点：L2 fixed-amount 每标的固定金额（迁移 AF_FixedAmount，F4） */
TEST_CASE("test_AllocateFunds_L2_fixed_amount") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto af = AF_EqualWeight();
    af->setMode("A");
    af->setParam<double>("fixed-amount", 20000.0);
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
    af->_toTargets(date, tm, suggestions, weights, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(2000.0));
}

/** @par 检测点：L2 模式 B 透传子系统指令（父不换算，仅防御性裁剪 SELL） */
TEST_CASE("test_AllocateFunds_L2_modeB_passthrough") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    auto af = AF_EqualWeight();
    af->setMode("B");
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
    af->_toTargets(date, tm, suggestions, weights, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(888.0));

    /** @arg SELL 裁剪不超父当前持仓（current=0 → 裁剪为 0） */
    s.type = SuggestionType::SELL;
    s.number = 1000;
    suggestions[0] = s;
    af->_toTargets(date, tm, suggestions, weights, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(0.0));
}

// ============================================================================
// 组合分配引擎 L3：组合风控裁剪
// ============================================================================

/** @par 检测点：L3 集中度风控（max-single-position 裁剪 / 不限制 / 模式 B 跳过） */
TEST_CASE("test_AllocateFunds_L3_check_risk") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);  // 未建仓，total_assets=100000
    auto af = AF_EqualWeight();
    af->setMode("A");
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
    af->setParam<double>("max-single-position", 0.5);
    TradeSuggestionList suggestions{s};
    af->_checkRisk(date, tm, suggestions, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(5000.0));

    /** @arg max-single-position>=1.0：不限制（数量不变） */
    af->setParam<double>("max-single-position", 1.0);
    suggestions[0] = s;
    af->_checkRisk(date, tm, suggestions, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(20000.0));

    /** @arg 模式 B：跳过风控（尊重子管理人自主权，数量不变） */
    af->setMode("B");
    af->setParam<double>("max-single-position", 0.5);
    suggestions[0] = s;
    af->_checkRisk(date, tm, suggestions, KQuery());
    CHECK_EQ(suggestions[0].number, doctest::Approx(20000.0));
}

// ============================================================================
// v5：adjust-mode 内化（调仓日展开）与 PF/AF 兼容工厂
// ============================================================================

/** @par 检测点：adjust-mode 参数（默认值、大小写归一、非法回退）与 delay-to-trading-day 开关 */
TEST_CASE("test_MultiSystem_adjust_mode") {
    MultiSystem ms("ms");
    /** @arg 默认 adjust-mode 为 "query"，delay-to-trading-day 为 true */
    CHECK_EQ(ms.getAdjustMode(), "query");
    CHECK_EQ(ms.getParam<string>("adjust-mode"), "query");
    CHECK_EQ(ms.getDelayToTradingDay(), true);

    /** @arg 合法取值均可设置（大小写归一为小写） */
    ms.setAdjustMode("WEEK");
    CHECK_EQ(ms.getAdjustMode(), "week");
    ms.setAdjustMode("Month");
    CHECK_EQ(ms.getAdjustMode(), "month");
    ms.setAdjustMode("quarter");
    CHECK_EQ(ms.getAdjustMode(), "quarter");
    ms.setAdjustMode("year");
    CHECK_EQ(ms.getAdjustMode(), "year");
    ms.setAdjustMode("day");
    CHECK_EQ(ms.getAdjustMode(), "day");
    ms.setAdjustMode("query");
    CHECK_EQ(ms.getAdjustMode(), "query");

    /** @arg 非法取值回退 query（不抛异常） */
    ms.setAdjustMode("monthly");
    CHECK_EQ(ms.getAdjustMode(), "query");
    ms.setAdjustMode("");
    CHECK_EQ(ms.getAdjustMode(), "query");

    /** @arg delay-to-trading-day 可独立开关 */
    ms.setDelayToTradingDay(false);
    CHECK_EQ(ms.getDelayToTradingDay(), false);
    ms.setDelayToTradingDay(true);
    CHECK_EQ(ms.getDelayToTradingDay(), true);
}

/** @par 检测点：calcAdjustDates 纯函数（week/month/quarter/year × delay 真/假 + 边界） */
TEST_CASE("test_MultiSystem_calc_adjust_dates") {
    /** @arg 空轴返回空（边界：0 个交易日） */
    CHECK_EQ(MultiSystem::calcAdjustDates(DatetimeList(), "month", 1, true).size(), 0);

    /** @arg 非法 mode 返回空 */
    DatetimeList axis{Datetime(2025, 1, 1), Datetime(2025, 1, 2), Datetime(2025, 1, 6)};
    CHECK_EQ(MultiSystem::calcAdjustDates(axis, "query", 1, true).size(), 0);
    CHECK_EQ(MultiSystem::calcAdjustDates(axis, "day", 1, false).size(), 0);
    CHECK_EQ(MultiSystem::calcAdjustDates(axis, "", 1, false).size(), 0);

    // 2025-01 真实日历：Jan1=Wed, Jan2=Thu, Jan3=Fri, Jan6=Mon, Jan7=Tue, ... Jan10=Fri,
    //                 Jan13=Mon, Jan14=Tue, Jan15=Wed, Jan16=Thu, Jan17=Fri, Jan20=Mon,
    //                 Jan21=Tue, Jan22=Wed, Jan23=Thu, Jan24=Fri（模拟交易日轴，跳过 1/1 时见下）
    DatetimeList jan{Datetime(2025, 1, 1),  Datetime(2025, 1, 2),  Datetime(2025, 1, 3),
                     Datetime(2025, 1, 6),  Datetime(2025, 1, 7),  Datetime(2025, 1, 8),
                     Datetime(2025, 1, 9),  Datetime(2025, 1, 10), Datetime(2025, 1, 13),
                     Datetime(2025, 1, 14), Datetime(2025, 1, 15), Datetime(2025, 1, 16),
                     Datetime(2025, 1, 17), Datetime(2025, 1, 20), Datetime(2025, 1, 21),
                     Datetime(2025, 1, 22), Datetime(2025, 1, 23), Datetime(2025, 1, 24)};

    /** @arg week/1 非顺延：命中全部周一（dayOfWeek()==1） */
    {
        auto r = MultiSystem::calcAdjustDates(jan, "week", 1, false);
        REQUIRE_EQ(r.size(), 3);
        CHECK_EQ(r[0], Datetime(2025, 1, 6));
        CHECK_EQ(r[1], Datetime(2025, 1, 13));
        CHECK_EQ(r[2], Datetime(2025, 1, 20));
    }

    /** @arg week/2 非顺延：命中全部周二（Jan 7/14/21） */
    {
        auto r = MultiSystem::calcAdjustDates(jan, "week", 2, false);
        REQUIRE_EQ(r.size(), 3);
        CHECK_EQ(r[0], Datetime(2025, 1, 7));
        CHECK_EQ(r[2], Datetime(2025, 1, 21));
    }

    /** @arg month/1 非顺延：仅命中「日==1」的交易日（此处为 Jan 1） */
    {
        auto r = MultiSystem::calcAdjustDates(jan, "month", 1, false);
        REQUIRE_EQ(r.size(), 1);
        CHECK_EQ(r[0], Datetime(2025, 1, 1));
    }

    /** @arg month/15 非顺延：仅命中「日==15」的交易日 */
    {
        auto r = MultiSystem::calcAdjustDates(jan, "month", 15, false);
        REQUIRE_EQ(r.size(), 1);
        CHECK_EQ(r[0], Datetime(2025, 1, 15));
    }

    /** @arg month/32 非顺延：不存在第 32 日（边界：超范围） */
    CHECK_EQ(MultiSystem::calcAdjustDates(jan, "month", 32, false).size(), 0);

    /** @arg quarter/1 | year/1 非顺延：当月/当年第 1 日（Jan 1） */
    {
        auto rq = MultiSystem::calcAdjustDates(jan, "quarter", 1, false);
        REQUIRE_EQ(rq.size(), 1);
        CHECK_EQ(rq[0], Datetime(2025, 1, 1));
        auto ry = MultiSystem::calcAdjustDates(jan, "year", 1, false);
        REQUIRE_EQ(ry.size(), 1);
        CHECK_EQ(ry[0], Datetime(2025, 1, 1));
    }

    /** @arg adjust_cycle<=0 视为 1（边界：0 与负值） */
    {
        auto r0 = MultiSystem::calcAdjustDates(jan, "week", 0, false);
        auto r1 = MultiSystem::calcAdjustDates(jan, "week", 1, false);
        REQUIRE_EQ(r0.size(), r1.size());
        CHECK_EQ(r0[0], r1[0]);
        auto rn = MultiSystem::calcAdjustDates(jan, "week", -5, false);
        REQUIRE_EQ(rn.size(), r1.size());
        CHECK_EQ(rn[0], r1[0]);
    }

    /** @arg month/1 顺延：Jan 1 非交易日时顺延至当周期首个交易日（Jan 2） */
    {
        DatetimeList jan2{Datetime(2025, 1, 2), Datetime(2025, 1, 3), Datetime(2025, 1, 6),
                          Datetime(2025, 1, 7), Datetime(2025, 1, 8)};
        auto r = MultiSystem::calcAdjustDates(jan2, "month", 1, true);
        REQUIRE_EQ(r.size(), 1);
        CHECK_EQ(r[0], Datetime(2025, 1, 2));
        /** @arg 同一周期不重复命中（Jan 3/6/7/8 不再命中） */
    }

    /** @arg month/1 顺延：目标日恰为交易日时命中当日 */
    {
        auto r = MultiSystem::calcAdjustDates(jan, "month", 1, true);
        REQUIRE_EQ(r.size(), 1);
        CHECK_EQ(r[0], Datetime(2025, 1, 1));
    }

    /** @arg week/1 顺延：周一为假期时顺延至当周首个交易日（Jan 6 为周一 → 直接命中） */
    {
        DatetimeList noMon{Datetime(2025, 1, 7), Datetime(2025, 1, 8), Datetime(2025, 1, 9),
                           Datetime(2025, 1, 10), Datetime(2025, 1, 14)};
        auto r = MultiSystem::calcAdjustDates(noMon, "week", 1, true);
        REQUIRE_EQ(r.size(), 2);
        CHECK_EQ(r[0], Datetime(2025, 1, 7));   // Jan 6 缺失 → 顺延至当周首个交易日 Jan 7
        CHECK_EQ(r[1], Datetime(2025, 1, 14));  // Jan 13 缺失 → 顺延至 Jan 14
    }

    /** @arg 跨月轴 quarter/1 顺延：同一季度内只命中一次（Jan 1 缺失 → Jan 2 命中；2 月不再命中） */
    {
        DatetimeList two{Datetime(2025, 1, 2), Datetime(2025, 1, 3), Datetime(2025, 2, 3),
                         Datetime(2025, 2, 4)};
        auto rq = MultiSystem::calcAdjustDates(two, "quarter", 1, true);
        REQUIRE_EQ(rq.size(), 1);
        CHECK_EQ(rq[0], Datetime(2025, 1, 2));

        /** @arg month/1 顺延：跨月轴每月各命中一次 */
        auto rm = MultiSystem::calcAdjustDates(two, "month", 1, true);
        REQUIRE_EQ(rm.size(), 2);
        CHECK_EQ(rm[0], Datetime(2025, 1, 2));
        CHECK_EQ(rm[1], Datetime(2025, 2, 3));
    }
}

/** @par 检测点：PF 兼容工厂（PF_Simple → 模式 B / PF_WithoutAF → 模式 A，参数逐项映射） */
TEST_CASE("test_PF_factories") {
    auto tm = crtTM(Datetime(200001010000LL), 100000.0);

    /** @arg PF_Simple：模式 B；af 类型为 AFPtr 并映射入 MultiSystem；SE/调仓参数逐项透传 */
    auto pf = PF_Simple(tm, SE_Fixed(), AF_FixedWeight(0.2), 5, "month", false);
    REQUIRE(pf != nullptr);
    CHECK_EQ(pf->name(), "PF_Simple");
    CHECK_EQ(pf->getMode(), "B");
    CHECK_EQ(pf->getAdjustCycle(), 5);
    CHECK_EQ(pf->getAdjustMode(), "month");
    CHECK_EQ(pf->getDelayToTradingDay(), false);
    REQUIRE(pf->getAF() != nullptr);
    CHECK_EQ(pf->getAF()->name(), "AF_FixedWeight");
    CHECK_EQ(pf->getAF()->getParam<double>("weight"), doctest::Approx(0.2));
    CHECK_UNARY(pf->getSE() != nullptr);
    /** @arg 与 master SimplePortfolio 对齐：收盘执行 + 未选中强制清仓 */
    CHECK_EQ(pf->getTradeOnClose(), true);
    CHECK_EQ(pf->getSellAtNotSelected(), true);

    /** @arg PF_Simple 缺省参数：af 缺省等权（AF_EqualWeight）、adjust_cycle=1、mode="query"、delay=true */
    auto pf_def = PF_Simple(tm, SE_Fixed());
    CHECK_EQ(pf_def->getMode(), "B");
    CHECK_EQ(pf_def->getAdjustCycle(), 1);
    CHECK_EQ(pf_def->getAdjustMode(), "query");
    CHECK_EQ(pf_def->getDelayToTradingDay(), true);
    REQUIRE(pf_def->getAF() != nullptr);
    CHECK_EQ(pf_def->getAF()->name(), "AF_EqualWeight");

    /** @arg PF_WithoutAF：模式 A；参数逐项透传；sys_use_self_tm 无对应语义但不应崩溃 */
    auto pf2 = PF_WithoutAF(tm, SE_Fixed(), 3, "week", true, false, true, true);
    REQUIRE(pf2 != nullptr);
    CHECK_EQ(pf2->name(), "PF_WithoutAF");
    CHECK_EQ(pf2->getMode(), "A");
    CHECK_EQ(pf2->getAdjustCycle(), 3);
    CHECK_EQ(pf2->getAdjustMode(), "week");
    CHECK_EQ(pf2->getDelayToTradingDay(), true);
    CHECK_EQ(pf2->getTradeOnClose(), false);
    CHECK_EQ(pf2->getSellAtNotSelected(), true);
    REQUIRE(pf2->getAF() != nullptr);
    CHECK_EQ(pf2->getAF()->name(), "AF_EqualWeight");

    /** @arg PF_WithoutAF 缺省：trade_on_close=true、sell_at_not_selected=false */
    auto pf2_def = PF_WithoutAF(tm, SE_Fixed());
    CHECK_EQ(pf2_def->getTradeOnClose(), true);
    CHECK_EQ(pf2_def->getSellAtNotSelected(), false);
    REQUIRE(pf2_def->getAF() != nullptr);
    CHECK_EQ(pf2_def->getAF()->name(), "AF_EqualWeight");
}

/** @par 检测点：AF 工厂（返回 AFPtr，各内置算法名称与参数透传） */
TEST_CASE("test_AF_factories") {
    /** @arg AF_EqualWeight → 等权分配（L1 等权 1/N） */
    auto af1 = AF_EqualWeight();
    REQUIRE(af1 != nullptr);
    CHECK_EQ(af1->name(), "AF_EqualWeight");

    /** @arg AF_FixedWeight → 固定比例（weight 透传） */
    auto af2 = AF_FixedWeight(0.25);
    REQUIRE(af2 != nullptr);
    CHECK_EQ(af2->name(), "AF_FixedWeight");
    CHECK_EQ(af2->getParam<double>("weight"), doctest::Approx(0.25));

    /** @arg AF_FixedWeightList → 固定比例列表（权重列表透传） */
    auto af3 = AF_FixedWeightList({0.3, 0.7});
    REQUIRE(af3 != nullptr);
    CHECK_EQ(af3->name(), "AF_FixedWeightList");
    auto ws = af3->getParam<PriceList>("weights");
    REQUIRE_EQ(ws.size(), 2);
    CHECK_EQ(ws[0], doctest::Approx(0.3));
    CHECK_EQ(ws[1], doctest::Approx(0.7));

    /** @arg AF_FixedAmount → 固定金额（fixed-amount 参数透传） */
    auto af4 = AF_FixedAmount(30000.0);
    REQUIRE(af4 != nullptr);
    CHECK_EQ(af4->name(), "AF_FixedAmount");
    CHECK_EQ(af4->getParam<double>("fixed-amount"), doctest::Approx(30000.0));

    /** @arg AF_MultiFactor → 多因子 */
    auto af5 = AF_MultiFactor();
    REQUIRE(af5 != nullptr);
    CHECK_EQ(af5->name(), "AF_MultiFactor");
}

/** @par 检测点：master 兼容别名（PortfolioPtr → MultiSystemPtr，AFPtr → AllocateFundsPtr，见 design.md §4.5 / §5） */
TEST_CASE("test_PF_AF_compat_aliases") {
    /** @arg PortfolioPtr 与 MultiSystemPtr 同一类型，存量 `PortfolioPtr pf = PF_Simple(...)` 可编译 */
    static_assert(std::is_same_v<PortfolioPtr, MultiSystemPtr>,
                  "PortfolioPtr must alias MultiSystemPtr");
    /** @arg AFPtr 与 AllocateFundsPtr 同一类型（v5：AF 已独立于 MM） */
    static_assert(std::is_same_v<AFPtr, AllocateFundsPtr>, "AFPtr must alias AllocateFundsPtr");

    auto tm = crtTM(Datetime(200001010000LL), 100000.0);
    PortfolioPtr pf = PF_Simple(tm, SE_Fixed(), AF_FixedWeight(0.2));
    REQUIRE(pf != nullptr);
    CHECK_EQ(pf->getMode(), "B");

    AFPtr af = AF_FixedWeightList({0.4, 0.6});
    REQUIRE(af != nullptr);
    CHECK_EQ(af->name(), "AF_FixedWeightList");
}

/** @} */
