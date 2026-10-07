/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-17
 *      Author: fasiondog
 *
 *  Some miscellaneous functions
 */

#pragma once

#include "trade_sys/system/System.h"

#include "hikyuu/trade_manage/Performance.h"

namespace hku {

/**
 * @brief Run the systems in parallel
 * @param system_list system list
 * @param query query condition for running the systems
 * @param reset whether to reset according to the shared attribute of the system parts before
 *              execution
 * @param resetAll force resetting all the parts
 * @return the funds list of every time point within the Query period of each system
 */
vector<FundsList> HKU_API parallel_run_sys(const SystemList& system_list, const KQuery& query,
                                           bool reset = true, bool resetAll = false);


/**
 * Get the funds list of the account list at once
 * @param tm_list
 * @param ref_dates
 * @return vector<FundsList>
 * @ingroup TradeManagerClass
 */
vector<FundsList> HKU_API getFundsList(const vector<TMPtr>& tm_list, const DatetimeList& ref_dates);

/**
 * Get the funds list of the account list at once
 * @param tm_list
 * @param datetime deadline date
 * @param ktype K-line type
 * @param ext whether to get the extended statistics (donation users, otherwise the basic
 *            statistics are still returned)
 * @return vector<FundsList>
 * @ingroup TradeManagerClass
 */
vector<Performance> HKU_API getPerformanceList(const vector<TMPtr>& tm_list,
                                               const Datetime& datetime = Datetime::now(),
                                               const KQuery::KType& ktype = KQuery::DAY,
                                               bool ext = true);

}  // namespace hku