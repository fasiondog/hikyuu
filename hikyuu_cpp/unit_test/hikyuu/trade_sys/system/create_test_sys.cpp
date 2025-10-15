/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-16
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include "hikyuu/indicator/crt/MA.h"
#include "hikyuu/indicator/crt/KDATA.h"
#include "hikyuu/indicator/crt/NOT.h"
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_sys/signal/crt/SG_Bool.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_Nothing.h>
#include "create_test_sys.h"

SYSPtr create_test_sys(int fast_n, int slow_n) {
    auto ind = MA(CLOSE(), fast_n) > MA(CLOSE(), slow_n);
    auto sg = SG_Bool(ind, NOT(ind));
    auto tm = crtTM();
    auto sys = SYS_Simple(tm, MM_Nothing(), EnvironmentPtr(), ConditionPtr(), sg);
    sys->name(fmt::format("test_sys_{}_{}", fast_n, slow_n));
    return sys;
}