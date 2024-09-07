/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-25
 *      Author: fasiondog
 */

#include "hikyuu/Block.h"
#include "analysis_sys.h"

namespace hku {

vector<AnalysisSystemOutput> HKU_API analysisSystemList(const SystemList& sys_list,
                                                        const StockList& stk_list,
                                                        const KQuery& query) {
    HKU_ASSERT(sys_list.size() == stk_list.size());

    vector<AnalysisSystemOutput> result;
    size_t total = sys_list.size();
    HKU_IF_RETURN(0 == total, result);

    result = parallel_for_index(0, total, [&](size_t i) {
        const auto& sys = sys_list[i];
        const auto& stk = stk_list[i];

        AnalysisSystemOutput ret;
        if (!sys || stk.isNull()) {
            return ret;
        }

        try {
            sys->run(stk, query);
            Performance per;
            per.statistics(sys->getTM());
            ret.market_code = stk.market_code();
            ret.name = stk.name();
            ret.values = per.values();
        } catch (const std::exception& e) {
            HKU_ERROR(e.what());
        } catch (...) {
            HKU_ERROR("Unknown error!");
        }
        return ret;
    });

    return result;
}

vector<AnalysisSystemOutput> HKU_API analysisSystemList(const StockList& stk_list,
                                                        const KQuery& query,
                                                        const SystemPtr& pro_sys) {
    HKU_CHECK(pro_sys, "pro_sys is null!");

    return parallel_for_range(0, stk_list.size(), [=](const range_t& range) {
        vector<AnalysisSystemOutput> ret;
        auto sys = pro_sys->clone();
        Performance per;
        AnalysisSystemOutput out;
        for (size_t i = range.first; i < range.second; i++) {
            try {
                auto stk = stk_list[i];
                sys->run(stk, query);
                per.statistics(sys->getTM());
                out.market_code = stk.market_code();
                out.name = stk.name();
                out.values = per.values();
                ret.emplace_back(std::move(out));
            } catch (const std::exception& e) {
                HKU_ERROR(e.what());
            }
        }
        return ret;
    });
}

vector<AnalysisSystemOutput> HKU_API analysisSystemList(const SystemList& sys_list,
                                                        const Stock& stk, const KQuery& query) {
    vector<AnalysisSystemOutput> result;
    size_t total = sys_list.size();
    HKU_IF_RETURN(0 == total, result);

    auto date_list = StockManager::instance().getTradingCalendar(query);
    HKU_IF_RETURN(!date_list.empty(), result);
    Datetime last_datetime = date_list.back();

    result = parallel_for_index(0, total, [&, stk, last_datetime](size_t i) {
        const auto& sys = sys_list[i];
        AnalysisSystemOutput ret;
        if (!sys || stk.isNull()) {
            return ret;
        }

        try {
            sys->run(stk, query);
            Performance per;
            per.statistics(sys->getTM(), last_datetime);
            ret.market_code = stk.market_code();
            ret.name = sys->name();
            ret.values = per.values();
        } catch (const std::exception& e) {
            HKU_ERROR(e.what());
        } catch (...) {
            HKU_ERROR("Unknown error!");
        }
        return ret;
    });

    return result;
}

std::pair<double, SYSPtr> findOptimalSystem(const SystemList& sys_list, const Stock& stk,
                                            const KQuery& query, const string& sort_key) {
    std::pair<double, SYSPtr> result{std::numeric_limits<double>::min(), SYSPtr()};
    size_t total = sys_list.size();
    HKU_IF_RETURN(0 == total, result);

    HKU_ERROR_IF_RETURN(stk.isNull(), result, "stock is null!");

    string statistic_key = sort_key.empty() ? "帐户平均年收益率%" : sort_key;
    HKU_ERROR_IF_RETURN(!Performance::exist(statistic_key), result,
                        "Invalid sort key: {}! A statistical item does not exist!", statistic_key);

    // 保证只统计到 query 指定的最后日期，而不是默认到现在，否则仍有持仓的系统收益不合适
    auto date_list = StockManager::instance().getTradingCalendar(query);
    HKU_IF_RETURN(!date_list.empty(), result);
    Datetime last_datetime = date_list.back();

    auto all_result = parallel_for_index(0, total, [&, stk, last_datetime](size_t i) {
        const auto& sys = sys_list[i];
        std::pair<double, SYSPtr> ret{std::numeric_limits<double>::min(), sys};

        HKU_ERROR_IF_RETURN(!sys, ret, "sys_list[{}] is null!", i);

        try {
            sys->run(stk, query);
            Performance per;
            per.statistics(sys->getTM(), last_datetime);
            ret = std::make_pair(per.get(sort_key), sys);

        } catch (const std::exception& e) {
            HKU_ERROR("sys_list[{}] run failed! {}", i, e.what());
        } catch (...) {
            HKU_ERROR("sys_list[{}] run failed! Unknown error!", i);
        }
        return ret;
    });

    if (!all_result.empty()) {
        std::sort(all_result.begin(), all_result.end(),
                  [](const std::pair<double, SYSPtr>& a, const std::pair<double, SYSPtr>& b) {
                      return a.first > b.first;
                  });
        result = all_result.front();
    }

    return result;
}

}  // namespace hku
