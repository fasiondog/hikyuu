/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-13
 *      Author: fasiondog
 */

#include "hikyuu/utilities/thread/algorithm.h"
#include "hikyuu/indicator/crt/PRICELIST.h"
#include "EqualWeightMultiFactor.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::EqualWeightMultiFactor)
#endif

namespace hku {

EqualWeightMultiFactor::EqualWeightMultiFactor() : MultiFactorBase("MF_EqualWeight") {}

EqualWeightMultiFactor::EqualWeightMultiFactor(const StockList& stks, const KQuery& query,
                                               const Stock& ref_stk, int ic_n, bool spearman,
                                               int mode, bool save_all_factors)
: MultiFactorBase(stks, query, ref_stk, "MF_EqualWeight", ic_n, spearman, mode, save_all_factors) {}

IndicatorList EqualWeightMultiFactor::_calculate(const vector<IndicatorList>& all_stk_inds) {
    size_t days_total = m_ref_dates.size();
    size_t stk_count = m_stks.size();
    // 修改：正确处理FactorSet和IndicatorList两种情况
    size_t ind_count = m_inds.empty() ? m_factorset.size() : m_inds.size();

    return global_parallel_for_index(0, stk_count, [&](size_t si) {
        vector<price_t> sumByDate(days_total);
        vector<size_t> countByDate(days_total);

        const auto& curStkInds = all_stk_inds[si];
        for (size_t ii = 0; ii < ind_count; ii++) {
            const auto* curInd = curStkInds[ii].data();
            for (size_t di = 0; di < days_total; di++) {
                auto value = curInd[di];
                if (!std::isnan(value)) {
                    sumByDate[di] += value;
                    countByDate[di] += 1;
                }
            }
        }

        // 均值权重
        for (size_t di = 0; di < days_total; di++) {
            sumByDate[di] =
              (countByDate[di] == 0) ? Null<value_t>() : sumByDate[di] / countByDate[di];
        }

        Indicator ret = PRICELIST(sumByDate);
        ret.updateDiscard(true);
        ret.name("IC");
        return ret;
    });
}

MultiFactorPtr HKU_API MF_EqualWeight() {
    return make_shared<EqualWeightMultiFactor>();
}

MultiFactorPtr HKU_API MF_EqualWeight(const StockList& stks, const KQuery& query,
                                      const Stock& ref_stk, int ic_n, bool spearman, int mode,
                                      bool save_all_factors) {
    return make_shared<EqualWeightMultiFactor>(stks, query, ref_stk, ic_n, spearman, mode,
                                               save_all_factors);
}

}  // namespace hku