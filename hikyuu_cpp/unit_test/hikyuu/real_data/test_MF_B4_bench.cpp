/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-10-15
 *      Author: fasiondog
 *
 *  B4 performance benchmark for the multifactor component (real market data).
 *  Uses cheap factors (CLOSE) so that the normalization / neutralization / index-building blocks
 *  dominate the calculate() cost and the per-item overheads become attributable. It prints median
 *  milliseconds per scenario so the same build can be compared before and after each optimization.
 */

#include "doctest/doctest.h"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <vector>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/trade_sys/multifactor/buildin_norm.h>
#include <hikyuu/trade_sys/multifactor/crt/MF_EqualWeight.h>
#include <hikyuu/trade_sys/multifactor/crt/MF_ICIRWeight.h>

using namespace hku;

namespace {

// Fixed stock pool for reproducible timing across rebuilds
StockList make_pool(StockManager& sm, size_t cap) {
    auto all = sm.getStockList([](const Stock& s) { return s.valid(); });
    std::sort(all.begin(), all.end(),
              [](const Stock& a, const Stock& b) { return a.market_code() < b.market_code(); });
    if (all.size() > cap) {
        all.resize(cap);
    }
    return all;
}

double median(std::vector<double> v) {
    std::sort(v.begin(), v.end());
    size_t n = v.size();
    return n % 2 ? v[n / 2] : (v[n / 2 - 1] + v[n / 2]) / 2.0;
}

// Warm up once (ignore: first pass loads the clickhouse / factor caches), then time reset+calculate
double bench(MultiFactorPtr mf, int runs = 5) {
    mf->reset();
    mf->calculate();  // warm up, discarded
    std::vector<double> ms;
    for (int i = 0; i < runs; i++) {
        mf->reset();
        auto t0 = std::chrono::steady_clock::now();
        mf->calculate();
        auto t1 = std::chrono::steady_clock::now();
        ms.push_back(std::chrono::duration<double, std::milli>(t1 - t0).count());
    }
    return median(ms);
}

}  // namespace

/**
 * @defgroup test_MF_B4_bench test_MF_B4_bench
 * @ingroup test_hikyuu_real_data
 * @{
 */

/** @par Benchmark scenarios for the B4 items (real data) */
TEST_CASE("test_MF_B4_bench") {
    StockManager& sm = StockManager::instance();
    sm.waitDataReady();  // wait for the background preload so getValues hits the local cache
    Stock ref_stk = sm.getMarketStock("SH");
    REQUIRE(!ref_stk.isNull());

    size_t cap = 800;
    size_t days = 2000;
    StockList stks = make_pool(sm, cap);
    REQUIRE_GT(stks.size(), 10);
    KQuery query = KQuery(-(int64_t)days);
    IndicatorList inds = {CLOSE(), CLOSE(), CLOSE()};

    // S1: bare (no norm / neutralize) -> getValues + composite + _buildIndex (MF-029 signal)
    auto mf_bare = MF_EqualWeight(inds, stks, query, ref_stk);
    double t_bare = bench(mf_bare);

    // S2: global normalization -> normalize block allocation + operator[] reads (MF-015/016)
    auto mf_norm = MF_EqualWeight(inds, stks, query, ref_stk);
    mf_norm->setNormalize(NORM_Zscore());
    double t_norm = bench(mf_norm);

    // S3: special norm + style neutralize -> per-(di,ii) clone + style_value_day + QR (MF-015/018)
    auto mf_style = MF_EqualWeight(inds, stks, query, ref_stk);
    mf_style->addSpecialNormalize("CLOSE", NORM_Zscore(), "", {CLOSE()});
    double t_style = bench(mf_style);

    // S4: ICIR -> dead getKData every calculate (MF-014)
    auto mf_icir = MF_ICIRWeight(inds, stks, query, ref_stk);
    double t_icir = bench(mf_icir);

    printf("[B4 bench] pool=%zu days=%zu\n", stks.size(), days);
    printf("[B4 bench] S1 bare          : %.1f ms\n", t_bare);
    printf("[B4 bench] S2 global norm   : %.1f ms (norm delta = %.1f)\n", t_norm, t_norm - t_bare);
    printf("[B4 bench] S3 special+style : %.1f ms (style delta = %.1f)\n", t_style,
           t_style - t_bare);
    printf("[B4 bench] S4 ICIR          : %.1f ms\n", t_icir);
    CHECK_UNARY(t_bare > 0.0);
}

/** @} */
