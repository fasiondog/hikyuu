/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-12
 *      Author: fasiondog
 */

#include "hikyuu/indicator/crt/ALIGN.h"
#include "hikyuu/indicator/crt/ROCP.h"
#include "hikyuu/indicator/crt/PRICELIST.h"
#include "hikyuu/indicator/crt/IC.h"
#include "hikyuu/indicator/crt/ICIR.h"
#include "SingleFactor.h"

namespace hku {

SingleFactor::SingleFactor() {
    setParam<bool>("fill_null", true);
}

SingleFactor::SingleFactor(const Indicator& ind, const StockList& stks, const KQuery& query,
                           const Stock& ref_stk)
: m_ind(ind), m_stks(stks), m_ref_stk(ref_stk), m_query(query) {
    setParam<bool>("fill_null", true);
}

const Indicator& SingleFactor::get(const Stock& stk) const {
    const auto iter = m_stk_map.find(stk);
    HKU_CHECK(iter != m_stk_map.cend(), "Could not find this stock: {}", stk);
    return m_all_inds[iter->second];
}

const vector<std::pair<Stock, SingleFactor::value_t>>& SingleFactor::get(const Datetime& d) const {
    const auto iter = m_date_index.find(d);
    HKU_CHECK(iter != m_date_index.cend(), "Could not find this date: {}", d);
    return m_stk_factor_by_date[iter->second];
}

Indicator SingleFactor::getIC(int ndays) const {
    return IC(m_ind, m_stks, m_query, ndays, m_ref_stk);
}

Indicator SingleFactor::getICIR(int ic_n, int ir_n) const {
    return ICIR(IC(m_ind, m_stks, m_query, ic_n, m_ref_stk), ir_n);
}

void SingleFactor::calculate() {
    HKU_ERROR_IF_RETURN(m_ref_stk.isNull(), void(), "ref_stk is null!");
    auto ref_dates = m_ref_stk.getDatetimeList(m_query);
    size_t days_total = ref_dates.size();

    HKU_WARN_IF_RETURN(
      days_total < 2 || m_stks.size() < 2, void(),
      "The number(>=2) of stock or data length(>=2) is insufficient! current data len: {}, "
      "current stock number: {}",
      days_total, m_stks.size());

    bool fill_null = getParam<bool>("fill_null");

    size_t stk_count = m_stks.size();
    m_all_inds.reserve(stk_count);
    m_stk_map.reserve(stk_count);
    for (size_t i = 0; i < stk_count; i++) {
        const auto& stk = m_stks[i];
        HKU_WARN_IF(stk.isNull(), "Exist null stock!");
        auto k = stk.getKData(m_query);
        m_stk_map[stk] = i;
        m_all_inds.emplace_back(ALIGN(m_ind(k), ref_dates, fill_null));
    }

    m_stk_factor_by_date.resize(days_total);
    vector<std::pair<Stock, value_t>> one_day(stk_count);
    for (size_t i = 0; i < days_total; i++) {
        for (size_t j = 0; j < stk_count; j++) {
            one_day[i] = std::make_pair(m_stks[j], m_all_inds[j][i]);
        }
        std::sort(one_day.begin(), one_day.end(),
                  [](const std::pair<Stock, value_t>& a, const std::pair<Stock, value_t>& b) {
                      return a.second > b.second;
                  });
        m_stk_factor_by_date[i] = one_day;
        m_date_index[ref_dates[i]] = i;
    }
}

}  // namespace hku