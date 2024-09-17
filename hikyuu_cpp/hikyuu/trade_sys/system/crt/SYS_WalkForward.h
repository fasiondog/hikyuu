/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240916 added by fasiondog
 */

#pragma once

#include "../System.h"

namespace hku {

SystemPtr HKU_API SYS_WalkForward(const SystemList& candidate_sys_list,
                                  const TradeManagerPtr& tm = TradeManagerPtr(),
                                  size_t train_len = 100, size_t test_len = 20,
                                  const TradeManagerPtr& train_tm = TradeManagerPtr());

}