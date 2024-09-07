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

vector<AnalysisSystemOutput> HKU_API analysisSystemList(const SystemList& sys_list,
                                                        const StockList& stk_list,
                                                        const KQuery& query);

vector<AnalysisSystemOutput> HKU_API analysisSystemList(const StockList& stk_list,
                                                        const KQuery& query,
                                                        const SystemPtr& pro_sys);

vector<AnalysisSystemOutput> HKU_API analysisSystemList(const SystemList& sys_list,
                                                        const Stock& stk, const KQuery& query);

std::pair<double, SYSPtr> findOptimalSystem(const SystemList& sys_list, const Stock& stk,
                                            const KQuery& query, const string& sort_key);

}  // namespace hku
