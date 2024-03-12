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
#include "hikyuu/indicator/crt/SPEARMAN.h"
#include "MultiFactor.h"

namespace hku {

MultiFactor::MultiFactor() {
    setParam<bool>("fill_null", true);
}

MultiFactor::MultiFactor(const vector<Indicator>& inds, const StockList& stks, const KQuery& query,
                         const Stock& ref_stk)
: m_inds(inds), m_stks(stks), m_ref_stk(ref_stk), m_query(query) {
    setParam<bool>("fill_null", true);
    setParam<int>("ic_n", 1);
    setParam<int>("ir_n", 10);
}

const Indicator& MultiFactor::get(const Stock& stk) const {
    const auto iter = m_stk_map.find(stk);
    HKU_CHECK(iter != m_stk_map.cend(), "Could not find this stock: {}", stk);
    return m_all_factors[iter->second];
}

const vector<std::pair<Stock, MultiFactor::value_t>>& MultiFactor::get(const Datetime& d) const {
    const auto iter = m_date_index.find(d);
    HKU_CHECK(iter != m_date_index.cend(), "Could not find this date: {}", d);
    return m_stk_factor_by_date[iter->second];
}

vector<Indicator> MultiFactor::_getAllReturns(int ndays) const {
    bool fill_null = getParam<bool>("fill_null");
    vector<Indicator> all_returns;
    all_returns.reserve(m_stks.size());
    for (const auto& stk : m_stks) {
        auto k = stk.getKData(m_query);
        all_returns.push_back(ALIGN(ROCP(k.close(), ndays), m_ref_dates, fill_null));
    }
    return all_returns;
}

Indicator MultiFactor::getIC(int ndays) const {
    size_t days_total = m_ref_dates.size();

    bool fill_null = getParam<bool>("fill_null");

    vector<Indicator> all_returns = _getAllReturns(ndays);

    size_t ind_count = m_all_factors.size();
    PriceList tmp(ind_count, Null<price_t>());
    PriceList tmp_return(ind_count, Null<price_t>());
    Indicator result = PRICELIST(PriceList(days_total, 0.0));
    auto* dst = result.data();
    for (size_t i = 0; i < days_total; i++) {
        for (size_t j = 0; j < ind_count; j++) {
            tmp[j] = m_all_factors[j][i];
            tmp_return[j] = all_returns[j][i];
        }
        auto a = PRICELIST(tmp);
        auto b = PRICELIST(tmp_return);
        auto ic = hku::SPEARMAN(a, b, ind_count);
        dst[i] = ic[ic.size() - 1];
    }
    return result;
}

Indicator MultiFactor::getICIR(int ic_n, int ir_n) const {
    return ICIR(getIC(ic_n), ir_n);
}

void MultiFactor::_calculateAllFactorsByEqualWeight() {
    size_t days_total = m_ref_dates.size();
    size_t stk_count = m_stks.size();
    size_t ind_count = m_inds.size();

    m_all_factors.resize(stk_count);
    for (size_t si = 0; si < stk_count; si++) {
        vector<value_t> sumByDate(days_total, 0.0);
        vector<size_t> countByDate(days_total, 0);

        for (size_t di = 0; di < days_total; di++) {
            for (size_t ii = 0; ii < ind_count; ii++) {
                auto value = m_all_stk_inds[si][ii][di];
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

        m_all_factors[si] = PRICELIST(sumByDate);
    }
}

void MultiFactor::_calculateAllFactorsByIC() {
    size_t days_total = m_ref_dates.size();
    size_t stk_count = m_stks.size();
    size_t ind_count = m_inds.size();

    bool fill_null = getParam<bool>("fill_null");
    int ic_n = getParam<int>("ic_n");

    m_all_factors.resize(stk_count);

    vector<Indicator> ic(ind_count);
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
                auto value = m_all_stk_inds[si][ii][di];
                new_values[di] = value * sumByDate[di];
            }
        }
        m_all_factors[si] = PRICELIST(new_values);
    }
}

void MultiFactor::_calculateAllFactorsByICIR() {
    size_t days_total = m_ref_dates.size();
    size_t stk_count = m_stks.size();
    size_t ind_count = m_inds.size();

    bool fill_null = getParam<bool>("fill_null");
    int ic_n = getParam<int>("ic_n");
    int ir_n = getParam<int>("ir_n");

    m_all_factors.resize(stk_count);

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
                auto value = m_all_stk_inds[si][ii][di];
                new_values[di] = value * sumByDate[di];
            }
        }
        m_all_factors[si] = PRICELIST(new_values);
    }
}

void MultiFactor::_alignAllInds() {
    size_t days_total = m_ref_dates.size();
    bool fill_null = getParam<bool>("fill_null");
    size_t stk_count = m_stks.size();
    size_t ind_count = m_inds.size();

    m_all_stk_inds.resize(stk_count);
    for (size_t i = 0; i < stk_count; i++) {
        const auto& stk = m_stks[i];
        auto kdata = stk.getKData(m_query);
        m_all_stk_inds[i].resize(ind_count);
        for (size_t j = 0; j < ind_count; j++) {
            m_all_stk_inds[i][j] = ALIGN(m_inds[j](kdata), m_ref_dates, fill_null);
        }
    }
}

void MultiFactor::calculate() {
    HKU_ERROR_IF_RETURN(m_ref_stk.isNull(), void(), "ref_stk is null!");
    auto m_ref_dates = m_ref_stk.getDatetimeList(m_query);
    size_t days_total = m_ref_dates.size();

    HKU_WARN_IF_RETURN(
      days_total < 2 || m_stks.size() < 2, void(),
      "The number(>=2) of stock or data length(>=2) is insufficient! current data len: {}, "
      "current stock number: {}",
      days_total, m_stks.size());

    bool fill_null = getParam<bool>("fill_null");

    size_t stk_count = m_stks.size();

    _alignAllInds();
    _calculateAllFactorsByEqualWeight();

    m_stk_factor_by_date.resize(days_total);
    vector<std::pair<Stock, value_t>> one_day(stk_count);
    for (size_t i = 0; i < days_total; i++) {
        for (size_t j = 0; j < stk_count; j++) {
            one_day[i] = std::make_pair(m_stks[j], m_all_factors[j][i]);
        }
        std::sort(one_day.begin(), one_day.end(),
                  [](const std::pair<Stock, value_t>& a, const std::pair<Stock, value_t>& b) {
                      return a.second > b.second;
                  });
        m_stk_factor_by_date[i] = one_day;
        m_date_index[m_ref_dates[i]] = i;
    }
}

}  // namespace hku