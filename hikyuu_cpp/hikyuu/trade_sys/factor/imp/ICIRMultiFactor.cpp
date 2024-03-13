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
#include "ICIRMultiFactor.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ICIRMultiFactor)
#endif

namespace hku {

ICIRMultiFactor::ICIRMultiFactor() : MultiFactorBase("MF_ICIRWeight") {
    setParam<int>("ic_n", 1);
    setParam<int>("ir_n", 10);
}

ICIRMultiFactor::ICIRMultiFactor(const vector<Indicator>& inds, const StockList& stks,
                                 const KQuery& query, const Stock& ref_stk)
: MultiFactorBase(inds, stks, query, ref_stk, "MF_ICIRWeight") {
    setParam<int>("ic_n", 1);
    setParam<int>("ir_n", 10);
}

IndicatorList ICIRMultiFactor::_calculate(const vector<IndicatorList>& all_stk_inds) {
    size_t days_total = m_ref_dates.size();
    size_t stk_count = m_stks.size();
    size_t ind_count = m_inds.size();

    int ic_n = getParam<int>("ic_n");
    int ir_n = getParam<int>("ir_n");

    vector<Indicator> all_factors(stk_count);

    vector<Indicator> icir(ind_count);
    for (size_t ii = 0; ii < ind_count; ii++) {
        icir[ii] = ICIR(IC(m_inds[ii], m_stks, m_query, ic_n, m_ref_stk), ir_n);
    }

    // 计算 IC 权重
    vector<value_t> sumByDate(days_total, 0.0);
    vector<size_t> countByDate(days_total, 0);
    for (size_t di = 0; di < days_total; di++) {
        for (size_t ii = 0; ii < ind_count; ii++) {
            auto value = icir[ii][di];
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

}  // namespace hku