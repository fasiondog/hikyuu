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
    setParam<int>("ic_rolling_n", 120);
}

ICIRMultiFactor::ICIRMultiFactor(const vector<Indicator>& inds, const StockList& stks,
                                 const KQuery& query, const Stock& ref_stk, int ic_n,
                                 int ic_rolling_n)
: MultiFactorBase(inds, stks, query, ref_stk, "MF_ICIRWeight", ic_n) {
    setParam<int>("ic_rolling_n", ic_rolling_n);
}

IndicatorList ICIRMultiFactor::_calculate(const vector<IndicatorList>& all_stk_inds) {
    size_t days_total = m_ref_dates.size();
    size_t stk_count = m_stks.size();
    size_t ind_count = m_inds.size();

    int ic_n = getParam<int>("ic_n");
    int ir_n = getParam<int>("ic_rolling_n");

    size_t discard = 0;
    vector<Indicator> icir(ind_count);
    for (size_t ii = 0; ii < ind_count; ii++) {
        icir[ii] = ICIR(IC(m_inds[ii], m_stks, m_query, ic_n, m_ref_stk), ir_n);
        if (icir[ii].discard() > discard) {
            discard = icir[ii].discard();
        }
    }

    // 以 ICIR 为权重，计算加权后的合成因子
    IndicatorList all_factors(stk_count);
    PriceList new_values(days_total, 0);
    for (size_t si = 0; si < stk_count; si++) {
        memset(new_values.data(), 0, sizeof(price_t) * days_total);
        for (size_t di = discard; di < days_total; di++) {
            for (size_t ii = 0; ii < ind_count; ii++) {
                if (!std::isnan(all_stk_inds[si][ii][di]) && !std::isnan(icir[ii][di])) {
                    new_values[di] += all_stk_inds[si][ii][di] * icir[ii][di];
                }
            }
        }
        all_factors[si] = PRICELIST(new_values);
        all_factors[si].name("ICIR");
        all_factors[si].setDiscard(discard);
    }

    return all_factors;
}

MultiFactorPtr HKU_API MF_ICIRWeight(const IndicatorList& inds, const StockList& stks,
                                     const KQuery& query, const Stock& ref_stk, int ic_n,
                                     int ic_rolling_n) {
    return make_shared<ICIRMultiFactor>(inds, stks, query, ref_stk, ic_n, ic_rolling_n);
}

}  // namespace hku