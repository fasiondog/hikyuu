/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-13
 *      Author: fasiondog
 */

#include "hikyuu/indicator/crt/PRICELIST.h"
#include "hikyuu/indicator/crt/IC.h"
#include "hikyuu/indicator/crt/ICIR.h"
#include "hikyuu/indicator/crt/SPEARMAN.h"
#include "EqualWeightMultiFactor.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::EqualWeightMultiFactor)
#endif

namespace hku {

EqualWeightMultiFactor::EqualWeightMultiFactor() : MultiFactorBase("MF_EqualWeight") {}

EqualWeightMultiFactor::EqualWeightMultiFactor(const vector<Indicator>& inds, const StockList& stks,
                                               const KQuery& query, const Stock& ref_stk, int ic_n)
: MultiFactorBase(inds, stks, query, ref_stk, "MF_EqualWeight", ic_n) {}

vector<Indicator> EqualWeightMultiFactor::_calculate(
  const vector<vector<Indicator>>& all_stk_inds) {
    size_t days_total = m_ref_dates.size();
    size_t stk_count = m_stks.size();
    size_t ind_count = m_inds.size();

    vector<Indicator> all_factors(stk_count);
    for (size_t si = 0; si < stk_count; si++) {
        vector<price_t> sumByDate(days_total, 0.0);
        vector<size_t> countByDate(days_total, 0);

        for (size_t di = 0; di < days_total; di++) {
            for (size_t ii = 0; ii < ind_count; ii++) {
                auto value = all_stk_inds[si][ii][di];
                if (!std::isnan(value)) {
                    sumByDate[di] += value;
                    countByDate[di] += 1;
                }
            }
        }

        // 均值权重
        for (size_t di = 0; di < days_total; di++) {
            if (countByDate[di] == 0) {
                sumByDate[di] = Null<value_t>();
            } else {
                sumByDate[di] = sumByDate[di] / countByDate[di];
            }
        }

        all_factors[si] = PRICELIST(sumByDate);
    }
    return all_factors;
}

MultiFactorPtr HKU_API MF_EqualWeight(const IndicatorList& inds, const StockList& stks,
                                      const KQuery& query, const Stock& ref_stk, int ic_n) {
    return make_shared<EqualWeightMultiFactor>(inds, stks, query, ref_stk, ic_n);
}

}  // namespace hku