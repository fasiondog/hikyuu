/*
 * test_MF_IndustryNeutralize.cpp
 *
 * 行业中性化的白盒单元测试 + 端到端路由验证。
 *
 *   - 白盒用例直接测试 calculate_industry_residuals 纯函数（经 industry_neutralize.h
 *     inline 暴露），构造 PriceList 验证数学性质，无任何外部数据依赖。
 *   - E2E 用例验证 _buildDummyIndex -> calculate_industry_residuals 的路由连通性，
 *     依赖 test_data/block/hybk.ini（中文板块名，需 supportChineseSimple）。
 */

#include "../../test_config.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/utilities/runtimeinfo.h>
#include <hikyuu/trade_sys/multifactor/MultiFactorBase.h>
#include <hikyuu/trade_sys/multifactor/buildin_norm.h>
#include <hikyuu/trade_sys/multifactor/industry_neutralize.h>
#include <hikyuu/trade_sys/multifactor/crt/MF_EqualWeight.h>

using namespace hku;

/**
 * @defgroup test_MF_IndustryNeutralize test_MF_IndustryNeutralize
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

//-----------------------------------------------------------------------------
// 白盒测试：calculate_industry_residuals 纯函数
//-----------------------------------------------------------------------------

/** @par 置换不变性 —— 核心反例，锁定本次 bug 修复
 *  行业中性化结果必须与板块编号方式无关。原实现对整数编号做单变量回归，
 *  重排板块列表会改变残差；组内去均值则不受编号影响。
 */
TEST_CASE("test_industry_residuals_permutation_invariance") {
    PriceList y{10.0, 20.0, 30.0, 40.0};
    size_t blk_count = 2;

    // 场景 A：组 0 = {10,20}(均值15)，组 1 = {30,40}(均值35)
    PriceList labelsA{0.0, 0.0, 1.0, 1.0};
    auto resA = calculate_industry_residuals(y, labelsA, blk_count);
    CHECK_EQ(resA.size(), y.size());
    CHECK_EQ(resA[0], doctest::Approx(-5.0).epsilon(1e-9));
    CHECK_EQ(resA[1], doctest::Approx(5.0).epsilon(1e-9));
    CHECK_EQ(resA[2], doctest::Approx(-5.0).epsilon(1e-9));
    CHECK_EQ(resA[3], doctest::Approx(5.0).epsilon(1e-9));

    // 场景 B：完全颠倒行业编号 —— 残差必须与 A 完全一致
    PriceList labelsB{1.0, 1.0, 0.0, 0.0};
    auto resB = calculate_industry_residuals(y, labelsB, blk_count);
    for (size_t i = 0; i < y.size(); i++) {
        CHECK_EQ(resB[i], doctest::Approx(resA[i]).epsilon(1e-9));
    }
}

/** @par 边界：行业孤儿（无归属）与标签 NaN —— 残差置 NaN，不参与组内均值 */
TEST_CASE("test_industry_residuals_orphan_and_nan_label") {
    PriceList y{10.0, 20.0, 30.0};
    size_t blk_count = 2;
    // 索引1: label=2 >= blk_count (孤儿)；索引2: 标签 NaN
    PriceList labels{0.0, 2.0, Null<price_t>()};
    auto res = calculate_industry_residuals(y, labels, blk_count);

    // 索引0: 组0 仅自身，残差 = 10 - 10 = 0
    CHECK_EQ(res[0], doctest::Approx(0.0).epsilon(1e-9));
    // 孤儿与标签缺失：残差 NaN
    CHECK_UNARY(std::isnan(res[1]));
    CHECK_UNARY(std::isnan(res[2]));
}

/** @par 边界：k=1 全市场同行业 —— 退化为去全局均值 */
TEST_CASE("test_industry_residuals_single_group_global_mean") {
    PriceList y{10.0, 20.0, 30.0};
    size_t blk_count = 1;
    PriceList labels{0.0, 0.0, 0.0};
    auto res = calculate_industry_residuals(y, labels, blk_count);
    // 全局均值 = 20，残差 = {-10, 0, 10}
    CHECK_EQ(res[0], doctest::Approx(-10.0).epsilon(1e-9));
    CHECK_EQ(res[1], doctest::Approx(0.0).epsilon(1e-9));
    CHECK_EQ(res[2], doctest::Approx(10.0).epsilon(1e-9));
}

/** @par 边界：同行业含无效数据 —— 算组内均值时跳过 NaN，残差对无效点置 NaN */
TEST_CASE("test_industry_residuals_group_with_invalid_y") {
    PriceList y{10.0, Null<price_t>(), 20.0};
    size_t blk_count = 1;
    PriceList labels{0.0, 0.0, 0.0};
    auto res = calculate_industry_residuals(y, labels, blk_count);
    // 有效组内均值 = (10+20)/2 = 15
    CHECK_EQ(res[0], doctest::Approx(-5.0).epsilon(1e-9));
    CHECK_UNARY(std::isnan(res[1]));
    CHECK_EQ(res[2], doctest::Approx(5.0).epsilon(1e-9));
}

