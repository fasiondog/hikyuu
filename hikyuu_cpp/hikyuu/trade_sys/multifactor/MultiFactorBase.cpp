/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-12
 *      Author: fasiondog
 */

#include <cmath>
#include "hikyuu/utilities/thread/algorithm.h"
#include "hikyuu/indicator/crt/ALIGN.h"
#include "hikyuu/indicator/crt/ROCP.h"
#include "hikyuu/indicator/crt/REF.h"
#include "hikyuu/indicator/crt/PRICELIST.h"
#include "hikyuu/indicator/crt/IC.h"
#include "hikyuu/indicator/crt/ICIR.h"
#include "hikyuu/indicator/crt/SPEARMAN.h"
#include "hikyuu/indicator/crt/CORR.h"
#include "hikyuu/indicator/crt/ZSCORE.h"
#include "MultiFactorBase.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& out, const MultiFactorBase& mf) {
    out << "MultiFactor{" << "\n  name: " << mf.name() << "\n  params: " << mf.getParameter()
        << "\n  query: " << mf.getQuery() << "\n  ref stock: " << mf.m_ref_stk;

    out << "\n  src inds count: " << mf.m_inds.size() << " [";
    if (mf.m_inds.size() <= 5) {
        for (const auto& ind : mf.m_inds) {
            out << ind.name() << ", ";
        }
    } else {
        for (size_t i = 0; i < 5; i++) {
            out << mf.m_inds[i].name() << ", ";
        }
        out << "......";
    }
    out << "]";

    out << "\n  stocks count: " << mf.m_stks.size() << " [";
    size_t print_stk_len = std::min<size_t>(5, mf.m_stks.size());
    for (size_t i = 0; i < print_stk_len; i++) {
        out << mf.m_stks[i].market_code() << ", ";
    }
    if (mf.m_stks.size() > 5) {
        out << "......";
    }
    out << "]";

    out << "\n}";
    return out;
}

HKU_API std::ostream& operator<<(std::ostream& out, const MultiFactorPtr& mf) {
    if (mf) {
        out << *mf;
    } else {
        out << "MultiFactor(NULL)";
    }
    return out;
}

MultiFactorBase::MultiFactorBase() : m_name("MultiFactorBase") {
    initParam();
}

MultiFactorBase::MultiFactorBase(const string& name) : m_name(name) {
    initParam();
}

MultiFactorBase::MultiFactorBase(const MultiFactorBase& base)
: m_params(base.m_params),
  m_name(base.m_name),
  m_inds(base.m_inds),
  m_stks(base.m_stks),
  m_ref_stk(base.m_ref_stk),
  m_query(base.m_query) {}

MultiFactorBase::MultiFactorBase(const IndicatorList& inds, const StockList& stks,
                                 const KQuery& query, const Stock& ref_stk, const string& name,
                                 int ic_n, bool spearman)
: m_name(name), m_inds(inds), m_stks(stks), m_ref_stk(ref_stk), m_query(query) {
    initParam();
    setParam<bool>("spearman", spearman);
    setParam<int>("ic_n", ic_n);
    checkParam("ic_n");
    _checkData();
}

void MultiFactorBase::initParam() {
    setParam<bool>("fill_null", true);
    setParam<int>("ic_n", 1);
    setParam<bool>("enable_min_max_normalize", false);
    setParam<bool>("enable_zscore", false);
    setParam<bool>("zscore_out_extreme", false);
    setParam<bool>("zscore_recursive", false);
    setParam<double>("zscore_nsigma", 3.0);
    setParam<bool>("use_spearman", true);  // 默认使用SPEARMAN计算相关系数, 否则使用pearson相关系数
}

void MultiFactorBase::baseCheckParam(const string& name) const {
    if ("ic_n" == name) {
        HKU_ASSERT(getParam<int>("ic_n") >= 1);
    } else if ("zscore_nsigma" == name) {
        HKU_ASSERT(getParam<double>("zscore_nsigma") > 0.0);
    }
}

void MultiFactorBase::paramChanged() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_calculated = false;
}

void MultiFactorBase::_checkData() {
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

void MultiFactorBase::reset() {
    std::lock_guard<std::mutex> lock(m_mutex);
    _reset();

    // 仅重置 m_calculated，其他缓存不重置，否则线程不安全
    m_calculated = false;
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
    p->m_ref_dates = m_ref_dates;

    p->m_inds.reserve(m_inds.size());
    for (const auto& ind : m_inds) {
        p->m_inds.emplace_back(ind.clone());
    }

    p->m_calculated = false;
    // 强制重算，不克隆以下缓存，避免非线程安全
    // p->m_stk_map = m_stk_map;
    // p->m_date_index = m_date_index;
    // p->m_stk_factor_by_date = m_stk_factor_by_date;
    // p->m_ic = m_ic.clone();
    // p->m_all_factors.reserve(m_all_factors.size());
    // for (const auto& ind : m_all_factors) {
    //     p->m_all_factors.emplace_back(ind.clone());
    // }
    return p;
}

void MultiFactorBase::setQuery(const KQuery& query) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_query = query;
    _reset();
    m_calculated = false;
}

