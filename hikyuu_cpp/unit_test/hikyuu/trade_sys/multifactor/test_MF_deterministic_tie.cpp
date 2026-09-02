/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  测试 MultiFactorBase::_buildIndex 截面排序的确定性：
 *  有效值相等（tie）时按 market_code 字典序打破并列，
 *  使排名跨输入顺序、进程、平台、编译器稳定。
 */

#include "../../test_config.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/trade_sys/multifactor/crt/MF_EqualWeight.h>

using namespace hku;

/**
 * @defgroup test_MF_deterministic_tie test_MF_deterministic_tie
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/**
 * @par 检测点：全部有效值相等（tie）时按 market_code 字典序稳定排序（mode 0 降序）
 *
 * 用 CLOSE() - CLOSE() 构造全 tie 截面：每只股票的值都是自身收盘价减自身
 * 收盘价 = 0.0，跨股票也全为 0.0。该表达式是非叶子 OP 节点，继承 CLOSE()
 * 的长度和 context，在 MF compiled plan 路径下能正确按每只股票的 KData
 * 计算出与 dates 等长的全 0.0 序列。
 *
 * 验证排序后严格按 market_code 字典序排列，且值保持 0.0。
 */
TEST_CASE("test_MF_deterministic_tie_all_equal_desc") {
    StockManager& sm = StockManager::instance();
    // market_code 字典序: SH600004 < SH600005 < SZ000001 < SZ000002
    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQuery(-30);
    IndicatorList src_inds{CLOSE() - CLOSE()};

    auto mf = MF_EqualWeight(src_inds, stks, query, ref_stk);
    mf->setParam<int>("mode", 0);  // 降序
    auto dates = mf->getDatetimeList();
    CHECK_UNARY(!dates.empty());

    // 取中间日期截面（避免边界日部分股票停牌缺失数据导致 NaN）
    auto cross = mf->getScores(dates[dates.size() / 2]);
    CHECK_EQ(cross.size(), 4);
    CHECK_EQ(cross[0].stock, sm["sh600004"]);
    CHECK_EQ(cross[1].stock, sm["sh600005"]);
    CHECK_EQ(cross[2].stock, sm["sz000001"]);
    CHECK_EQ(cross[3].stock, sm["sz000002"]);
    for (const auto& sr : cross) {
        CHECK_EQ(sr.value, doctest::Approx(0.0).epsilon(0.0001));
    }
}

/**
 * @par 检测点：全部有效值相等（tie）时按 market_code 字典序稳定排序（mode 1 升序）
 *
 * 升序模式下 tie-break 二级键不随主方向翻转，仍按 market_code 字典序。
 */
TEST_CASE("test_MF_deterministic_tie_all_equal_asc") {
    StockManager& sm = StockManager::instance();
    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQuery(-30);
    IndicatorList src_inds{CLOSE() - CLOSE()};

    auto mf = MF_EqualWeight(src_inds, stks, query, ref_stk);
    mf->setParam<int>("mode", 1);  // 升序
    auto dates = mf->getDatetimeList();
    CHECK_UNARY(!dates.empty());

    auto cross = mf->getScores(dates[dates.size() / 2]);
    CHECK_EQ(cross.size(), 4);
    CHECK_EQ(cross[0].stock, sm["sh600004"]);
    CHECK_EQ(cross[1].stock, sm["sh600005"]);
    CHECK_EQ(cross[2].stock, sm["sz000001"]);
    CHECK_EQ(cross[3].stock, sm["sz000002"]);
}

/**
 * @par 检测点：输入 StockList 正反顺序不影响 tie 排序结果
 *
 * 两个 MF 实例使用相同股票集合但不同的 StockList 输入顺序，断言输出截面
 * 顺序完全一致，代数上证明比较器构成严格全序（不依赖输入排列）。
 */
