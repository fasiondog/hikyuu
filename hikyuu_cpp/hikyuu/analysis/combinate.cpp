/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-10-10
 *      Author: fasiondog
 */

#include "hikyuu/utilities/osdef.h"
#include "hikyuu/utilities/thread/thread.h"
#include "hikyuu/indicator/crt/EXIST.h"
#include "hikyuu/trade_sys/signal/crt/SG_Bool.h"
#include "combinate.h"
#include "analysis_sys.h"

namespace hku {

std::vector<Indicator> HKU_API combinateIndicator(const std::vector<Indicator>& inputs, int n) {
    std::vector<Indicator> ret;
    auto indexs = combinateIndex(inputs);
    for (size_t i = 0, len = indexs.size(); i < len; i++) {
        size_t count = indexs[i].size();
        Indicator tmp = EXIST(inputs[indexs[i][0]], n);
        std::string name = inputs[indexs[i][0]].name();
        for (size_t j = 1; j < count; j++) {
            tmp = tmp & EXIST(inputs[indexs[i][j]], n);
            name = fmt::format("{} & {}", name, inputs[indexs[i][j]].name());
        }
        tmp.name(name);
        ret.emplace_back(tmp);
    }
    return ret;
}

std::map<std::string, Performance> HKU_API combinateIndicatorAnalysis(
  const Stock& stk, const KQuery& query, TradeManagerPtr tm, SystemPtr sys,
  const std::vector<Indicator>& buy_inds, const std::vector<Indicator>& sell_inds, int n) {
    HKU_CHECK(tm, "tm is null!");
    HKU_CHECK(sys, "sys is null!");

    auto inds = combinateIndicator(buy_inds, n);
    std::vector<SignalPtr> sgs;
    for (const auto& buy_ind : inds) {
        for (const auto& sell_ind : sell_inds) {
            auto sg = SG_Bool(buy_ind, sell_ind);
            sg->name(fmt::format("{} + {}", buy_ind.name(), sell_ind.name()));
            sgs.emplace_back(sg);
        }
    }

    std::map<std::string, Performance> result;
    // The statistics must stop at the last trading day of the query, as analysis_sys does; now()
    // would value the open positions with data beyond the backtest window.
    DatetimeList date_list = getAnalysisCalendar(StockList{stk}, query);
    HKU_IF_RETURN(date_list.empty(), result);
    Datetime last_datetime = date_list.back();

    for (const auto& sg : sgs) {
        sys->setSG(sg);
        sys->setTM(tm);
        sys->run(stk, query);
        Performance per;
        per.statistics(tm, last_datetime);
        result[sg->name()] = std::move(per);
    }

    return result;
}

vector<CombinateAnalysisOutput> HKU_API combinateIndicatorAnalysisWithBlock(
  const Block& blk, const KQuery& query, TradeManagerPtr tm, SystemPtr sys,
  const std::vector<Indicator>& buy_inds, const std::vector<Indicator>& sell_inds, int n) {
    HKU_CHECK(tm, "tm is null!");
    HKU_CHECK(sys, "sys is null!");

    SPEND_TIME(combinateIndicatorAnalysisWithBlock);
    auto inds = combinateIndicator(buy_inds, n);
    std::vector<SignalPtr> sgs;
    for (const auto& buy_ind : inds) {
        for (const auto& sell_ind : sell_inds) {
            auto sg = SG_Bool(buy_ind, sell_ind);
            sg->name(fmt::format("{} + {}", buy_ind.name(), sell_ind.name()));
            sgs.emplace_back(sg);
        }
    }

    vector<CombinateAnalysisOutput> result;
    auto stocks = blk.getStockList();
    size_t total = stocks.size();
    HKU_IF_RETURN(total == 0, result);

    DatetimeList date_list = getAnalysisCalendar(stocks, query);
    HKU_IF_RETURN(date_list.empty(), result);
    Datetime last_datetime = date_list.back();

    // auto work_num = std::thread::hardware_concurrency();
    // ThreadPool tg(work_num);
    auto* tg = get_global_task_group();
    vector<std::future<vector<CombinateAnalysisOutput>>> tasks;

    vector<Stock> buf;
    auto ranges = parallelIndexRange(0, total);
    for (const auto& range : ranges) {
        buf.clear();
        for (size_t i = range.first; i < range.second; i++) {
            buf.emplace_back(stocks[i]);
        }
        tasks.emplace_back(tg->submit([sgs, stks = std::move(buf), n_query = query, last_datetime,
                                       n_tm = tm->clone(), n_sys = sys->clone()]() {
            vector<CombinateAnalysisOutput> ret;
            Performance per;
            CombinateAnalysisOutput out;
            for (size_t i = 0, len = stks.size(); i < len; i++) {
                const Stock& n_stk = stks[i];
                for (const auto& sg : sgs) {
                    try {
                        auto n_sg = sg->clone();
                        n_sys->setSG(n_sg);
                        n_sys->setTM(n_tm);
                        n_sys->run(n_stk, n_query);
                        per.statistics(n_tm, last_datetime);
                        out.combinateName = n_sg->name();
                        out.market_code = n_stk.market_code();
                        out.name = n_stk.name();
                        out.values = per.values();
                        ret.emplace_back(out);
                    } catch (const std::exception& e) {
                        HKU_ERROR(e.what());
                    } catch (...) {
                        HKU_ERROR("Unknown error!");
                    }
                }
            }
            return ret;
        }));
    }

    result.reserve(sgs.size() * stocks.size());
    for (auto& task : tasks) {
        auto records = task.get();
        for (auto& record : records) {
            result.emplace_back(std::move(record));
        }
    }
    return result;
}

}  // namespace hku