/*
 * build_in.h
 *
 *  Created on: 2013-4-19
 *      Author: fasiondog
 */

#pragma once
#ifndef MONEYMANAGER_BUILD_IN_H_
#define MONEYMANAGER_BUILD_IN_H_

#include "crt/MM_Nothing.h"
#include "crt/MM_FixedPercent.h"
#include "crt/MM_FixedRisk.h"
#include "crt/MM_FixedCapital.h"
#include "crt/MM_FixedCapitalFunds.h"
#include "crt/MM_FixedCount.h"
#include "crt/MM_FixedCountTps.h"
#include "crt/MM_FixedUnits.h"
#include "crt/MM_WilliamsFixedRisk.h"
// v5：组合级资金分配（L1/L2/L3）已迁移至 AllocateFundsBase（AF），MM 仅保留单系统形态，
//      见 docs/design/pf_af_compat/design.md §5.2

#endif /* MONEYMANAGER_BUILD_IN_H_ */
