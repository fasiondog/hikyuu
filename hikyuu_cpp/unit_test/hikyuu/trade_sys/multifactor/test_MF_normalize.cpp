/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2026-10-15
 *      Author: fasiondog
 */

#include "../../test_config.h"
#include <cmath>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/EMA.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/trade_sys/multifactor/buildin_norm.h>
#include <hikyuu/trade_sys/multifactor/crt/MF_EqualWeight.h>
#include <hikyuu/trade_sys/multifactor/imp/EqualWeightMultiFactor.h>

using namespace hku;

/**
 * @defgroup test_MF_normalize test_MF_normalize
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_MF_special_normalize_only_is_applied") {
    StockManager& sm = StockManager::instance();
    StockList stks = {sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQuery(-30);
    IndicatorList inds = {MA(CLOSE(), 5), EMA(CLOSE(), 10)};

    auto mf = MF_EqualWeight(inds, stks, query, ref_stk);
    // only a special normalization is configured (no global norm / category / style)
    mf->addSpecialNormalize("MA", NORM_Zscore());
    mf->calculate();
    auto src = mf->getAllSrcFactors();

    size_t di = mf->getDatetimeList().size() - 1;
    price_t sum = 0.0;
    size_t count = 0;
    for (size_t si = 0; si < src.size(); si++) {
        price_t v = src[si][0][di];
        if (!std::isnan(v)) {
            sum += v;
            count++;
        }
    }
    REQUIRE(count >= 2);

    /** @arg the special-only normalization must actually run: the z-scored cross-section is
     * centered at 0 instead of leaking the raw price level */
    CHECK_UNARY(std::fabs(sum / count) < 1e-6);
}

/** @par Test points */
TEST_CASE("test_MF_clone_preserves_style_neutralize") {
    StockManager& sm = StockManager::instance();
    StockList stks = {sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQuery(-30);
    IndicatorList inds = {MA(CLOSE(), 5), EMA(CLOSE(), 10)};

    auto mf = MF_EqualWeight(inds, stks, query, ref_stk);
    // a style neutralization only: null norm, empty category, one style factor
    mf->addSpecialNormalize("MA", NORM_NOTHING(), "", {CLOSE()});
    mf->calculate();
    auto src = mf->getAllSrcFactors();

    /** @arg the cloned MF must keep the style neutralization config, so its processed source
     * factors match the original */
    auto mf2 = mf->clone();
    mf2->calculate();
    auto src2 = mf2->getAllSrcFactors();

    REQUIRE_EQ(src.size(), src2.size());
    for (size_t si = 0; si < src.size(); si++) {
        REQUIRE_EQ(src[si].size(), src2[si].size());
        for (size_t ii = 0; ii < src[si].size(); ii++) {
            CHECK_UNARY(src[si][ii].equal(src2[si][ii]));
        }
    }
}

/** @par Test points */
TEST_CASE("test_MF_copy_ctor_preserves_normalize_config") {
    StockManager& sm = StockManager::instance();
    StockList stks = {sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQuery(-30);
    IndicatorList inds = {MA(CLOSE(), 5), EMA(CLOSE(), 10)};

    auto mf = MF_EqualWeight(inds, stks, query, ref_stk);
    mf->setNormalize(NORM_Zscore());
    auto ptr = std::static_pointer_cast<EqualWeightMultiFactor>(mf);

    /** @arg the copy constructor must carry the normalization config, so the copied object yields
     * the same processed source factors */
    EqualWeightMultiFactor copy(*ptr);
    mf->calculate();
    copy.calculate();
    auto src = mf->getAllSrcFactors();
    auto src2 = copy.getAllSrcFactors();

    REQUIRE_EQ(src.size(), src2.size());
    for (size_t si = 0; si < src.size(); si++) {
        REQUIRE_EQ(src[si].size(), src2[si].size());
        for (size_t ii = 0; ii < src[si].size(); ii++) {
            CHECK_UNARY(src[si][ii].equal(src2[si][ii]));
        }
    }
}

/** @} */
