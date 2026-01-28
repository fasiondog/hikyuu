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
                                 int ic_rolling_n, bool spearman, int mode, bool save_all_factors)
: MultiFactorBase(inds, stks, query, ref_stk, "MF_ICIRWeight", ic_n, spearman, mode,
                  save_all_factors) {
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

    auto ref_k = m_ref_stk.getKData(m_query);

    vector<Indicator> icir =
      global_parallel_for_index(0, ind_count, [this, ic_n, ir_n, spearman, &ref_k](size_t ii) {
          return ICIR(m_inds[ii], m_stks, ic_n, ir_n, spearman)(ref_k).getResult(0)();
      });

    size_t discard = 0;
    for (size_t ii = 0; ii < ind_count; ii++) {
        if (icir[ii].discard() > discard) {
            discard = icir[ii].discard();
        }
    }

    return global_parallel_for_index(0, stk_count, [&, discard, ind_count, days_total](size_t si) {
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
}

MultiFactorPtr HKU_API MF_ICIRWeight() {
    return make_shared<ICIRMultiFactor>();
}

MultiFactorPtr HKU_API MF_ICIRWeight(const IndicatorList& inds, const StockList& stks,
                                     const KQuery& query, const Stock& ref_stk, int ic_n,
                                     int ic_rolling_n, bool spearman, int mode,
                                     bool save_all_factors) {
    return make_shared<ICIRMultiFactor>(inds, stks, query, ref_stk, ic_n, ic_rolling_n, spearman,
                                        mode, save_all_factors);
}

}  // namespace hku