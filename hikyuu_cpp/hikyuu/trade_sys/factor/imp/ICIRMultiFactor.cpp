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

    // 计算 IC 权重
    vector<value_t> sumByDate(days_total, 0.0);
    vector<size_t> countByDate(days_total, 0);
    for (size_t di = discard; di < days_total; di++) {
        for (size_t ii = 0; ii < ind_count; ii++) {
            const auto& value = icir[ii][di];
            if (!std::isnan(value)) {
                sumByDate[di] += value;
                countByDate[di] += 1;
            }
        }
    }

    for (size_t di = discard; di < days_total; di++) {
        sumByDate[di] = (countByDate[di] == 0) ? Null<value_t>() : sumByDate[di] / countByDate[di];
    }

    vector<Indicator> all_factors(stk_count);
    PriceList new_values(days_total);
    for (size_t si = 0; si < stk_count; si++) {
        memset(new_values.data(), 0, sizeof(price_t) * days_total);
        for (size_t di = discard; di < days_total; di++) {
            for (size_t ii = 0; ii < ind_count; ii++) {
                const auto& value = all_stk_inds[si][ii][di];
                new_values[di] += value * sumByDate[di];
            }
        }
        for (size_t di = discard; di < days_total; di++) {
            new_values[di] =
              (countByDate[di] == 0) ? Null<value_t>() : new_values[di] / countByDate[di];
        }
        all_factors[si] = PRICELIST(new_values);
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
}

MultiFactorPtr HKU_API MF_ICIRWeight(const IndicatorList& inds, const StockList& stks,
                                     const KQuery& query, const Stock& ref_stk, int ic_n,
                                     int ic_rolling_n) {
    return make_shared<ICIRMultiFactor>(inds, stks, query, ref_stk, ic_n, ic_rolling_n);
}

}  // namespace hku