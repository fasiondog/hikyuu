/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-09-10
 *      Author: fasiondog
 */

#include "../utilities/thread/ThreadPool.h"
#include "SignalCombinationAnalysis.h"

namespace hku {

void HKU_API signalCombinationAnaysis(const vector<SGPtr>& sgs, const SYSPtr& sys,
                                      const TMPtr& tm) {
    vector<vector<size_t>> seqs = generateCombinationsIndex(sgs);

    ThreadPool tg(std::thread::hardware_concurrency());

    for (const auto& seq : seqs) {
        SGPtr sg = sgs[seq[0]]->clone();
        for (size_t i = 1, len = seq.size(); i < len; i++) {
        }
    }
}

}  // namespace hku