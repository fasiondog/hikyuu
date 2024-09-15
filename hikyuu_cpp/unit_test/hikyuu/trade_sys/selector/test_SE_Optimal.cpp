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
#include <hikyuu/trade_sys/selector/imp/OptimalSelector.h>
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

    /** @arg 尝试加入空系统 */
    CHECK_THROWS(se->addSystem(SYSPtr()));
    CHECK_UNARY(se->getProtoSystemList().empty());

    /** @arg 尝试加入含有空系统的系统列表 */
    auto sys = create_test_sys(2, 3);
    sys->setStock(getStock("sz000001"));
    se->addSystemList(SystemList{sys});
    CHECK_EQ(se->getProtoSystemList().size(), 1);
    CHECK_THROWS(se->addSystemList(SystemList{sys, SYSPtr()}));
    CHECK_EQ(se->getProtoSystemList().size(), 2);

    /** @arg 尝试加入未指定证券标的的系统 */
    sys = create_test_sys(2, 3);
    CHECK_THROWS(se->addSystem(sys));

    /** @arg 尝试加入未指定证券标的的系统列表 */
    sys = create_test_sys(2, 3);
    CHECK_THROWS(se->addSystemList({sys}));

    /** @arg 只有一个候选系统 */
    Stock stk = getStock("sz000001");
    sys = create_test_sys(2, 3);
    sys->setStock(stk);
    se->removeAll();
    se->addSystem(sys);
    REQUIRE(se->getProtoSystemList().size() == 1);
    KQuery query(-50);
    se->setParam<int>("train_len", 30);
    se->setParam<int>("test_len", 25);
    se->calculate(SystemList(), query);
    OptimalSelector* raw_se = dynamic_cast<OptimalSelector*>(se.get());
    auto run_ranges = raw_se->getRunRanges();
    HKU_INFO("total: {}", run_ranges.size());
    for (size_t i = 0, len = run_ranges.size(); i < len; i++) {
        HKU_INFO("{}: {} - {}", i, run_ranges[i].first, run_ranges[i].second);
        // HKU_INFO("{}: {}", i, run_ranges[i].first);
    }

#if 0
    vector<std::pair<int, int>> params{{3, 5}, {3, 10}, {5, 10}, {5, 20}};
    for (const auto& param : params) {
        auto sys = create_test_sys(param.first, param.second);
        sys->name(fmt::format("test_sys_{}_{}", param.first, param.second));
        sys->setStock(stk);
        se->addSystem(sys);
    }

    query = KQueryByIndex(-200);
    KData k = getKData("sh000001", query);
    HKU_INFO("sh000001 k len: {}, {} - {}", k.size(), k[0].datetime, k[k.size() - 1].datetime);
    k = getKData("sz000001", query);
    HKU_INFO("sz000001 k len: {}, {} - {}", k.size(), k[0].datetime, k[k.size() - 1].datetime);

    se->setParam<bool>("trace", true);
    se->setParam<int>("train_len", 100);
    se->setParam<int>("test_len", 20);
    se->calculate(SystemList(), query);
#endif
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_SE_Optimal_export") {}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */