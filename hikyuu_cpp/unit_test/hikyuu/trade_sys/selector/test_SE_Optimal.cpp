/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-15
 *      Author: fasiondog
 */

#include "../../test_config.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include <hikyuu/trade_sys/selector/crt/SE_Optimal.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/NOT.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_sys/signal/crt/SG_Bool.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_Nothing.h>

using namespace hku;

/**
 * @defgroup test_Selector_optimal test_Selector_optimal
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

static SYSPtr create_test_sys(int fast_n, int slow_n) {
    auto ind = MA(CLOSE(), fast_n) > MA(CLOSE(), slow_n);
    auto sg = SG_Bool(ind, NOT(ind));
    auto tm = crtTM();
    return SYS_Simple(tm, MM_Nothing(), EnvironmentPtr(), ConditionPtr(), sg);
}

/** @par 检测点 */
TEST_CASE("test_SE_Optimal") {
    auto se = SE_Optimal();
    CHECK_EQ(se->name(), "SE_Optimal");

    Stock stk = getStock("sz000001");
    vector<std::pair<int, int>> params{{3, 5}, {3, 10}, {5, 10}, {5, 20}};
    for (const auto& param : params) {
        auto sys = create_test_sys(param.first, param.second);
        sys->name(fmt::format("test_sys_{}_{}", param.first, param.second));
        sys->setStock(stk);
        se->addSystem(sys);
    }

    KQuery query(-200);
    KData k = getKData("sh000001", query);
    HKU_INFO("sh000001 k len: {}, {} - {}", k.size(), k[0].datetime, k[k.size() - 1].datetime);
    k = getKData("sz000001", query);
    HKU_INFO("sz000001 k len: {}, {} - {}", k.size(), k[0].datetime, k[k.size() - 1].datetime);

    se->setParam<bool>("trace", true);
    se->setParam<int>("train_len", 100);
    se->setParam<int>("test_len", 20);
    se->calculate(SystemList(), query);

    // my_sg = SG_Bool(ind, NOT(ind))
    // ret = SYS_Simple(tm=tm, sg=my_sg, mm=mm, sp=sp)
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_SE_Optimal_export") {}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */