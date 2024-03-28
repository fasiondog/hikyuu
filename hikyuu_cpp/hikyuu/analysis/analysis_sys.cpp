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

    result = parallel_for_index(0, total, [&](size_t i) {
        const auto& sys = sys_list[i];
        const auto& stk = stk_list[i];

        AnalysisSystemWithBlockOut ret;
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

vector<AnalysisSystemWithBlockOut> HKU_API analysisSystemList(const StockList& stk_list,
                                                              const KQuery& query,
                                                              const SystemPtr& pro_sys) {
    HKU_CHECK(pro_sys, "pro_sys is null!");

    return parallel_for_range(0, stk_list.size(), [=](const range_t& range) {
        vector<AnalysisSystemWithBlockOut> ret;
        auto sys = pro_sys->clone();
        Performance per;
        AnalysisSystemWithBlockOut out;
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

}  // namespace hku
