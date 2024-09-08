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

    auto date_list = StockManager::instance().getTradingCalendar(query);
    HKU_IF_RETURN(date_list.empty(), result);
    Datetime last_datetime = date_list.back();

    result = parallel_for_index(0, total, [&, last_datetime](size_t i) {
        const auto& sys = sys_list[i];
        const auto& stk = stk_list[i];

        AnalysisSystemOutput ret;
        if (!sys || stk.isNull()) {
            return ret;
        }

        try {
            sys->run(stk, query);
            Performance per;
            per.statistics(sys->getTM(), last_datetime);
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

vector<AnalysisSystemOutput> HKU_API analysisSystemList(const SystemList& sys_list,
                                                        const Stock& stk, const KQuery& query) {
    vector<AnalysisSystemOutput> result;
    size_t total = sys_list.size();
    HKU_IF_RETURN(0 == total, result);

    auto date_list = StockManager::instance().getTradingCalendar(query);
    HKU_IF_RETURN(date_list.empty(), result);
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

std::pair<double, SYSPtr> HKU_API findOptimalSystem(const SystemList& sys_list, const Stock& stk,
                                                    const KQuery& query, const string& sort_key,
                                                    int sort_mode) {
    SPEND_TIME(findOptimalSystem);
    double init_val =
      sort_mode == 0 ? std::numeric_limits<double>::min() : std::numeric_limits<double>::max();
    std::pair<double, SYSPtr> result{init_val, SYSPtr()};
    size_t total = sys_list.size();
    HKU_IF_RETURN(0 == total, result);

    HKU_ERROR_IF_RETURN(stk.isNull(), result, "stock is null!");

    string statistic_key = sort_key.empty() ? "帐户平均年收益率%" : sort_key;
    HKU_ERROR_IF_RETURN(!Performance::exist(statistic_key), result,
                        "Invalid sort key: {}! A statistical item does not exist!", statistic_key);

    // 保证只统计到 query 指定的最后日期，而不是默认到现在，否则仍有持仓的系统收益不合适
    auto date_list = StockManager::instance().getTradingCalendar(query);
    HKU_IF_RETURN(date_list.empty(), result);
    Datetime last_datetime = date_list.back();

    for (size_t i = 0; i < total; i++) {
        const auto& sys = sys_list[i];
        if (!sys) {
            continue;
        }

        try {
            sys->run(stk, query);
            Performance per;
            per.statistics(sys->getTM(), last_datetime);
            double val = per.get(sort_key);
            if (sort_mode == 0 && val > result.first) {
                result.first = val;
                result.second = sys;
            } else if (sort_mode != 0 && val < result.first) {
                result.first = val;
                result.second = sys;
            }

        } catch (const std::exception& e) {
            HKU_ERROR("sys_list[{}] run failed! {}", i, e.what());
        } catch (...) {
            HKU_ERROR("sys_list[{}] run failed! Unknown error!", i);
        }
    }

    return result;
}

std::pair<double, SYSPtr> HKU_API findOptimalSystemMulti(const SystemList& sys_list,
                                                         const Stock& stk, const KQuery& query,
                                                         const string& sort_key, int sort_mode) {
    SPEND_TIME(findOptimalSystemMulti);
    double init_val =
      sort_mode == 0 ? std::numeric_limits<double>::min() : std::numeric_limits<double>::max();
    std::pair<double, SYSPtr> result{init_val, SYSPtr()};
    size_t total = sys_list.size();
    HKU_IF_RETURN(0 == total, result);

    HKU_ERROR_IF_RETURN(stk.isNull(), result, "stock is null!");

    string statistic_key = sort_key.empty() ? "帐户平均年收益率%" : sort_key;
    HKU_ERROR_IF_RETURN(!Performance::exist(statistic_key), result,
                        "Invalid sort key: {}! A statistical item does not exist!", statistic_key);

    // 保证只统计到 query 指定的最后日期，而不是默认到现在，否则仍有持仓的系统收益不合适
    auto date_list = StockManager::instance().getTradingCalendar(query);
    HKU_IF_RETURN(date_list.empty(), result);
    Datetime last_datetime = date_list.back();

    auto all_result = parallel_for_index(0, total, [&, stk, last_datetime, init_val](size_t i) {
        const auto& sys = sys_list[i];
        std::pair<double, SYSPtr> ret{init_val, sys};

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

    if (0 == sort_mode) {
        for (auto& v : all_result) {
            if (v.first > result.first) {
                result.first = v.first;
                result.second = v.second;
            }
        }
    } else {
        for (auto& v : all_result) {
            if (v.first < result.first) {
                result.first = v.first;
                result.second = v.second;
            }
        }
    }

    return result;
}

}  // namespace hku
