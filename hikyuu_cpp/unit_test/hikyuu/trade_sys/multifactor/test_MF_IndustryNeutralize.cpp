/*
 * test_MF_IndustryNeutralize.cpp
 *
 * The white box unit test of the industry neutralization + the end-to-end routing verification.
 *
 *   - The white box case tests the calculate_industry_residuals pure function directly (exposed
 *     inline via industry_neutralize.h); a PriceList is built to verify the math properties with no
 * external data dependency.
 *   - The E2E case verifies the routing connectivity of _buildDummyIndex ->
 * calculate_industry_residuals, depending on test_data/block/hybk.ini (Chinese block names,
 * supportChineseSimple needed).
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
// The white box test: the calculate_industry_residuals pure function
//-----------------------------------------------------------------------------

/** @par Permutation invariance - the core counter example that pins down this bug fix
 *  The result must not depend on the block numbering. The original implementation did a unary
 * regression on the integer numbering, so reordering the block list changed the residuals; removing
 * the group mean is unaffected.
 */
TEST_CASE("test_industry_residuals_permutation_invariance") {
    PriceList y{10.0, 20.0, 30.0, 40.0};
    size_t blk_count = 2;

    // Scenario A: group 0 = {10,20} (mean 15) and group 1 = {30,40} (mean 35)
    PriceList labelsA{0.0, 0.0, 1.0, 1.0};
    auto resA = calculate_industry_residuals(y, labelsA, blk_count);
    CHECK_EQ(resA.size(), y.size());
    CHECK_EQ(resA[0], doctest::Approx(-5.0).epsilon(1e-9));
    CHECK_EQ(resA[1], doctest::Approx(5.0).epsilon(1e-9));
    CHECK_EQ(resA[2], doctest::Approx(-5.0).epsilon(1e-9));
    CHECK_EQ(resA[3], doctest::Approx(5.0).epsilon(1e-9));

    // Scenario B: the industry numbering is reversed - the residuals must match A exactly
    PriceList labelsB{1.0, 1.0, 0.0, 0.0};
    auto resB = calculate_industry_residuals(y, labelsB, blk_count);
    for (size_t i = 0; i < y.size(); i++) {
        CHECK_EQ(resB[i], doctest::Approx(resA[i]).epsilon(1e-9));
    }
}

/** @par Boundary: an industry orphan (no membership) and a NaN label - the residual is NaN and it
 * is excluded from the group mean */
TEST_CASE("test_industry_residuals_orphan_and_nan_label") {
    PriceList y{10.0, 20.0, 30.0};
    size_t blk_count = 2;
    // Index 1: label=2 >= blk_count (an orphan); index 2: a NaN label
    PriceList labels{0.0, 2.0, Null<price_t>()};
    auto res = calculate_industry_residuals(y, labels, blk_count);

    // Index 0: group 0 has itself only, the residual = 10 - 10 = 0
    CHECK_EQ(res[0], doctest::Approx(0.0).epsilon(1e-9));
    // The orphan and the missing label: the residual is NaN
    CHECK_UNARY(std::isnan(res[1]));
    CHECK_UNARY(std::isnan(res[2]));
}

/** @par Boundary: k=1, the whole market in one industry - it degenerates to the global mean */
TEST_CASE("test_industry_residuals_single_group_global_mean") {
    PriceList y{10.0, 20.0, 30.0};
    size_t blk_count = 1;
    PriceList labels{0.0, 0.0, 0.0};
    auto res = calculate_industry_residuals(y, labels, blk_count);
    // The global mean = 20 and the residuals = {-10, 0, 10}
    CHECK_EQ(res[0], doctest::Approx(-10.0).epsilon(1e-9));
    CHECK_EQ(res[1], doctest::Approx(0.0).epsilon(1e-9));
    CHECK_EQ(res[2], doctest::Approx(10.0).epsilon(1e-9));
}

/** @par Boundary: the industry contains invalid data - the NaN is skipped in the group mean and the
 * residual of an invalid point is NaN */
