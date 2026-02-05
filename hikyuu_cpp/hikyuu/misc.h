/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-17
 *      Author: fasiondog
 *
 *  部分杂类函数
 */

#pragma once

#include "trade_sys/system/System.h"
#include "trade_sys/portfolio/Portfolio.h"
#include "hikyuu/trade_manage/Performance.h"

namespace hku {

/**
 * @brief 并行运行系统
 * @param system_list 系统列表
 * @param query 系统运行查询条件
 * @param reset 执行前是否依据系统部件共享属性复位
 * @param resetAll 强制复位所有部件
 * @return 各系统Query周期内各时间点的资金列表
 */
vector<FundsList> HKU_API parallel_run_sys(const SystemList& system_list, const KQuery& query,
                                           bool reset = true, bool resetAll = false);

/**
 * @brief 并行运行组合
 * @param pf_list 组合列表
 * @param query 组合运行查询条件
 * @param force 强制复位
 * @return vector<FundsList>
 */
vector<FundsList> HKU_API parallel_run_pf(const vector<PFPtr>& pf_list, const KQuery& query,
                                          bool force = false);

/**
 * 从账户列表中一次性获取资金列表
 * @param tm_list
 * @param ref_dates
 * @return vector<FundsList>
 * @ingroup TradeManagerClass
 */
vector<FundsList> HKU_API getFundsList(const vector<TMPtr>& tm_list, const DatetimeList& ref_dates);

/**
 * 从账户列表中一次性获取资金列表
 * @param tm_list
 * @param ref_dates
 * @return vector<FundsList>
 * @ingroup TradeManagerClass
 */
vector<Performance> HKU_API getPerformanceList(const vector<TMPtr>& tm_list,
                                               const Datetime& datetime = Datetime::now(),
                                               const KQuery::KType& ktype = KQuery::DAY);

}  // namespace hku