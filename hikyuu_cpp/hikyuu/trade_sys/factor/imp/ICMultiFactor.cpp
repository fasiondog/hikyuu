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
#include "ICMultiFactor.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ICMultiFactor)
#endif

namespace hku {

ICMultiFactor::ICMultiFactor() : MultiFactorBase("MF_ICWeight") {}

ICMultiFactor::ICMultiFactor(const IndicatorList& inds, const StockList& stks, const KQuery& query,
                             const Stock& ref_stk, int ic_n)
: MultiFactorBase(inds, stks, query, ref_stk, "MF_ICWeight", ic_n) {}

IndicatorList ICMultiFactor::_calculate(const vector<IndicatorList>& all_stk_inds) {
    size_t days_total = m_ref_dates.size();
    size_t stk_count = m_stks.size();
    size_t ind_count = m_inds.size();

    int ic_n = getParam<int>("ic_n");

    IndicatorList all_factors(stk_count);

    IndicatorList ic(ind_count);
    for (size_t ii = 0; ii < ind_count; ii++) {
        ic[ii] = IC(m_inds[ii], m_stks, m_query, ic_n, m_ref_stk);
    }

    // 计算 IC 权重
    vector<value_t> sumByDate(days_total, 0.0);
    vector<size_t> countByDate(days_total, 0);
    for (size_t di = 0; di < days_total; di++) {
        for (size_t ii = 0; ii < ind_count; ii++) {
            auto value = ic[ii][di];
            if (!std::isnan(value)) {
                sumByDate[di] += value;
                countByDate[di] += 1;
            }
        }
    }

    for (size_t di = 0; di < days_total; di++) {
        if (countByDate[di] == 0) {
            sumByDate[di] = Null<value_t>();
        } else {
            sumByDate[di] = sumByDate[di] / countByDate[di];
        }
    }

    for (size_t si = 0; si < stk_count; si++) {
        PriceList new_values(days_total);
        for (size_t di = 0; di < days_total; di++) {
            for (size_t ii = 0; ii < ind_count; ii++) {
                auto value = all_stk_inds[si][ii][di];
                new_values[di] = value * sumByDate[di];
            }
        }
        all_factors[si] = PRICELIST(new_values);
    }

    return all_factors;
}

MultiFactorPtr HKU_API MF_ICWeight(const IndicatorList& inds, const StockList& stks,
                                   const KQuery& query, const Stock& ref_stk, int ic_n) {
    return std::make_shared<ICMultiFactor>(inds, stks, query, ref_stk, ic_n);
}

}  // namespace hku