TEST_CASE("test_industry_residuals_group_with_invalid_y") {
    PriceList y{10.0, Null<price_t>(), 20.0};
    size_t blk_count = 1;
    PriceList labels{0.0, 0.0, 0.0};
    auto res = calculate_industry_residuals(y, labels, blk_count);
    // The valid group mean = (10+20)/2 = 15
    CHECK_EQ(res[0], doctest::Approx(-5.0).epsilon(1e-9));
    CHECK_UNARY(std::isnan(res[1]));
    CHECK_EQ(res[2], doctest::Approx(5.0).epsilon(1e-9));
}

/** @par Boundary: an all NaN/Inf input - no crash, no floating point exception, all NaN */
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

/** @par Boundary: a negative / oversized label - guard the UB of static_cast<size_t>, no crash
 *  static_cast<size_t>(negative) is a C++ UB: it may overflow to SIZE_MAX and cause an out of range
 * segfault of sums[g]. The implementation must intercept label<0 or label>=blk_count with a double
 * compare before the  cast. */
*/ TEST_CASE("test_industry_residuals_negative_and_oversized_label") {
    PriceList y{10.0, 20.0, 30.0, 40.0};
    size_t blk_count = 2;
    // label[0]=-1 (a negative UB risk), [1]=999 (oversized), [2]=0 (valid), [3]=1 (valid)
    PriceList labels{-1.0, 999.0, 0.0, 1.0};
    auto res = calculate_industry_residuals(y, labels, blk_count);
    // The negative and the oversized labels: the residual is NaN (no crash, no out of range)
    CHECK_UNARY(std::isnan(res[0]));
    CHECK_UNARY(std::isnan(res[1]));
    // The valid labels: group 0 has 30 only (mean 30, residual 0), group 1 has 40 only (mean 40)
    CHECK_EQ(res[2], doctest::Approx(0.0).epsilon(1e-9));
    CHECK_EQ(res[3], doctest::Approx(0.0).epsilon(1e-9));
}

//-----------------------------------------------------------------------------
// The E2E routing verification: _buildDummyIndex -> calculate_industry_residuals connectivity
//-----------------------------------------------------------------------------

/** @par The end-to-end routing: with the neutralization on, the cross-sectional residual sum of the
 * stocks in one industry is about 0 The dynamic discovery: iterate over the blocks of the industry
 * category and find the first one  that contains at least 2 stocks with valid K-lines as the
 * same-industry probe pair. No stock code is hard coded, so a CI data change is adapted;  if the
 * whole category has no same-industry pair, a REQUIRE failure exposes the insufficient data instead
 * of passing  silently.
 *
 *  Note: the test environment uses QLBlockInfoDriver (reading an ini) whose save throws  "Not
 * support", so sm.addBlock cannot inject an in-memory Mock, hence the dynamic discovery.
 */
TEST_CASE("test_mf_industry_neutralize_routing") {
    if (!supportChineseSimple()) {
        return;
    }

    StockManager& sm = StockManager::instance();
    KQuery query = KQuery(-30);

    // Dynamically discover a pair of stocks in the same industry that have data
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

    // Key: calculate() must be triggered first to fill m_ref_dates, otherwise getAllSrcFactors()
    // would skip the cross-sectional loop with an empty m_ref_dates (days_total=0) and return the
    // // raw factor values silently.
    mf->getDatetimeList();

    auto all_src = mf->getAllSrcFactors();
    CHECK_EQ(all_src.size(), stks.size());

    auto ref_dates = mf->getDatetimeList();
    REQUIRE_GT(ref_dates.size(), 5u);
    size_t di = ref_dates.size() / 2;

    // A probe against a silent failure: the residual sum of the same industry is about 0. If the
    // name routing misses (an ALIGN rename makes m_special_category miss), the raw MA values are
    // output     // and the MA sum of two stocks with different prices cannot be 0.
    double r0 = all_src[0][0][di];
    double r1 = all_src[1][0][di];
    CHECK_UNARY_FALSE(std::isnan(r0));
    CHECK_UNARY_FALSE(std::isnan(r1));
    CHECK_EQ(r0 + r1, doctest::Approx(0.0).epsilon(1e-6));
}

/** @} */
