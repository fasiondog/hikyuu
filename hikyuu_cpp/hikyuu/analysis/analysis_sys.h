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
    string market_code;  ///< Security code
    string name;         ///< Security name
    PriceList values;    ///< The values of every statistics item

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
 * Run the statistics of all the given system strategies; every system in the system list should be
 * an independent system instance
 * @note The system instance list and the security list correspond one to one, they are not combined
 *       pairwise
 * @param sys_list system strategy instance list
 * @param stk_list security list, it must correspond one to one with the system strategy list
 * @param query query condition
 * @return vector<AnalysisSystemOutput>
 */
vector<AnalysisSystemOutput> HKU_API analysisSystemList(const SystemList& sys_list,
                                                        const StockList& stk_list,
                                                        const KQuery& query);

/**
 * Run all the system strategies on the given stock and collect the statistics
 * @param sys_list system strategy list, every one of them should be a separate instance
 * @param stk the given security
 * @param query query condition
 * @return vector<AnalysisSystemOutput>
 */
vector<AnalysisSystemOutput> HKU_API analysisSystemList(const SystemList& sys_list,
                                                        const Stock& stk, const KQuery& query);

/**
 * Run all the system strategies on the given stock and collect the statistics, and return the
 * system instance with the highest or lowest value of the given statistics item together with its
 * value
 * @param sys_list system strategy list, every one of them should be a separate instance
 * @param stk the given security
 * @param query query condition
 * @param sort_key the name of the statistics item in Performance, returned by that statistics item
 * @param sort_mode 0 returns the system with the highest statistics value, any other value returns
 *                  the system with the lowest statistics value
 * @return std::pair<double, SYSPtr>
 */
std::pair<double, SYSPtr> HKU_API findOptimalSystem(const SystemList& sys_list, const Stock& stk,
                                                    const KQuery& query, const string& sort_key,
                                                    int sort_mode = 0);

std::pair<double, SYSPtr> HKU_API findOptimalSystemMulti(const SystemList& sys_list,
                                                         const Stock& stk, const KQuery& query,
                                                         const string& sort_key, int sort_mode = 0);
}  // namespace hku
