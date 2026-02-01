/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-13
 *      Author: fasiondog
 */

#include "hikyuu/utilities/thread/algorithm.h"
#include "hikyuu/indicator/crt/PRICELIST.h"
#include "hikyuu/indicator/crt/IC.h"
#include "hikyuu/indicator/crt/MA.h"
#include "ICMultiFactor.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ICMultiFactor)
#endif

namespace hku {

ICMultiFactor::ICMultiFactor() : MultiFactorBase("MF_ICWeight") {
    setParam<int>("ic_rolling_n", 120);  // 计算滚动ic的滚动周期, 通常取 120 或 250
}

ICMultiFactor::ICMultiFactor(const IndicatorList& inds, const StockList& stks, const KQuery& query,
                             const Stock& ref_stk, int ic_n, int ic_rolling_n, bool spearman,
                             int mode, bool save_all_factors)
: MultiFactorBase(inds, stks, query, ref_stk, "MF_ICWeight", ic_n, spearman, mode,
                  save_all_factors) {
    setParam<int>("ic_rolling_n", ic_rolling_n);
    checkParam("ic_rolling_n");
}

void ICMultiFactor::_checkParam(const string& name) const {
    if ("ic_rolling_n" == name) {
        HKU_ASSERT(getParam<int>("ic_rolling_n") >= 1);
    }
}

IndicatorList ICMultiFactor::_calculate(const vector<IndicatorList>& all_stk_inds) {
    size_t days_total = m_ref_dates.size();
    size_t stk_count = m_stks.size();
    size_t ind_count = m_inds.size();

    int ic_n = getParam<int>("ic_n");
    int ic_rolling_n = getParam<int>("ic_rolling_n");
    bool spearman = getParam<bool>("use_spearman");

    // 计算每个经过标准化/风格化的原始因子的滚动IC值
    IndicatorList all_returns = _getAllReturns(ic_n);
    IndicatorList ic =
      global_parallel_for_index(0, ind_count, [&, ic_n, spearman, ic_rolling_n](size_t ii) {
          IndicatorList inds_stk;
          inds_stk.reserve(stk_count);
          for (size_t si = 0; si < stk_count; si++) {
              inds_stk.push_back(all_stk_inds[si][ii]);
          }
          return MA(IC(inds_stk, all_returns, ic_n, spearman), ic_rolling_n).getResult(0);
      });

    size_t discard = 0;
    for (size_t ii = 0; ii < ind_count; ii++) {
        if (ic[ii].discard() > discard) {
            discard = ic[ii].discard();
        }
    }

    return global_parallel_for_index(0, stk_count, [&, ind_count, days_total, discard](size_t si) {
        PriceList new_values(days_total, 0.0);
        PriceList sum_weight(days_total, 0.0);
        for (size_t di = 0; di < discard; di++) {
            new_values[di] = Null<price_t>();
        }

        for (size_t ii = 0; ii < ind_count; ii++) {
            const auto* ind_data = all_stk_inds[si][ii].data();
            const auto* ic_data = ic[ii].data();
            for (size_t di = discard; di < days_total; di++) {
                new_values[di] += ind_data[di] * ic_data[di];
                sum_weight[di] += std::abs(ic_data[di]);
            }
        }

        for (size_t di = discard; di < days_total; di++) {
            if (!std::isnan(new_values[di]) && sum_weight[di] != 0.0) {
                new_values[di] = new_values[di] / sum_weight[di];
            }
        }

        Indicator ret = PRICELIST(new_values);
        ret.name("IC");

        const auto* data = ret.data();
        for (size_t di = discard; di < days_total; di++) {
            if (!std::isnan(data[di])) {
                ret.setDiscard(discard);
            }
        }

        return ret;
    });
}

MultiFactorPtr HKU_API MF_ICWeight() {
    return std::make_shared<ICMultiFactor>();
}

MultiFactorPtr HKU_API MF_ICWeight(const IndicatorList& inds, const StockList& stks,
                                   const KQuery& query, const Stock& ref_stk, int ic_n,
                                   int ic_rolling_n, bool spearman, int mode,
                                   bool save_all_factors) {
    return std::make_shared<ICMultiFactor>(inds, stks, query, ref_stk, ic_n, ic_rolling_n, spearman,
                                           mode, save_all_factors);
}

}  // namespace hku