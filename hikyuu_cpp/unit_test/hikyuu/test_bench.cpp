/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240229 added by fasiondog
 */

#include "test_config.h"
#include <hikyuu/hikyuu.h>

#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_normal_benchmark") {
    auto& sm = StockManager::instance();

    Indicator test_ind = AMA();
    {
        SPEND_TIME(MA_benchmark);
        KQuery query(0, Null<int64_t>(), KQuery::MIN);
        Indicator result;
        for (auto& stk : sm) {
            auto k = stk.getKData(query);
            result = result + test_ind(k.close());
        }
    }

    {
        SPEND_TIME(MA_benchmark);
        KQuery query(0, Null<int64_t>(), KQuery::MIN);
        Indicator result;
        for (auto& stk : sm) {
            auto k = stk.getKData(query);
            result = result + test_ind(k.close());
        }
    }
}
#endif