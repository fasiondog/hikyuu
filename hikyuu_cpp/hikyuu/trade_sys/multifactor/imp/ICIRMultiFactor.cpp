/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-13
 *      Author: fasiondog
 */

#include "hikyuu/utilities/thread/algorithm.h"
#include "hikyuu/indicator/crt/PRICELIST.h"
#include "hikyuu/indicator/crt/IC.h"
#include "hikyuu/indicator/crt/ICIR.h"
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
                                 int ic_rolling_n, bool spearman)
: MultiFactorBase(inds, stks, query, ref_stk, "MF_ICIRWeight", ic_n, spearman) {
    setParam<int>("ic_rolling_n", ic_rolling_n);
    checkParam("ic_rolling_n");
}

void ICIRMultiFactor::_checkParam(const string& name) const {
    if ("ic_rolling_n" == name) {
        HKU_ASSERT(getParam<int>("ic_rolling_n") >= 1);
    }
}

IndicatorList ICIRMultiFactor::_calculate(const vector<IndicatorList>& all_stk_inds) {
    size_t days_total = m_ref_dates.size();
    size_t stk_count = m_stks.size();
    size_t ind_count = m_inds.size();

    int ic_n = getParam<int>("ic_n");
    int ir_n = getParam<int>("ic_rolling_n");
    bool spearman = getParam<bool>("use_spearman");

#if !MF_USE_MULTI_THREAD
    size_t discard = 0;
    vector<Indicator> icir(ind_count);
    for (size_t ii = 0; ii < ind_count; ii++) {
        icir[ii] = ICIR(m_inds[ii], m_stks, m_query, m_ref_stk, ic_n, ir_n, spearman);
        if (icir[ii].discard() > discard) {
            discard = icir[ii].discard();
        }
    }
#else
    vector<Indicator> icir =
      parallel_for_index(0, ind_count, [this, ic_n, ir_n, spearman](size_t ii) {
          return ICIR(m_inds[ii], m_stks, m_query, m_ref_stk, ic_n, ir_n, spearman);
      });

    size_t discard = 0;
    for (size_t ii = 0; ii < ind_count; ii++) {
        if (icir[ii].discard() > discard) {
            discard = icir[ii].discard();
        }
    }
#endif

    // 以 ICIR 为权重，计算加权后的合成因子
#if !MF_USE_MULTI_THREAD
    IndicatorList all_factors(stk_count);
    PriceList new_values(days_total, 0.0);
    PriceList sum_weight(days_total, 0.0);
    for (size_t si = 0; si < stk_count; si++) {
        memset(new_values.data(), 0, sizeof(price_t) * days_total);
        memset(sum_weight.data(), 0, sizeof(price_t) * days_total);
        for (size_t di = 0; di < discard; di++) {
            new_values[di] = Null<price_t>();
        }
        for (size_t ii = 0; ii < ind_count; ii++) {
            const auto* ind_data = all_stk_inds[si][ii].data();
            const auto* icir_data = icir[ii].data();
            for (size_t di = discard; di < days_total; di++) {
                new_values[di] += ind_data[di] * icir_data[di];
                sum_weight[di] += std::abs(icir_data[di]);
            }
        }

        for (size_t di = discard; di < days_total; di++) {
            if (!std::isnan(new_values[di]) && sum_weight[di] != 0.0) {
                new_values[di] = new_values[di] / sum_weight[di];
            }
        }

        all_factors[si] = PRICELIST(new_values);
        all_factors[si].name("ICIR");

        const auto* data = all_factors[si].data();
        for (size_t di = discard; di < days_total; di++) {
            if (!std::isnan(data[di])) {
                all_factors[si].setDiscard(discard);
            }
        }
    }

    return all_factors;
#else
    return parallel_for_index(0, stk_count, [&, discard, ind_count, days_total](size_t si) {
        PriceList new_values(days_total, 0.0);
        PriceList sum_weight(days_total, 0.0);
        for (size_t di = 0; di < discard; di++) {
            new_values[di] = Null<price_t>();
        }
        for (size_t ii = 0; ii < ind_count; ii++) {
            const auto* ind_data = all_stk_inds[si][ii].data();
            const auto* icir_data = icir[ii].data();
            for (size_t di = discard; di < days_total; di++) {
                new_values[di] += ind_data[di] * icir_data[di];
                sum_weight[di] += std::abs(icir_data[di]);
            }
        }

        for (size_t di = discard; di < days_total; di++) {
            if (!std::isnan(new_values[di]) && sum_weight[di] != 0.0) {
                new_values[di] = new_values[di] / sum_weight[di];
            }
        }

        Indicator ret = PRICELIST(new_values);
        ret.name("ICIR");

        const auto* data = ret.data();
        for (size_t di = discard; di < days_total; di++) {
            if (!std::isnan(data[di])) {
                ret.setDiscard(discard);
            }
        }

        return ret;
    });
#endif
}

MultiFactorPtr HKU_API MF_ICIRWeight() {
    return make_shared<ICIRMultiFactor>();
}

MultiFactorPtr HKU_API MF_ICIRWeight(const IndicatorList& inds, const StockList& stks,
                                     const KQuery& query, const Stock& ref_stk, int ic_n,
                                     int ic_rolling_n, bool spearman) {
    return make_shared<ICIRMultiFactor>(inds, stks, query, ref_stk, ic_n, ic_rolling_n, spearman);
}

}  // namespace hku