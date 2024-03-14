/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-12
 *      Author: fasiondog
 */

#include "hikyuu/indicator/crt/ALIGN.h"
#include "hikyuu/indicator/crt/ROCP.h"
#include "hikyuu/indicator/crt/REF.h"
#include "hikyuu/indicator/crt/PRICELIST.h"
#include "hikyuu/indicator/crt/IC.h"
#include "hikyuu/indicator/crt/ICIR.h"
#include "hikyuu/indicator/crt/SPEARMAN.h"
#include "MultiFactorBase.h"

namespace hku {

MultiFactorBase::MultiFactorBase() {
    setParam<bool>("fill_null", true);
    setParam<int>("ic_n", 1);
}

MultiFactorBase::MultiFactorBase(const string& name) {
    setParam<bool>("fill_null", true);
    setParam<int>("ic_n", 1);
}

MultiFactorBase::MultiFactorBase(const IndicatorList& inds, const StockList& stks,
                                 const KQuery& query, const Stock& ref_stk, const string& name,
                                 int ic_n)
: m_name(name), m_inds(inds), m_stks(stks), m_ref_stk(ref_stk), m_query(query) {
    setParam<bool>("fill_null", true);
    setParam<int>("ic_n", ic_n);

    HKU_CHECK(!m_ref_stk.isNull(), "The reference stock must be set!");
    HKU_CHECK(!m_inds.empty(), "Input source factor list is empty!");

    // 后续计算需要保持对齐，夹杂 Null stock 处理麻烦，直接抛出异常屏蔽
    for (const auto& stk : m_stks) {
        HKU_CHECK(!stk.isNull(), "Exist null stock in stks!");
    }

    // 获取用于对齐的参考日期
    m_ref_dates = m_ref_stk.getDatetimeList(m_query);
    HKU_CHECK(m_ref_dates.size() >= 2, "The dates len is insufficient! current len: {}",
              m_ref_dates.size());

    HKU_CHECK(m_stks.size() >= 2, "The number of stock is insufficient! current stock number: {}",
              m_stks.size());
}

MultiFactorPtr MultiFactorBase::clone() {
    std::lock_guard<std::mutex> lock(m_mutex);
    MultiFactorPtr p;
    try {
        p = _clone();
    } catch (...) {
        HKU_ERROR("Subclass _clone failed!");
        p = MultiFactorPtr();
    }

    if (!p || p.get() == this) {
        HKU_ERROR("Failed clone! Will use self-ptr!");
        return shared_from_this();
    }

    p->m_params = m_params;
    p->m_stks = m_stks;
    p->m_ref_stk = m_ref_stk;
    p->m_query = m_query;
    p->m_stk_map = m_stk_map;
    p->m_all_factors = m_all_factors;
    p->m_date_index = m_date_index;
    p->m_stk_factor_by_date = m_stk_factor_by_date;
    p->m_ref_dates = m_ref_dates;
    p->m_ic = m_ic.clone();
    p->m_calculated = m_calculated;

    p->m_inds.reserve(m_inds.size());
    for (const auto& ind : m_inds) {
        p->m_inds.emplace_back(ind.clone());
    }

    p->m_all_factors.reserve(m_all_factors.size());
    for (const auto& ind : m_all_factors) {
        p->m_all_factors.emplace_back(ind.clone());
    }
    return p;
}

const Indicator& MultiFactorBase::getFactor(const Stock& stk) {
    calculate();
    const auto iter = m_stk_map.find(stk);
    HKU_CHECK(iter != m_stk_map.cend(), "Could not find this stock: {}", stk);
    return m_all_factors[iter->second];
}

const IndicatorList& MultiFactorBase::getAllFactors() {
    calculate();
    return m_all_factors;
}

const vector<std::pair<Stock, MultiFactorBase::value_t>>& MultiFactorBase::getCross(
  const Datetime& d) {
    calculate();
    const auto iter = m_date_index.find(d);
    HKU_CHECK(iter != m_date_index.cend(), "Could not find this date: {}", d);
    return m_stk_factor_by_date[iter->second];
}

const vector<vector<std::pair<Stock, MultiFactorBase::value_t>>>& MultiFactorBase::getAllCross() {
    calculate();
    return m_stk_factor_by_date;
}

Indicator MultiFactorBase::getIC(int ndays) {
    calculate();
    std::lock_guard<std::mutex> lock(m_mutex);

    // 如果 ndays 和 ic_n 参数相同，优先取缓存的 ic 结果
    // 新的因子计算 IC，本质上不需要缓存（如等权重），但通过 IC 或 ICIR 计算权重的新因子
    // 虽然可以用不同的 N 日收益率来计算新因子IC，但最好还是使用相同值
    // 通过IC/ICIR计算权重的情况较多，所以这里直接缓存一份，减少重复计算
    // 实际使用时，最好保证 getIC(ndays) 中的 ndays 和 ic_n 一致
    int ic_n = getParam<int>("ic_n");
    HKU_IF_RETURN(ic_n == ndays && !m_ic.empty(), m_ic.clone());

    size_t days_total = m_ref_dates.size();
    Indicator result = PRICELIST(PriceList(days_total, Null<price_t>()));
    result.name("IC");
    if (ndays + 1 >= days_total) {
        result.setDiscard(days_total);
        if (ic_n == ndays) {
            m_ic = result;
        }
        return result;
    }

    auto all_returns = _getAllReturns(ndays);

    size_t discard = ndays;
    size_t ind_count = m_all_factors.size();
    for (size_t i = 0; i < ind_count; i++) {
        if (all_returns[i].discard() > discard) {
            discard = all_returns[i].discard();
        }
        if (m_all_factors[i].discard() > discard) {
            discard = m_all_factors[i].discard();
        }
    }

    if (discard >= days_total) {
        result.setDiscard(days_total);
        if (ic_n == ndays) {
            m_ic = result;
        }
        return result;
    }

    result.setDiscard(discard);

    PriceList tmp(ind_count, Null<price_t>());
    PriceList tmp_return(ind_count, Null<price_t>());
    auto* dst = result.data();
    for (size_t i = discard; i < days_total; i++) {
        for (size_t j = 0; j < ind_count; j++) {
            tmp[j] = m_all_factors[j][i];
            tmp_return[j] = all_returns[j][i];
        }
        auto a = PRICELIST(tmp);
        auto b = PRICELIST(tmp_return);
        auto ic = hku::SPEARMAN(a, b, ind_count);
        dst[i] = ic[ic.size() - 1];
    }

    // 如果 ndays 和 ic_n 参数相同，缓存计算结果
    if (ic_n == ndays) {
        m_ic = result;
    }
    return result;
}

Indicator MultiFactorBase::getICIR(int ic_n, int ir_n) {
    return ICIR(getIC(ic_n), ir_n);
}

IndicatorList MultiFactorBase::_getAllReturns(int ndays) const {
    bool fill_null = getParam<bool>("fill_null");
    vector<Indicator> all_returns;
    all_returns.reserve(m_stks.size());
    for (const auto& stk : m_stks) {
        auto k = stk.getKData(m_query);
        all_returns.push_back(ALIGN(REF(ROCP(k.close(), ndays), ndays), m_ref_dates, fill_null));
    }
    return all_returns;
}

vector<IndicatorList> MultiFactorBase::_alignAllInds() {
    bool fill_null = getParam<bool>("fill_null");
    size_t stk_count = m_stks.size();
    size_t ind_count = m_inds.size();

    vector<IndicatorList> all_stk_inds;
    all_stk_inds.resize(stk_count);
    for (size_t i = 0; i < stk_count; i++) {
        const auto& stk = m_stks[i];
        auto kdata = stk.getKData(m_query);
        all_stk_inds[i].resize(ind_count);
        for (size_t j = 0; j < ind_count; j++) {
            all_stk_inds[i][j] = ALIGN(m_inds[j](kdata), m_ref_dates, fill_null);
        }
    }

    return all_stk_inds;
}

void MultiFactorBase::_buildIndex() {
    size_t stk_count = m_stks.size();
    HKU_ASSERT(stk_count == m_all_factors.size());
    for (size_t i = 0; i < stk_count; i++) {
        m_stk_map[m_stks[i]] = i;
    }

    // 建立每日截面的索引，并每日降序排序
    size_t days_total = m_ref_dates.size();
    m_stk_factor_by_date.resize(days_total);
    vector<std::pair<Stock, value_t>> one_day;
    for (size_t i = 0; i < days_total; i++) {
        one_day.resize(stk_count);
        for (size_t j = 0; j < stk_count; j++) {
            one_day[j] = std::make_pair(m_stks[j], m_all_factors[j][i]);
        }
        std::sort(one_day.begin(), one_day.end(),
                  [](const std::pair<Stock, value_t>& a, const std::pair<Stock, value_t>& b) {
                      if (std::isnan(a.second) && std::isnan(b.second)) {
                          return false;
                      } else if (!std::isnan(a.second) && std::isnan(b.second)) {
                          return true;
                      } else if (std::isnan(a.second) && !std::isnan(b.second)) {
                          return false;
                      }
                      return a.second > b.second;
                  });
        m_stk_factor_by_date[i] = std::move(one_day);
        m_date_index[m_ref_dates[i]] = i;
    }
}

void MultiFactorBase::calculate() {
    SPEND_TIME(MultiFactorBase_calculate);
    std::lock_guard<std::mutex> lock(m_mutex);
    HKU_IF_RETURN(m_calculated, void());

    // 获取所有证券所有对齐后的原始因子
    vector<vector<Indicator>> all_stk_inds = _alignAllInds();

    try {
        if (m_inds.size() == 1) {
            // 直接使用原始因子
            size_t stk_count = m_stks.size();
            m_all_factors.resize(stk_count);
            for (size_t i = 0; i < stk_count; i++) {
                m_all_factors[i] = std::move(all_stk_inds[i][0]);
            }

        } else {
            // 计算每支证券调整后的合成因子
            m_all_factors = _calculate(all_stk_inds);
        }

        // 计算完成后创建截面索引
        _buildIndex();

    } catch (const std::exception& e) {
        HKU_ERROR(e.what());
    } catch (...) {
        HKU_ERROR_UNKNOWN;
    }

    // 更新计算状态
    m_calculated = true;
}

}  // namespace hku