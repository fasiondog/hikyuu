/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-15
 *      Author: fasiondog
 */

#include "../../test_config.h"
#include <fstream>
#include <cmath>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/AMA.h>
#include <hikyuu/indicator/crt/EMA.h>
#include <hikyuu/indicator/crt/IC.h>
#include <hikyuu/indicator/crt/ROCR.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/trade_sys/multifactor/crt/MF_ICWeight.h>

using namespace hku;

/**
 * @defgroup test_MF_ICWeight test_MF_ICWeight
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

TEST_CASE("test_MF_ICWeight") {
    StockManager& sm = StockManager::instance();
    int ndays = 3;
    int ic_rolling_n = 3;
    Stock ref_stk = sm["sh000001"];
    IndicatorList src_inds = {MA(ROCR(CLOSE(), ndays)), AMA(ROCR(CLOSE(), ndays)),
                              EMA(ROCR(CLOSE(), ndays))};
    StockList stks = {sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    KQuery query = KQuery(-20);
    KData ref_k = ref_stk.getKData(query);
    DatetimeList ref_dates = ref_k.getDatetimeList();
    auto mf = MF_ICWeight(src_inds, stks, query, ref_stk, ndays, ic_rolling_n);
    CHECK_EQ(mf->name(), "MF_ICWeight");
    CHECK_THROWS_AS(mf->getFactor(sm["sh600000"]), std::exception);

    auto stk = sm["sh600004"];
    auto ind1 = MA(ROCR(CLOSE(stk.getKData(query)), ndays));
    auto ic1 = MA(IC(MA(ROCR(CLOSE(), ndays)), stks, query, ref_stk, ndays), ic_rolling_n);
    auto ind2 = AMA(ROCR(CLOSE(stk.getKData(query)), ndays));
    auto ic2 = MA(IC(AMA(ROCR(CLOSE(), ndays)), stks, query, ref_stk, ndays), ic_rolling_n);
    auto ind3 = EMA(ROCR(CLOSE(stk.getKData(query)), ndays));
    auto ic3 = MA(IC(EMA(ROCR(CLOSE(), ndays)), stks, query, ref_stk, ndays), ic_rolling_n);

    auto ind4 = mf->getFactor(stk);
    for (size_t i = 0; i < ind4.discard(); i++) {
        CHECK_UNARY(std::isnan(ind4[i]));
    }
    CHECK_EQ(ind4.discard(), std::max(ic1.discard(), std::max(ic2.discard(), ic3.discard())));
    for (size_t i = 0; i < ind4.discard(); i++) {
        CHECK_UNARY(std::isnan(ind4[i]));
    }
    for (size_t i = ind4.discard(), len = ref_dates.size(); i < len; i++) {
        Indicator::value_t w = (ind1[i] * ic1[i] + ind2[i] * ic2[i] + ind3[i] * ic3[i]) /
                               (std::abs(ic1[i]) + std::abs(ic2[i]) + std::abs(ic3[i]));
        // HKU_INFO("{}: {}, {}", i, w, ind4[i]);
        CHECK_EQ(ind4[i], doctest::Approx(w));
    }
    // HKU_INFO("{}", ind4);
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_MF_ICWeight_benchmark") {
    StockManager& sm = StockManager::instance();
    int ndays = 3;
    IndicatorList src_inds = {MA(ROCR(CLOSE(), ndays)), AMA(ROCR(CLOSE(), ndays)),
                              EMA(ROCR(CLOSE(), ndays))};
    StockList stks = {sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    KQuery query = KQuery(0);
    Stock ref_stk = sm["sh000001"];
    auto ref_k = ref_stk.getKData(query);
    auto ref_dates = ref_k.getDatetimeList();

    int cycle = 10;  // 测试循环次数

    {
        BENCHMARK_TIME_MSG(test_MF_ICWeight_benchmark, cycle,
                           fmt::format("data len: {}", ref_k.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            auto mf = MF_ICWeight(src_inds, stks, query, ref_stk);
            auto ic = mf->getIC();
        }
    }
}
#endif

/** @} */