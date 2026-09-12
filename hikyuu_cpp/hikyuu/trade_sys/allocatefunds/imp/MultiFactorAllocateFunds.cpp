/*
 * MultiFactorAllocateFunds.cpp
 *
 *  Copyright (c) 2025 hikyuu.org
 */

#include "MultiFactorAllocateFunds.h"

#include <algorithm>

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::MultiFactorAllocateFunds)
#endif

namespace hku {

MultiFactorAllocateFunds::MultiFactorAllocateFunds() : AllocateFundsBase("AF_MultiFactor") {}

MultiFactorAllocateFunds::~MultiFactorAllocateFunds() {}

AllocateFundsBase::Weights MultiFactorAllocateFunds::_allocate(const Datetime& date,
                                                               const TradeManagerPtr& tm,
                                                               SubSystemContextList& contexts,
                                                               const KQuery& query) {
    std::vector<double> weights(contexts.size(), 0.0);
    double sum = 0.0;
    for (size_t i = 0; i < contexts.size(); ++i) {
        double s = contexts[i].score;
        if (s < 0.0) {
            s = 0.0;  // 负分按 0 处理（不支持做空额度分配）
        }
        weights[i] = s;
        sum += s;
    }
    if (sum <= 0.0) {
        // 未配置 SE / 非调仓日 / 得分全零：回退等权，避免零权重导致无法分配
        HKU_WARN("AF_MultiFactor: all SE score is zero/absent, fallback to equal weight! [{}]",
                 name());
        if (!contexts.empty()) {
            weights.assign(contexts.size(), 1.0 / contexts.size());
        }
    }
    // 不归一化：得分即权重（等价 master MultiFactorAllocaterFunds 原样返回 se_list）
    return _applyWeights(date, tm, contexts, query, weights);
}

}  // namespace hku
