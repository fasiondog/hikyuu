/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-13
 *      Author: fasiondog
 */

#include "hikyuu/utilities/thread/algorithm.h"
#include "hikyuu/indicator/crt/PRICELIST.h"
#include "WeightMultiFactor.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::WeightMultiFactor)
#endif

namespace hku {

WeightMultiFactor::WeightMultiFactor() : MultiFactorBase("MF_Weight") {}

WeightMultiFactor::WeightMultiFactor(const PriceList& weights, const StockList& stks,
                                     const KQuery& query, const Stock& ref_stk, int ic_n,
                                     bool spearman, int mode, bool save_all_factors)
: MultiFactorBase(stks, query, ref_stk, "MF_Weight", ic_n, spearman, mode, save_all_factors),
  m_weights(weights) {}

MultiFactorPtr WeightMultiFactor::_clone() {
    return make_shared<WeightMultiFactor>(
      m_weights, m_stks, m_query, m_ref_stk, getParam<int>("ic_n"), getParam<bool>("use_spearman"),
      getParam<int>("mode"), getParam<bool>("save_all_factors"));
}

vector<Indicator> WeightMultiFactor::_calculate(const vector<IndicatorList>& all_stk_inds) {
    size_t days_total = m_ref_dates.size();
    size_t stk_count = m_stks.size();
    size_t ind_count = m_factorset.size();

    HKU_CHECK(m_weights.size() == ind_count,
              "The size of weight is not equal to the number of the factors! weights.size()={}, "
              "ind_count={}",
              m_weights.size(), ind_count);

    return global_parallel_for_index(0, stk_count, [&](size_t si) {
        vector<price_t> sumByDate(days_total, 0.0);
        vector<size_t> countByDate(days_total, 0);
        const auto& curStkInds = all_stk_inds[si];
        for (size_t ii = 0; ii < ind_count; ii++) {
            const auto& curInd = curStkInds[ii];
            const auto* ind_data = curInd.data();
            for (size_t di = 0; di < days_total; di++) {
                auto value = ind_data[di];
                if (!std::isnan(value)) {
                    sumByDate[di] += value * m_weights[ii];
                    countByDate[di] += 1;
                }
            }
        }

        // A day with every factor missing has no contribution: keep it Null instead of a fake 0.0
        // score, so the warm-up is not treated as valid data nor silently selected
        for (size_t di = 0; di < days_total; di++) {
            if (countByDate[di] == 0) {
                sumByDate[di] = Null<price_t>();
            }
        }

        Indicator ret = PRICELIST(sumByDate);
        ret.updateDiscard(true);
        ret.name("IC");
        return ret;
    });
}

MultiFactorPtr HKU_API MF_Weight() {
    return make_shared<WeightMultiFactor>();
}

MultiFactorPtr HKU_API MF_Weight(const PriceList& weights, const StockList& stks,
                                 const KQuery& query, const Stock& ref_stk, int ic_n, bool spearman,
                                 int mode, bool save_all_factors) {
    return make_shared<WeightMultiFactor>(weights, stks, query, ref_stk, ic_n, spearman, mode,
                                          save_all_factors);
}

}  // namespace hku