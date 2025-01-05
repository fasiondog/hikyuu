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

WeightMultiFactor::WeightMultiFactor(const vector<Indicator>& inds, const PriceList& weights,
                                     const StockList& stks, const KQuery& query,
                                     const Stock& ref_stk, int ic_n, bool spearman)
: MultiFactorBase(inds, stks, query, ref_stk, "MF_Weight", ic_n, spearman), m_weights(weights) {
    HKU_ASSERT(inds.size() == weights.size());
}

vector<Indicator> WeightMultiFactor::_calculate(const vector<vector<Indicator>>& all_stk_inds) {
    size_t days_total = m_ref_dates.size();
    size_t stk_count = m_stks.size();
    size_t ind_count = m_inds.size();

#if !MF_USE_MULTI_THREAD
    vector<price_t> sumByDate(days_total);
    vector<Indicator> all_factors(stk_count);
    for (size_t si = 0; si < stk_count; si++) {
        memset(sumByDate.data(), Null<price_t>(), sizeof(price_t) * days_total);

        size_t discard = 0;
        const auto& curStkInds = all_stk_inds[si];
        for (size_t ii = 0; ii < ind_count; ii++) {
            if (curStkInds[ii].discard() > discard) {
                discard = curStkInds[ii].discard();
            }
        }

        for (size_t di = discard; di < days_total; di++) {
            for (size_t ii = 0; ii < ind_count; ii++) {
                const auto& value = curStkInds[ii][di];
                if (!std::isnan(value)) {
                    sumByDate[di] += value * m_weights[ii];
                }
            }
        }

        all_factors[si] = PRICELIST(sumByDate);
        all_factors[si].name("IC");

        // 更新 discard
        for (size_t di = discard; di < days_total; di++) {
            if (!std::isnan(all_factors[si][di])) {
                all_factors[si].setDiscard(di);
                break;
            }
            if (di == days_total - 1 && std::isnan(all_factors[si][di])) {
                all_factors[si].setDiscard(di);
            }
        }
    }

    return all_factors;
#endif

    return parallel_for_index(0, stk_count, [&](size_t si) {
        vector<price_t> sumByDate(days_total);

        size_t discard = 0;
        const auto& curStkInds = all_stk_inds[si];
        for (size_t ii = 0; ii < ind_count; ii++) {
            if (curStkInds[ii].discard() > discard) {
                discard = curStkInds[ii].discard();
            }
        }

        for (size_t di = discard; di < days_total; di++) {
            for (size_t ii = 0; ii < ind_count; ii++) {
                const auto& value = curStkInds[ii][di];
                if (!std::isnan(value)) {
                    sumByDate[di] += value * m_weights[ii];
                }
            }
        }

        Indicator ret = PRICELIST(sumByDate);
        ret.name("IC");

        // 更新 discard
        for (size_t di = discard; di < days_total; di++) {
            if (!std::isnan(ret[di])) {
                ret.setDiscard(di);
                break;
            }
            if (di == days_total - 1 && std::isnan(ret[di])) {
                ret.setDiscard(di);
            }
        }
        return ret;
    });
}

MultiFactorPtr HKU_API MF_Weight() {
    return make_shared<WeightMultiFactor>();
}

MultiFactorPtr HKU_API MF_Weight(const IndicatorList& inds, const PriceList& weights,
                                 const StockList& stks, const KQuery& query, const Stock& ref_stk,
                                 int ic_n, bool spearman) {
    HKU_CHECK(
      weights.size() == inds.size(),
      "The size of weight is not equal to the size of inds! weights.size()={}, inds.size()={}",
      weights.size(), inds.size());
    return make_shared<WeightMultiFactor>(inds, weights, stks, query, ref_stk, ic_n, spearman);
}

}  // namespace hku