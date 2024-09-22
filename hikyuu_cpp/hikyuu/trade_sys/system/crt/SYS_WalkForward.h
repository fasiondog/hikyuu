/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240916 added by fasiondog
 */

#pragma once

#include "hikyuu/trade_sys/system/System.h"
#include "hikyuu/trade_sys/selector/crt/SE_Optimal.h"

namespace hku {

// SystemPtr HKU_API SYS_WalkForward(const SystemList& candidate_sys_list,
//                                   const TradeManagerPtr& tm = TradeManagerPtr(),
//                                   size_t train_len = 100, size_t test_len = 20,
//                                   const string& key = "帐户平均年收益率%",
//                                   const TradeManagerPtr& train_tm = TradeManagerPtr());

SystemPtr HKU_API SYS_WalkForward(const SystemList& candidate_sys_list,
                                  const TradeManagerPtr& tm = TradeManagerPtr(),
                                  size_t train_len = 100, size_t test_len = 20,
                                  const SelectorPtr& se = SE_MaxFundsOptimal(),
                                  const TradeManagerPtr& train_tm = TradeManagerPtr());

}  // namespace hku