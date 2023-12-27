/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-25
 *      Author: fasiondog
 */

#include "hikyuu/Block.h"
#include "analysis_sys.h"

namespace hku {

vector<AnalysisSystemWithBlockOut> analysisSystemListWithBlock(const Block& blk,
                                                               const KQuery& query,
                                                               const SystemPtr sys_proto) {
    vector<AnalysisSystemWithBlockOut> result;
    HKU_IF_RETURN(blk.empty() || !sys_proto, result);

    //     auto cpu_num = std::thread::hardware_concurrency();
    // #if HKU_OS_WINDOWS
    //     size_t work_num = 5;
    //     if (cpu_num < work_num) {
    //         work_num = cpu_num;
    //     }
    // #else
    //     size_t work_num = cpu_num;
    // #endif
    // MQStealThreadPool tg(work_num);
    MQStealThreadPool tg;
    vector<std::future<AnalysisSystemWithBlockOut>> tasks;

    auto iter = blk.begin();
    for (; iter != blk.end(); ++iter) {
        if (iter->isNull()) {
            continue;
        }
        // HKU_INFO("{}", *iter);
        auto kdata = iter->getKData(query);

        // auto stk = kdata.getStock();
        // auto sys = sys_proto->clone();
        // AnalysisSystemWithBlockOut ret;
        // try {
        //     auto stk = kdata.getStock();
        //     HKU_CHECK(sys, "sys is null!");
        //     sys->run(kdata);
        //     // Performance per;
        //     // per.statistics(sys->getTM());
        //     ret.market_code = stk.market_code();
        //     ret.name = stk.name();
        //     // ret.values = per.values();
        // } catch (const std::exception& e) {
        //     // HKU_ERROR(e.what());
        // } catch (...) {
        //     // HKU_ERROR("Unknown error!");
        // }
        // result.emplace_back(ret);

        tasks.emplace_back(tg.submit([sys = sys_proto->clone(), kdata = iter->getKData(query)]() {
            AnalysisSystemWithBlockOut ret;
            try {
                auto stk = kdata.getStock();
                HKU_CHECK(sys, "sys is null!");
                sys->run(kdata);
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