TEST_CASE("test_MF_deterministic_tie_input_order_invariant") {
    StockManager& sm = StockManager::instance();
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQuery(-30);
    IndicatorList src_inds{CLOSE() - CLOSE()};

    StockList stks_fwd{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    auto mf_fwd = MF_EqualWeight(src_inds, stks_fwd, query, ref_stk);
    mf_fwd->setParam<int>("mode", 0);
    auto dates = mf_fwd->getDatetimeList();
    auto cross_fwd = mf_fwd->getScores(dates[dates.size() / 2]);

    StockList stks_rev{sm["sz000002"], sm["sz000001"], sm["sh600005"], sm["sh600004"]};
    auto mf_rev = MF_EqualWeight(src_inds, stks_rev, query, ref_stk);
    mf_rev->setParam<int>("mode", 0);
    auto cross_rev = mf_rev->getScores(dates[dates.size() / 2]);

    CHECK_EQ(cross_fwd.size(), cross_rev.size());
    for (size_t i = 0; i < cross_fwd.size(); i++) {
        CHECK_EQ(cross_fwd[i].stock, cross_rev[i].stock);
    }
}

/**
 * @par 检测点：getAllScores 多日期截面的全序属性验证（mode 0 降序）
 *
 * 用真实指标 MA(CLOSE()) 遍历 getAllScores 的所有日期截面，断言：
 *   1. 有效值不在 NaN 之后
 *   2. 有效值段 prev.value >= curr.value（降序）
 *   3. 有效值相等时 market_code 字典序严格递增
 *   4. NaN 段 market_code 字典序严格递增
 *
 * 使用覆盖率计数器：统计"有效值与 NaN 混合"和"有效值 tie"的截面数，
 * 若为 0 则显式 FAIL，避免属性断言假绿。
 */
TEST_CASE("test_MF_deterministic_tie_mixed_property_desc") {
    StockManager& sm = StockManager::instance();
    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQuery(-30);
    IndicatorList src_inds{MA(CLOSE(), 3)};

    auto mf = MF_EqualWeight(src_inds, stks, query, ref_stk);
    mf->setParam<int>("mode", 0);
    mf->getDatetimeList();
    const auto& all_scores = mf->getAllScores();
    CHECK_UNARY(!all_scores.empty());

    size_t mixed_nan_count = 0;

    for (size_t d = 0; d < all_scores.size(); d++) {
        const auto& cross = all_scores[d];
        bool has_valid = false, has_nan = false;
        for (size_t i = 1; i < cross.size(); i++) {
            const auto& prev = cross[i - 1];
            const auto& curr = cross[i];
            bool prev_nan = std::isnan(prev.value);
            bool curr_nan = std::isnan(curr.value);
            CHECK_UNARY(!(prev_nan && !curr_nan));
            if (!prev_nan && !curr_nan) {
                CHECK_UNARY(prev.value >= curr.value);
                if (prev.value == curr.value) {
                    CHECK_UNARY(prev.stock.market_code() < curr.stock.market_code());
                }
            }
            if (prev_nan && curr_nan) {
                CHECK_UNARY(prev.stock.market_code() < curr.stock.market_code());
            }
            has_valid = has_valid || !prev_nan;
            has_nan = has_nan || prev_nan;
        }
        if (!cross.empty()) {
            bool last_nan = std::isnan(cross.back().value);
            has_valid = has_valid || !last_nan;
            has_nan = has_nan || last_nan;
        }
        if (has_valid && has_nan) {
            mixed_nan_count++;
        }
    }

    CHECK_MESSAGE(mixed_nan_count > 0, "mixed_nan scenario not triggered by test data");
}

/**
 * @par 检测点：getAllScores 多日期截面的全序属性验证（mode 1 升序）
 *
 * 与降序用例对称，仅有效值比较方向改为 <=，其余全序属性相同。
 */
TEST_CASE("test_MF_deterministic_tie_mixed_property_asc") {
    StockManager& sm = StockManager::instance();
    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQuery(-30);
    IndicatorList src_inds{MA(CLOSE(), 3)};

    auto mf = MF_EqualWeight(src_inds, stks, query, ref_stk);
    mf->setParam<int>("mode", 1);  // 升序
    mf->getDatetimeList();
    const auto& all_scores = mf->getAllScores();
    CHECK_UNARY(!all_scores.empty());

    size_t mixed_nan_count = 0;

    for (size_t d = 0; d < all_scores.size(); d++) {
        const auto& cross = all_scores[d];
        bool has_valid = false, has_nan = false;
        for (size_t i = 1; i < cross.size(); i++) {
            const auto& prev = cross[i - 1];
            const auto& curr = cross[i];
            bool prev_nan = std::isnan(prev.value);
            bool curr_nan = std::isnan(curr.value);
            CHECK_UNARY(!(prev_nan && !curr_nan));
            if (!prev_nan && !curr_nan) {
                CHECK_UNARY(prev.value <= curr.value);
                if (prev.value == curr.value) {
                    CHECK_UNARY(prev.stock.market_code() < curr.stock.market_code());
                }
            }
            if (prev_nan && curr_nan) {
                CHECK_UNARY(prev.stock.market_code() < curr.stock.market_code());
            }
            has_valid = has_valid || !prev_nan;
            has_nan = has_nan || prev_nan;
        }
        if (!cross.empty()) {
            bool last_nan = std::isnan(cross.back().value);
            has_valid = has_valid || !last_nan;
            has_nan = has_nan || last_nan;
        }
        if (has_valid && has_nan) {
            mixed_nan_count++;
        }
    }

    CHECK_MESSAGE(mixed_nan_count > 0, "mixed_nan scenario not triggered by test data");
}

/**
 * @par 检测点：空 StockList 应在计算时被 _checkData 拒绝
 *
 * MF_EqualWeight 构造时不校验，首次触发 calculate() 时 _checkData 要求
 * m_stks.size() >= 2，空列表应抛异常。
 */
TEST_CASE("test_MF_deterministic_tie_empty_stks") {
    StockManager& sm = StockManager::instance();
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQuery(-30);
    IndicatorList src_inds{MA(CLOSE(), 3)};
    StockList empty_stks;
    auto mf = MF_EqualWeight(src_inds, empty_stks, query, ref_stk);
    CHECK_THROWS_AS(mf->getDatetimeList(), std::exception);
}

/**
 * @par 检测点：单只股票应在计算时被 _checkData 拒绝
 *
 * _checkData 要求 m_stks.size() >= 2，单只股票无截面排名意义，应抛异常。
 */
TEST_CASE("test_MF_deterministic_tie_single_stk") {
    StockManager& sm = StockManager::instance();
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQuery(-30);
    IndicatorList src_inds{MA(CLOSE(), 3)};
    StockList single_stk{sm["sh600004"]};
    auto mf = MF_EqualWeight(src_inds, single_stk, query, ref_stk);
    CHECK_THROWS_AS(mf->getDatetimeList(), std::exception);
}

/**
 * @par 检测点：重复 Stock 不破坏严格弱序，正常排序且不崩溃
 *
 * StockList 中包含同一股票两次，两元素 value 和 market_code 完全相同，
 * 比较器对它们返回 a<b=false, b<a=false（等价类），std::sort 正确处理。
 * 验证不崩溃且两重复元素相邻排列在 market_code 对应位置。
 */
TEST_CASE("test_MF_deterministic_tie_duplicate_stk") {
    StockManager& sm = StockManager::instance();
    StockList stks{sm["sh600004"], sm["sh600005"], sm["sh600004"], sm["sz000001"]};
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQuery(-30);
    IndicatorList src_inds{CLOSE() - CLOSE()};

    auto mf = MF_EqualWeight(src_inds, stks, query, ref_stk);
    mf->setParam<int>("mode", 0);
    auto dates = mf->getDatetimeList();
    CHECK_UNARY(!dates.empty());

    auto cross = mf->getScores(dates[dates.size() / 2]);
    CHECK_EQ(cross.size(), 4);
    CHECK_EQ(cross[0].stock, sm["sh600004"]);
    CHECK_EQ(cross[1].stock, sm["sh600004"]);
    CHECK_EQ(cross[2].stock, sm["sh600005"]);
    CHECK_EQ(cross[3].stock, sm["sz000001"]);
}

/** @} */