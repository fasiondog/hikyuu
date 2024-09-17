/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-16
 *      Author: fasiondog
 */

#include "../../test_config.h"
#include <hikyuu/trade_sys/system/crt/SYS_WalkForward.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/NOT.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_sys/signal/crt/SG_Bool.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_Nothing.h>

using namespace hku;

/**
 * @defgroup test_SYS_WalkForward test_SYS_WalkForward
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

static SYSPtr create_test_sys(int fast_n, int slow_n) {
    auto ind = MA(CLOSE(), fast_n) > MA(CLOSE(), slow_n);
    auto sg = SG_Bool(ind, NOT(ind));
    auto tm = crtTM();
    auto sys = SYS_Simple(tm, MM_Nothing(), EnvironmentPtr(), ConditionPtr(), sg);
    sys->name(fmt::format("test_sys_{}_{}", fast_n, slow_n));
    return sys;
}

/** @par 检测点 */
TEST_CASE("test_SYS_WalkForword") {
    Stock stk = getStock("sz000001");
    KQuery query = KQueryByIndex(-50);
    TMPtr tm = crtTM();

    /** @arg 候选系统列表为空 */
    CHECK_THROWS(SYS_WalkForward(SystemList(), tm));

    /** @arg 运行时未指定tm */
    SystemList candidate_sys_list{create_test_sys(3, 5)};
    auto sys = SYS_WalkForward(candidate_sys_list);
    CHECK_THROWS(sys->run(stk, query));

    /** @arg 执行时未指定证券标的 */
    CHECK_THROWS(sys->run(query));

    /** @arg 只有一个候选系统 */
    sys = SYS_WalkForward(SystemList{create_test_sys(3, 5)}, tm);
    query = KQueryByIndex(-500);
    sys->run(stk, query);
    // HKU_INFO("{}", sys->str());
    // std::cout << sys << std::endl;
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_SYS_WalkForword_export") {}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
