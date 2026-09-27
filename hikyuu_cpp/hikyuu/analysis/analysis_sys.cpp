/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-25
 *      Author: fasiondog
 */

#include "hikyuu/Block.h"
#include "analysis_sys.h"

namespace hku {

DatetimeList getAnalysisCalendar(const StockList& stk_list, const KQuery& query) {
    auto& sm = StockManager::instance();

    // A positive absolute index is ambiguous here: sys->run() interprets it in each security's own
    // K-line space, while the market calendar (backed by the market index stock) would interpret it
    // in the index stock's space, sending the statistics stop date years into the past.
    if (query.queryType() != KQuery::INDEX || query.start() < 0) {
        return sm.getTradingCalendar(query);
    }

    Datetime start_date;
    Datetime end_date;
    bool has = false;
    for (const auto& stk : stk_list) {
        if (stk.isNull()) {
            continue;
        }
        DatetimeList dates = stk.getDatetimeList(query);
        if (dates.empty()) {
            continue;
        }
        if (!has || dates.front() < start_date) {
            start_date = dates.front();
        }
        if (!has || dates.back() > end_date) {
            end_date = dates.back();
        }
        has = true;
    }

    if (!has) {
        return sm.getTradingCalendar(query);
    }

    // The date query is right-open; push the end one minute forward so end_date stays included for
    // both day and intraday ktypes.
    KQuery date_query =
      KQueryByDate(start_date, end_date + Minutes(1), query.kType(), query.recoverType());
    return sm.getTradingCalendar(date_query);
}

vector<AnalysisSystemOutput> HKU_API analysisSystemList(const SystemList& sys_list,
                                                        const StockList& stk_list,
                                                        const KQuery& query) {
    HKU_ASSERT(sys_list.size() == stk_list.size());

    vector<AnalysisSystemOutput> result;
    size_t total = sys_list.size();
    HKU_IF_RETURN(0 == total, result);

    auto date_list = getAnalysisCalendar(stk_list, query);
    HKU_IF_RETURN(date_list.empty(), result);
    Datetime last_datetime = date_list.back();

    result = global_parallel_for_index(0, total, [&, last_datetime](size_t i) {
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

    auto date_list = getAnalysisCalendar(StockList{stk}, query);
    HKU_IF_RETURN(date_list.empty(), result);
    Datetime last_datetime = date_list.back();

    result = global_parallel_for_index(0, total, [&, stk, last_datetime](size_t i) {
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
      sort_mode == 0 ? std::numeric_limits<double>::lowest() : std::numeric_limits<double>::max();
    std::pair<double, SYSPtr> result{init_val, SYSPtr()};
    size_t total = sys_list.size();
    HKU_IF_RETURN(0 == total, result);

    HKU_ERROR_IF_RETURN(stk.isNull(), result, "stock is null!");

    // Guarantee that the statistics only go to the last date given by query rather than to now by
    // default, otherwise the return of a system still holding a position would be inappropriate
    auto date_list = getAnalysisCalendar(StockList{stk}, query);
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
      sort_mode == 0 ? std::numeric_limits<double>::lowest() : std::numeric_limits<double>::max();
    std::pair<double, SYSPtr> result{init_val, SYSPtr()};
    size_t total = sys_list.size();
    HKU_IF_RETURN(0 == total, result);

    HKU_ERROR_IF_RETURN(stk.isNull(), result, "stock is null!");

    // Guarantee that the statistics only go to the last date given by query rather than to now by
    // default, otherwise the return of a system still holding a position would be inappropriate
    auto date_list = getAnalysisCalendar(StockList{stk}, query);
    HKU_IF_RETURN(date_list.empty(), result);
    Datetime last_datetime = date_list.back();

    auto all_result =
      global_parallel_for_index(0, total, [&, stk, last_datetime, init_val](size_t i) {
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
        for (const auto& v : all_result) {
            if (v.first > result.first) {
                result.first = v.first;
                result.second = v.second;
            }
        }
    } else {
        for (const auto& v : all_result) {
            if (v.first < result.first) {
                result.first = v.first;
                result.second = v.second;
            }
        }
    }

    return result;
}

}  // namespace hku
