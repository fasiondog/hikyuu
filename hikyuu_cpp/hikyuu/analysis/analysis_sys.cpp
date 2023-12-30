/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-25
 *      Author: fasiondog
 */

#include "hikyuu/Block.h"
#include "analysis_sys.h"

namespace hku {

vector<AnalysisSystemWithBlockOut> HKU_API analysisSystemList(const SystemList& sys_list,
                                                              const StockList& stk_list,
                                                              const KQuery& query) {
    HKU_ASSERT(sys_list.size() == stk_list.size());

    vector<AnalysisSystemWithBlockOut> result;
    size_t total = sys_list.size();
    HKU_IF_RETURN(0 == total, result);

    MQStealThreadPool tg;
    vector<std::future<AnalysisSystemWithBlockOut>> tasks;

    for (size_t i = 0; i < total; i++) {
        const auto& sys = sys_list[i];
        const auto& stk = stk_list[i];
        if (!sys || stk.isNull()) {
            continue;
        }

        tasks.emplace_back(tg.submit([&sys, &stk, &query]() {
            AnalysisSystemWithBlockOut ret;
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
        }));
    }

    for (auto& task : tasks) {
        result.emplace_back(task.get());
    }
    return result;
}

}  // namespace hku
