/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-25
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/trade_sys/system/System.h"
#include "hikyuu/trade_manage/Performance.h"
#include "hikyuu/utilities/thread/thread.h"

namespace hku {

struct HKU_API AnalysisSystemOutput {
    string market_code;  ///< 证券代码
    string name;         ///< 证券名称
    PriceList values;    ///< 统计各项指标值

    AnalysisSystemOutput() = default;
    AnalysisSystemOutput(const AnalysisSystemOutput& other) = default;
    AnalysisSystemOutput(AnalysisSystemOutput&& rv)
    : market_code(std::move(rv.market_code)),
      name(std::move(rv.name)),
      values(std::move(rv.values)) {}

    AnalysisSystemOutput& operator=(const AnalysisSystemOutput&) = default;
    AnalysisSystemOutput& operator=(AnalysisSystemOutput&& rv) {
        HKU_IF_RETURN(this == &rv, *this);
        market_code = std::move(rv.market_code);
        name = std::move(rv.name);
        values = std::move(rv.values);
        return *this;
    }
};

/**
 * 统计所有指定系统策略, 系统列表中的每一个系统都应该是独立的系统实例
 * @note 系统实例列表和证券列表是一一对应，不是两两组合
 * @param sys_list 系统策略实例列表
 * @param stk_list 证券列表，须与系统策略列表一一对应
 * @param query 查询条件
 * @return vector<AnalysisSystemOutput>
 */
vector<AnalysisSystemOutput> HKU_API analysisSystemList(const SystemList& sys_list,
                                                        const StockList& stk_list,
                                                        const KQuery& query);

/**
 * 以指定的 stock 统计执行所有的系统策略
 * @param sys_list 系统策略思路列表，每一个都应是单独的实例
 * @param stk 指定证券
 * @param query 查询条件
 * @return vector<AnalysisSystemOutput>
 */
vector<AnalysisSystemOutput> HKU_API analysisSystemList(const SystemList& sys_list,
                                                        const Stock& stk, const KQuery& query);

/**
 * 以指定的 stock 统计执行所有的系统策略，并按指定的统计项返回统计项最高或最低的系统实例及其统计值
 * @param sys_list 系统策略思路列表，每一个都应是单独的实例
 * @param stk 指定证券
 * @param query 查询条件
 * @param sort_key Performance 中的统计项名称，按该统计项返回
 * @param sort_mode 0 为取统计最高值的系统，其他值为取统计最低值的系统
 * @return std::pair<double, SYSPtr>
 */
std::pair<double, SYSPtr> HKU_API findOptimalSystem(const SystemList& sys_list, const Stock& stk,
                                                    const KQuery& query, const string& sort_key,
                                                    int sort_mode = 0);

std::pair<double, SYSPtr> HKU_API findOptimalSystemMulti(const SystemList& sys_list,
                                                         const Stock& stk, const KQuery& query,
                                                         const string& sort_key, int sort_mode = 0);
}  // namespace hku
