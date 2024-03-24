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
#include <hikyuu/indicator/crt/ICIR.h>
#include <hikyuu/indicator/crt/ROCR.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/STDEV.h>
#include <hikyuu/trade_sys/factor/crt/MF_ICIRWeight.h>

using namespace hku;

/**
 * @defgroup test_MF_ICIRWeight test_MF_ICIRWeight
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

TEST_CASE("test_MF_ICIRWeight") {
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

    /** @arg 输入非法 ic_n, ic_rolling_n */
    CHECK_THROWS_AS(MF_ICIRWeight(src_inds, stks, query, ref_stk, 0), std::exception);
    CHECK_THROWS_AS(MF_ICIRWeight(src_inds, stks, query, ref_stk, 1, -1), std::exception);

    /** @arg 正常计算 */
    auto mf = MF_ICIRWeight(src_inds, stks, query, ref_stk, ndays, ic_rolling_n);
    CHECK_EQ(mf->name(), "MF_ICIRWeight");
    CHECK_THROWS_AS(mf->getFactor(sm["sh600000"]), std::exception);
    CHECK_THROWS_AS(mf->setParam<int>("ic_n", -1), std::exception);
    mf->setParam<int>("ic_n", ndays);
    CHECK_THROWS_AS(mf->setParam<int>("ic_rolling_n", -1), std::exception);
    mf->setParam<int>("ic_rolling_n", ic_rolling_n);

    auto stk = sm["sh600004"];
    auto ind1 = MA(ROCR(CLOSE(stk.getKData(query)), ndays));
    auto ic1 = ICIR(MA(ROCR(CLOSE(), ndays)), stks, query, ref_stk, ndays, ic_rolling_n);
    auto ind2 = AMA(ROCR(CLOSE(stk.getKData(query)), ndays));
    auto ic2 = ICIR(AMA(ROCR(CLOSE(), ndays)), stks, query, ref_stk, ndays, ic_rolling_n);
    auto ind3 = EMA(ROCR(CLOSE(stk.getKData(query)), ndays));
    auto ic3 = ICIR(EMA(ROCR(CLOSE(), ndays)), stks, query, ref_stk, ndays, ic_rolling_n);

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
        // HKU_INFO("{}: {}, {}, {}, {}, {}", i, w, ind4[i], ind1[i], ma_ic1[i], stdev_ic1[i]);
        // HKU_INFO("{}: {}, {}", i, w, ind4[i]);
        if (!std::isnan(ind4[i]) && !std::isnan(w)) {
            CHECK_EQ(ind4[i], doctest::Approx(w));
        }
    }
    // HKU_INFO("{}", ind4);
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_MF_ICIRWeight_benchmark") {
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
        BENCHMARK_TIME_MSG(test_MF_ICIRWeight_benchmark, cycle,
                           fmt::format("data len: {}", ref_k.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            auto mf = MF_ICIRWeight(src_inds, stks, query, ref_stk);
            auto ic = mf->getIC();
        }
    }
}
#endif

/** @} */