/** @par 边界：全 NaN/Inf 输入 —— 不崩溃，不抛浮点异常，返回全 NaN */
TEST_CASE("test_industry_residuals_all_invalid") {
    PriceList y{Null<price_t>(), std::numeric_limits<price_t>::infinity(),
                -std::numeric_limits<price_t>::infinity()};
    size_t blk_count = 2;
    PriceList labels{0.0, 0.0, 1.0};
    auto res = calculate_industry_residuals(y, labels, blk_count);
    CHECK_EQ(res.size(), y.size());
    for (size_t i = 0; i < y.size(); i++) {
        CHECK_UNARY(std::isnan(res[i]));
    }
}

/** @par 边界：负数/超大标签 —— 守卫 static_cast<size_t> 的 UB，不崩溃不越界
 *  static_cast<size_t>(负数) 是 C++ UB，可能溢出为 SIZE_MAX 导致 sums[g] 越界段错误。
 *  实现须在 cast 前用 double 比较拦截 label<0 或 label>=blk_count。
 */
TEST_CASE("test_industry_residuals_negative_and_oversized_label") {
    PriceList y{10.0, 20.0, 30.0, 40.0};
    size_t blk_count = 2;
    // label[0]=-1(负数UB风险), [1]=999(超大越界), [2]=0(合法), [3]=1(合法)
    PriceList labels{-1.0, 999.0, 0.0, 1.0};
    auto res = calculate_industry_residuals(y, labels, blk_count);
    // 负数与超大标签：残差 NaN（不崩溃不越界）
    CHECK_UNARY(std::isnan(res[0]));
    CHECK_UNARY(std::isnan(res[1]));
    // 合法标签：组0仅30（均值30，残差0），组1仅40（均值40，残差0）
    CHECK_EQ(res[2], doctest::Approx(0.0).epsilon(1e-9));
    CHECK_EQ(res[3], doctest::Approx(0.0).epsilon(1e-9));
}

//-----------------------------------------------------------------------------
// E2E 路由验证：_buildDummyIndex -> calculate_industry_residuals 连通性
//-----------------------------------------------------------------------------

/** @par 端到端路由：启用行业中性化后，同一行业股票截面残差之和 ≈ 0
 *  动态发现策略：遍历"行业板块"所有板块，找第一个含≥2只有效 K 线股票的板块
 *  作为同行业探针对。不硬编码特定股票代码，CI 数据变动自动适配；若整个分类
 *  无同行业对，REQUIRE 失败明确暴露数据不足，不静默通过。
 *
 *  注：测试环境用 QLBlockInfoDriver（读 ini），其 save 抛"Not support"，无法用
 *  sm.addBlock 注入内存 Mock，故采用动态发现。
 */
TEST_CASE("test_mf_industry_neutralize_routing") {
    if (!supportChineseSimple()) {
        return;
    }

    StockManager& sm = StockManager::instance();
    KQuery query = KQuery(-30);

    // 动态发现一对同行业且有数据的股票
    Stock stk_a, stk_b;
    auto blks = sm.getBlockList("行业板块");
    for (const auto& blk : blks) {
        auto stocks = blk.getStockList();
        Stock first, second;
        for (const auto& s : stocks) {
            if (s.isNull() || s.getKData(query).size() < 10) {
                continue;
            }
            if (first.isNull()) {
                first = s;
            } else {
                second = s;
                break;
            }
        }
        if (!second.isNull()) {
            stk_a = first;
            stk_b = second;
            break;
        }
    }
    REQUIRE_UNARY_FALSE(stk_a.isNull());
    REQUIRE_UNARY_FALSE(stk_b.isNull());

    Stock ref_stk = sm["sh000001"];
    StockList stks{stk_a, stk_b};

    IndicatorList src_inds{MA(CLOSE(), 5)};
    auto mf = MF_EqualWeight(src_inds, stks, query, ref_stk);
    mf->setParam<bool>("save_all_factors", true);
    mf->setNormalize(NORM_NOTHING());
    mf->addSpecialNormalize("MA", NORM_NOTHING(), "行业板块");

    // 关键：必须先触发 calculate() 填充 m_ref_dates，否则 getAllSrcFactors() 因
    // m_ref_dates 为空（days_total=0）跳过截面中性化循环，静默返回原始因子值。
    mf->getDatetimeList();

    auto all_src = mf->getAllSrcFactors();
    CHECK_EQ(all_src.size(), stks.size());

    auto ref_dates = mf->getDatetimeList();
    REQUIRE_GT(ref_dates.size(), 5u);
    size_t di = ref_dates.size() / 2;

    // 反静默失效探针：同行业残差和≈0。若 name 路由匹配失败（ALIGN 改名导致
    // m_special_category 不命中），输出原始 MA 值，两只不同股价 MA 之和不会为 0。
    double r0 = all_src[0][0][di];
    double r1 = all_src[1][0][di];
    CHECK_UNARY_FALSE(std::isnan(r0));
    CHECK_UNARY_FALSE(std::isnan(r1));
    CHECK_EQ(r0 + r1, doctest::Approx(0.0).epsilon(1e-6));
}

/** @} */