void MultiFactorBase::setRefStock(const Stock& stk) {
    HKU_CHECK(!stk.isNull(), "The reference stock must be set!");
    DatetimeList ref_dates = stk.getDatetimeList(m_query);
    HKU_CHECK(ref_dates.size() >= 2, "The dates len is insufficient! current len: {}",
              ref_dates.size());
    std::lock_guard<std::mutex> lock(m_mutex);
    m_ref_stk = stk;
    m_ref_dates = std::move(ref_dates);
    _reset();
    m_calculated = false;
}

void MultiFactorBase::setStockList(const StockList& stks) {
    // 后续计算需要保持对齐，夹杂 Null stock 处理麻烦，直接抛出异常屏蔽
    for (const auto& stk : stks) {
        HKU_CHECK(!stk.isNull(), "Exist null stock in stks!");
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    m_stks = stks;
    _reset();
    m_calculated = false;
}

void MultiFactorBase::setRefIndicators(const IndicatorList& inds) {
    HKU_CHECK(!inds.empty(), "Input source factor list is empty!");
    std::lock_guard<std::mutex> lock(m_mutex);
    m_inds = inds;
    _reset();
    m_calculated = false;
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

ScoreRecordList MultiFactorBase::getScores(const Datetime& d) {
    calculate();
    ScoreRecordList ret;
    const auto iter = m_date_index.find(d);
    HKU_IF_RETURN(iter == m_date_index.cend(), ret);
    ret = m_stk_factor_by_date[iter->second];
    return ret;
}

ScoreRecordList MultiFactorBase::getScores(const Datetime& date, size_t start, size_t end) {
    ScoreRecordList ret;
    HKU_IF_RETURN(start >= end, ret);

    const auto& cross = getScores(date);
    if (end == Null<size_t>() || end > cross.size()) {
        end = cross.size();
    }

    for (size_t i = start; i < end; i++) {
        ret.emplace_back(cross[i]);
    }

    return ret;
}

ScoreRecordList MultiFactorBase::getScores(const Datetime& date, size_t start, size_t end,
                                           std::function<bool(const ScoreRecord&)>&& filter) {
    ScoreRecordList ret;
    HKU_IF_RETURN(start >= end, ret);

    const auto& cross = getScores(date);
    HKU_IF_RETURN(cross.empty(), ret);

    if (end == Null<size_t>() || end > cross.size()) {
        end = cross.size();
    }

    if (filter) {
        for (size_t i = start; i < end; i++) {
            if (filter(cross[i])) {
                ret.emplace_back(cross[i]);
            }
        }
    } else {
        for (size_t i = start; i < end; i++) {
            ret.emplace_back(cross[i]);
        }
    }

    return ret;
}

ScoreRecordList MultiFactorBase::getScores(
  const Datetime& date, size_t start, size_t end,
  std::function<bool(const Datetime&, const ScoreRecord&)>&& filter) {
    ScoreRecordList ret;
    HKU_IF_RETURN(start >= end, ret);

    const auto& cross = getScores(date);
    HKU_IF_RETURN(cross.empty(), ret);

    if (end == Null<size_t>() || end > cross.size()) {
        end = cross.size();
    }

    if (filter) {
        for (size_t i = start; i < end; i++) {
            if (filter(date, cross[i])) {
                ret.emplace_back(cross[i]);
            }
        }
    } else {
        for (size_t i = start; i < end; i++) {
            ret.emplace_back(cross[i]);
        }
    }

    return ret;
}

const vector<ScoreRecordList>& MultiFactorBase::getAllScores() {
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
    if (ndays == 0) {
        ndays = ic_n;
    }
    HKU_IF_RETURN(ic_n == ndays && !m_ic.empty(), m_ic.clone());

    size_t days_total = m_ref_dates.size();
    Indicator result = PRICELIST(PriceList(days_total, Null<price_t>()));
    result.name("IC");
    if (ndays + 1 >= days_total || ndays < 0) {
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

    Indicator (*spearman)(const Indicator&, const Indicator&, int, bool) = hku::SPEARMAN;
    if (!getParam<bool>("use_spearman")) {
        spearman = hku::CORR;
    }

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
        auto ic = spearman(a, b, ind_count, true);
        dst[i] = ic[ic.size() - 1];
    }

    // 如果 ndays 和 ic_n 参数相同，缓存计算结果
    if (ic_n == ndays) {
        m_ic = result;
    }
    return result;
}

Indicator MultiFactorBase::getICIR(int ir_n, int ic_n) {
    Indicator ic = getIC(ic_n);
    Indicator x = MA(ic, ir_n) / STDEV(ic, ir_n);
    x.name("ICIR");
    x.setParam<int>("n", ic_n);
    x.setParam<int>("rolling_n", ir_n);
    return x;
}

IndicatorList MultiFactorBase::_getAllReturns(int ndays) const {
    bool fill_null = getParam<bool>("fill_null");
#if !MF_USE_MULTI_THREAD
    vector<Indicator> all_returns;
    all_returns.reserve(m_stks.size());
    for (const auto& stk : m_stks) {
        auto k = stk.getKData(m_query);
        all_returns.emplace_back(ALIGN(REF(ROCP(k.close(), ndays), ndays), m_ref_dates, fill_null));
    }
    return all_returns;
#else
    return parallel_for_index(0, m_stks.size(), [this, ndays, fill_null](size_t i) {
        auto k = m_stks[i].getKData(m_query);
        return ALIGN(REF(ROCP(k.close(), ndays), ndays), m_ref_dates, fill_null);
    });
#endif
}

vector<IndicatorList> MultiFactorBase::getAllSrcFactors() {
    vector<IndicatorList> all_stk_inds;
    size_t stk_count = m_stks.size();
    HKU_IF_RETURN(stk_count == 0, all_stk_inds);
    all_stk_inds.resize(stk_count);

    bool fill_null = getParam<bool>("fill_null");
    size_t ind_count = m_inds.size();
    for (size_t i = 0; i < stk_count; i++) {
        const auto& stk = m_stks[i];
        auto kdata = stk.getKData(m_query);
        auto& cur_stk_inds = all_stk_inds[i];
        cur_stk_inds.resize(ind_count);
        for (size_t j = 0; j < ind_count; j++) {
            cur_stk_inds[j] = ALIGN(m_inds[j](kdata), m_ref_dates, fill_null);
            cur_stk_inds[j].name(m_inds[j].name());
        }
    }

    // 每日截面归一化
    if (getParam<bool>("enable_min_max_normalize")) {
        for (size_t di = 0, days_total = m_ref_dates.size(); di < days_total; di++) {
            for (size_t ii = 0; ii < ind_count; ii++) {
                Indicator::value_t min_value = std::numeric_limits<Indicator::value_t>::max();
                Indicator::value_t max_value = std::numeric_limits<Indicator::value_t>::min();
                for (size_t si = 0; si < stk_count; si++) {
                    auto value = all_stk_inds[si][ii][di];
                    if (!std::isnan(value)) {
                        if (value > max_value) {
                            max_value = value;
                        } else if (value < min_value) {
                            min_value = value;
                        }
                    }
                }

                if (max_value == min_value ||
                    max_value == std::numeric_limits<Indicator::value_t>::max()) {
                    for (size_t si = 0; si < stk_count; si++) {
                        auto* dst = all_stk_inds[si][ii].data();
                        dst[di] = Null<Indicator::value_t>();
                    }
                } else {
                    Indicator::value_t diff = max_value - min_value;
                    for (size_t si = 0; si < stk_count; si++) {
                        auto* dst = all_stk_inds[si][ii].data();
                        dst[di] = (dst[di] - min_value) / diff;
                    }
                }
            }
        }
    }

    // 每日截面标准化
    if (getParam<bool>("enable_zscore")) {
        Indicator one_day = PRICELIST(PriceList(stk_count, Null<price_t>()));
        for (size_t di = 0, days_total = m_ref_dates.size(); di < days_total; di++) {
            for (size_t ii = 0; ii < ind_count; ii++) {
                auto* one_day_data = one_day.data();
                for (size_t si = 0; si < stk_count; si++) {
                    one_day_data[si] = all_stk_inds[si][ii][di];
                }

                auto new_value =
                  ZSCORE(one_day, getParam<bool>("zscore_out_extreme"),
                         getParam<double>("zscore_nsigma"), getParam<bool>("zscore_recursive"));

                for (size_t si = 0; si < stk_count; si++) {
                    auto* dst = all_stk_inds[si][ii].data();
                    dst[di] = new_value[si];
                }
            }
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
    ScoreRecordList one_day;
    for (size_t i = 0; i < days_total; i++) {
        one_day.resize(stk_count);
        for (size_t j = 0; j < stk_count; j++) {
            one_day[j] = ScoreRecord(m_stks[j], m_all_factors[j][i]);
        }
        std::sort(one_day.begin(), one_day.end(), [](const ScoreRecord& a, const ScoreRecord& b) {
            if (std::isnan(a.value) && std::isnan(b.value)) {
                return false;
            } else if (!std::isnan(a.value) && std::isnan(b.value)) {
                return true;
            } else if (std::isnan(a.value) && !std::isnan(b.value)) {
                return false;
            }
            return a.value > b.value;
        });
        m_stk_factor_by_date[i] = std::move(one_day);
        m_date_index[m_ref_dates[i]] = i;
    }
}

void MultiFactorBase::calculate() {
    // SPEND_TIME(MultiFactorBase_calculate);
    std::lock_guard<std::mutex> lock(m_mutex);
    HKU_IF_RETURN(m_calculated, void());

    _checkData();

    // 获取所有证券所有对齐后的原始因子
    vector<vector<Indicator>> all_stk_inds = getAllSrcFactors();

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