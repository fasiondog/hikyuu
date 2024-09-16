/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240916 added by fasiondog
 */

#pragma once

#include "../System.h"

namespace hku {

SystemPtr HKU_API SYS_WalkForward(const TradeManagerPtr& tm, const SystemList& candidate_sys_list,
                                  const TradeManagerPtr& train_tm = TradeManagerPtr());

}