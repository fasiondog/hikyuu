/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-15
 *      Author: fasiondog
 */

#include "../../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/AMA.h>
#include <hikyuu/indicator/crt/EMA.h>
#include <hikyuu/indicator/crt/IC.h>
#include <hikyuu/indicator/crt/ROCR.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/trade_sys/factor/crt/MF_ICWeight.h>

using namespace hku;

/**
 * @defgroup test_MF_ICWeight test_MF_ICWeight
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

TEST_CASE("test_MF_ICWeight") {}

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