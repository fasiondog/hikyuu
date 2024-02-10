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
#include "hikyuu/global/GlobalTaskGroup.h"
#include "combinate.h"

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
    for (const auto& sg : sgs) {
        sys->setSG(sg);
        sys->setTM(tm);
        sys->run(stk, query);
        Performance per;
        per.statistics(tm, Datetime::now());
        result[sg->name()] = std::move(per);
    }

    return result;
}

vector<CombinateAnalysisOutput> HKU_API combinateIndicatorAnalysisWithBlock(
  const Block& blk, const KQuery& query, TradeManagerPtr tm, SystemPtr sys,
  const std::vector<Indicator>& buy_inds, const std::vector<Indicator>& sell_inds, int n) {
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
    auto cpu_num = std::thread::hardware_concurrency();
#if HKU_OS_WINDOWS
    size_t work_num = 5;
    if (cpu_num < work_num) {
        work_num = cpu_num;
    }
#else
    size_t work_num = cpu_num;
#endif
    MQStealThreadPool tg(work_num);
    vector<std::future<vector<CombinateAnalysisOutput>>> tasks;

    auto stocks = blk.getAllStocks();
    size_t total = stocks.size();
    HKU_IF_RETURN(total == 0, result);

    size_t per_num = total > work_num ? total / (work_num * 10) : 1;
    size_t count = total % per_num == 0 ? total / per_num : total / per_num + 1;

    vector<Stock> buf;
    for (size_t i = 0; i < count; i++) {
        buf.clear();
        for (size_t j = i * per_num, end = (i + 1) * per_num; j < end; j++) {
            if (j >= stocks.size()) {
                break;
            }
            buf.emplace_back(stocks[j]);
        }
        tasks.emplace_back(tg.submit([sgs, stks = std::move(buf), n_query = query,
                                      n_tm = tm->clone(), n_sys = sys->clone()]() {
            vector<CombinateAnalysisOutput> ret;
            Performance per;
            for (size_t i = 0, len = stks.size(); i < len; i++) {
                const Stock& n_stk = stks[i];
                for (const auto& sg : sgs) {
                    try {
                        auto n_sg = sg->clone();
                        n_sys->setSG(n_sg);
                        n_sys->setTM(n_tm);
                        n_sys->run(n_stk, n_query);
                        per.statistics(n_tm, Datetime::now());
                        CombinateAnalysisOutput out;
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

    for (auto& task : tasks) {
        // result.emplace_back(std::move(task.get()));
        auto records = task.get();
        for (auto& record : records) {
            result.emplace_back(record);
        }
    }

    return result;
}

}  // namespace hku