/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-12
 *      Author: fasiondog
 */

#include <cmath>
#include "hikyuu/utilities/thread/algorithm.h"
#include "hikyuu/indicator/crt/ALIGN.h"
#include "hikyuu/indicator/crt/KDATA.h"
#include "hikyuu/indicator/crt/ROCP.h"
#include "hikyuu/indicator/crt/REF.h"
#include "hikyuu/indicator/crt/PRICELIST.h"
#include "hikyuu/indicator/crt/IC.h"
#include "hikyuu/indicator/crt/ICIR.h"
#include "hikyuu/indicator/crt/SPEARMAN.h"
#include "hikyuu/indicator/crt/CORR.h"
#include "hikyuu/indicator/crt/ZSCORE.h"
#include "hikyuu/StockManager.h"
#include "MultiFactorBase.h"
#include "industry_neutralize.h"
#include "StyleRegression.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& out, const MultiFactorBase& mf) {
    out << "MultiFactor{" << "\n  name: " << mf.name() << "\n  params: " << mf.getParameter()
        << "\n  query: " << mf.getQuery() << "\n  ref stock: " << mf.m_ref_stk;

    out << "\n  src inds count: " << mf.m_factorset.size() << " [";
    if (mf.m_factorset.size() <= 5) {
        for (const auto& ind : mf.m_factorset) {
            out << ind.name() << ", ";
        }
    } else {
        for (size_t i = 0; i < 5; i++) {
            out << mf.m_factorset[i].name() << ", ";
        }
        out << "......";
    }
    out << "]";

    out << "\n  normalize: " << (mf.m_norm ? mf.m_norm->name() : "NULL");

    if (!mf.m_special_norms.empty()) {
        out << "\n  special norms: " << mf.m_special_norms.size();
        for (const auto& [name, norm] : mf.m_special_norms) {
            out << "\n    " << name << " -> " << norm->name();
        }
    }

    if (!mf.m_special_category.empty()) {
        out << "\n  special catogory handle: " << mf.m_special_category.size();
        for (const auto& [name, category] : mf.m_special_category) {
            out << "\n    " << name << " -> " << category;
        }
    }

    if (!mf.m_special_style_inds.empty()) {
        out << "\n  special style inds: " << mf.m_special_style_inds.size();
        for (const auto& [name, style_inds] : mf.m_special_style_inds) {
            out << "\n    " << name << " -> [";
            for (const auto& ind : style_inds) {
                out << ind.name() << ", ";
            }
            out << "]";
        }
    }

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
  m_is_python_object(base.m_is_python_object),
  m_name(base.m_name),
  m_factorset(base.m_factorset),
  m_stks(base.m_stks),
  m_ref_stk(base.m_ref_stk),
  m_query(base.m_query) {
    if (base.m_norm) {
        m_norm = base.m_norm->clone();
    }
    for (const auto& [norm_name, norm] : base.m_special_norms) {
        m_special_norms[norm_name] = norm->clone();
    }
    m_special_category = base.m_special_category;
    for (const auto& [style_name, style_inds] : base.m_special_style_inds) {
        IndicatorList& dst = m_special_style_inds[style_name];
        dst.reserve(style_inds.size());
        for (const auto& ind : style_inds) {
            dst.emplace_back(ind.clone());
        }
    }
}

MultiFactorBase::MultiFactorBase(const StockList& stks, const KQuery& query, const Stock& ref_stk,
                                 const string& name, int ic_n, bool spearman, int mode,
                                 bool save_all_factors)
: m_name(name), m_stks(stks), m_ref_stk(ref_stk), m_query(query) {
    initParam();
    setParam<bool>("use_spearman", spearman);
    setParam<int>("ic_n", ic_n);
    setParam<int>("mode", mode);
    setParam<bool>("save_all_factors", save_all_factors);
    checkParam("ic_n");
    checkParam("mode");
    // _checkData();
}

void MultiFactorBase::initParam() {
    setParam<bool>("fill_null", true);
    setParam<int>("ic_n", 1);
    setParam<bool>("use_spearman", true);  // SPEARMAN is used by default, otherwise the pearson one
    setParam<int>("mode", 0);              // Sorting mode: 0-descending, 1-ascending, 2-no sorting
    setParam<bool>("save_all_factors", false);  // Keep all the factor data after the calculation
                                                // the getAllFactors/getFactor methods
}

void MultiFactorBase::baseCheckParam(const string& name) const {
    if ("ic_n" == name) {
        HKU_ASSERT(getParam<int>("ic_n") >= 1);
    } else if ("mode" == name) {
        int mode = getParam<int>("mode");
        HKU_ASSERT(mode == 0 || mode == 1 || mode == 2);
    }
}

void MultiFactorBase::paramChanged() {
    m_calculated.store(false, std::memory_order_relaxed);
}

void MultiFactorBase::_checkData() {
    HKU_CHECK(!m_factorset.empty(), "Input factor set is empty!");

    // The subsequent calculation needs to stay aligned and handling a mixed Null stock is
    // troublesome, so an exception is thrown to block it
    for (const auto& stk : m_stks) {
        HKU_CHECK(!stk.isNull(), "Exist null stock in stks!");
    }

    // Get the reference dates used for the alignment
    if (m_ref_stk.isNull()) {
        m_ref_stk = StockManager::instance().getMarketStock("SH");
    }
    m_ref_dates = m_ref_stk.getDatetimeList(m_query);
    HKU_CHECK(m_ref_dates.size() >= 2, "The dates len is insufficient! current len: {}",
              m_ref_dates.size());

    HKU_CHECK(m_stks.size() >= 2, "The number of stock is insufficient! current stock number: {}",
              m_stks.size());
}

void MultiFactorBase::clearCalculatedData() {
    m_ref_dates = {};
    m_stk_map = {};
    m_all_factors = {};
    m_date_index = {};
    m_stk_factor_by_date = {};
    m_ic = {};
}

void MultiFactorBase::reset() {
    // The lock is held throughout: to avoid a write-write interleaving with an ongoing calculate.
    // Note: _reset() is a virtual function and a custom implementation must not re-enter, inside
    // the lock, a method of the same instance that needs m_mutex.
    std::lock_guard<std::mutex> lock(m_mutex);
    _reset();
    clearCalculatedData();
    m_calculated.store(false, std::memory_order_release);
}

MultiFactorPtr MultiFactorBase::clone() {
    MultiFactorPtr p = _clone();
    HKU_CHECK(p && p.get() != this,
              "Failed clone! The subclass _clone of {} must return a new object!", m_name);

    p->m_name = m_name;
    p->m_params = m_params;
    p->m_is_python_object = m_is_python_object;
    p->m_stks = m_stks;
    p->m_ref_stk = m_ref_stk;
    // p->m_ref_dates = m_ref_dates;
    p->m_query = m_query;

    p->m_factorset = m_factorset;

    if (m_norm) {
        p->m_norm = m_norm->clone();
    }

    for (const auto& [norm_name, norm] : m_special_norms) {
        p->m_special_norms[norm_name] = norm->clone();
    }

    p->m_special_category = m_special_category;

    for (const auto& [style_name, style_inds] : m_special_style_inds) {
        IndicatorList& dst = p->m_special_style_inds[style_name];
        dst.reserve(style_inds.size());
        for (const auto& ind : style_inds) {
            dst.emplace_back(ind.clone());
        }
    }

    p->m_calculated.store(false, std::memory_order_relaxed);
    // Force a recalculation without cloning the following caches, to avoid the thread unsafety
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
    m_query = query;
    m_calculated.store(false, std::memory_order_relaxed);
}

void MultiFactorBase::setRefStock(const Stock& stk) {
    Stock tmp_stk = stk.isNull() ? StockManager::instance().getMarketStock("SH") : stk;
    DatetimeList ref_dates = tmp_stk.getDatetimeList(m_query);
    HKU_CHECK(ref_dates.size() >= 2, "The dates len is insufficient! current len: {}",
              ref_dates.size());
    m_ref_stk = tmp_stk;
    m_calculated.store(false, std::memory_order_relaxed);
}

void MultiFactorBase::setStockList(const StockList& stks) {
    // The subsequent calculation needs to stay aligned and handling a mixed Null stock is
    // troublesome, so an exception is thrown to block it
    for (const auto& stk : stks) {
        HKU_CHECK(!stk.isNull(), "Exist null stock in stks!");
    }

    m_stks = stks;
    m_calculated.store(false, std::memory_order_relaxed);
}

void MultiFactorBase::setRefFactorSet(const FactorSet& factorset) {
    HKU_CHECK(!factorset.isNull() && !factorset.empty(), "Input factor set is null or empty!");
    m_factorset = factorset;
    m_calculated.store(false, std::memory_order_relaxed);
}

void MultiFactorBase::setNormalize(NormPtr norm) {
    m_norm = norm;
    m_calculated.store(false, std::memory_order_relaxed);
}

void MultiFactorBase::addSpecialNormalize(const string& name, NormalizePtr norm,
                                          const string& category, const IndicatorList& style_inds) {
    // No special handling is given
    HKU_WARN_IF(!norm && category.empty() && style_inds.empty(),
                "No special handling is specified!");

    bool found = false;
    string found_name;
    for (const auto& ind : m_factorset) {
        // An indicator may use a Chinese name, but a Factor may not
        if (utf8_fold_equal(ind.name(), name)) {
            found = true;
            found_name = ind.name();
            break;
        }
    }
    HKU_CHECK(found, "Can't find factor ({}) in MF!", name);

    if (!category.empty()) {
        auto blks = StockManager::instance().getBlockList(category);
        HKU_CHECK(!blks.empty(), "Can't find category block list: {}", category);
    }

    if (norm) {
        m_special_norms[found_name] = norm;
    }

    if (!category.empty()) {
        m_special_category[found_name] = category;
    }

    if (!style_inds.empty()) {
        m_special_style_inds[found_name] = style_inds;
    }

    m_calculated.store(false, std::memory_order_relaxed);
}

const DatetimeList& MultiFactorBase::getDatetimeList() {
    calculate();
    return m_ref_dates;
}

const Indicator& MultiFactorBase::getFactor(const Stock& stk) {
    HKU_CHECK(getParam<bool>("save_all_factors"),
              "param \"save_all_factors\" is false, can't get all factors!");
    calculate();
    const auto iter = m_stk_map.find(stk);
    HKU_CHECK(iter != m_stk_map.cend(), "Could not find this stock: {}", stk);
    return m_all_factors[iter->second];
}

const IndicatorList& MultiFactorBase::getAllFactors() {
    HKU_CHECK(getParam<bool>("save_all_factors"),
              "param \"save_all_factors\" is false, can't get all factors!");
    calculate();
    return m_all_factors;
}

const ScoreRecordList& MultiFactorBase::getScoresRef(const Datetime& date) {
    calculate();
    const auto iter = m_date_index.find(date);
    if (iter != m_date_index.cend()) {
        return m_stk_factor_by_date[iter->second];
    }

    HKU_WARN("The date {} is not in the reference dates, returning an empty score list. Use a date "
             "from get_datetime_list().",
             date);
    static const ScoreRecordList empty_scores;
    return empty_scores;
}

ScoreRecordList MultiFactorBase::getScores(const Datetime& d) {
    return getScoresRef(d);
}

ScoreRecordList MultiFactorBase::getScores(const Datetime& date, size_t start, size_t end) {
    ScoreRecordList ret;
    HKU_IF_RETURN(start >= end, ret);

    const auto& cross = getScoresRef(date);
    if (end == Null<size_t>() || end > cross.size()) {
        end = cross.size();
    }
    HKU_IF_RETURN(start >= end, ret);

    ret.reserve(end - start);
    for (size_t i = start; i < end; i++) {
        ret.emplace_back(cross[i]);
    }

    return ret;
}

ScoreRecordList MultiFactorBase::getScores(const Datetime& date, size_t start, size_t end,
                                           std::function<bool(const ScoreRecord&)>&& filter) {
    ScoreRecordList ret;
    HKU_IF_RETURN(start >= end, ret);

    const auto& cross = getScoresRef(date);
    HKU_IF_RETURN(cross.empty(), ret);

    if (end == Null<size_t>() || end > cross.size()) {
        end = cross.size();
    }
    HKU_IF_RETURN(start >= end, ret);

    ret.reserve(end - start);
    for (size_t i = start; i < end; i++) {
        ret.emplace_back(cross[i]);
    }
    // copy the slice out first: the external filter callback may trigger a recalculation that would
    // otherwise leave cross dangling
    if (filter) {
        ScoreRecordList kept;
        kept.reserve(ret.size());
        for (const auto& r : ret) {
            if (filter(r)) {
                kept.emplace_back(r);
            }
        }
        ret = std::move(kept);
    }

    return ret;
}

ScoreRecordList MultiFactorBase::getScores(
  const Datetime& date, size_t start, size_t end,
  std::function<bool(const Datetime&, const ScoreRecord&)>&& filter) {
    ScoreRecordList ret;
    HKU_IF_RETURN(start >= end, ret);

    const auto& cross = getScoresRef(date);
    HKU_IF_RETURN(cross.empty(), ret);

    if (end == Null<size_t>() || end > cross.size()) {
        end = cross.size();
    }
    HKU_IF_RETURN(start >= end, ret);

    ret.reserve(end - start);
    for (size_t i = start; i < end; i++) {
        ret.emplace_back(cross[i]);
    }
    // copy the slice out first: the external filter callback may trigger a recalculation that would
    // otherwise leave cross dangling
    if (filter) {
        ScoreRecordList kept;
        kept.reserve(ret.size());
        for (const auto& r : ret) {
            if (filter(date, r)) {
                kept.emplace_back(r);
            }
        }
        ret = std::move(kept);
    }

    return ret;
}

ScoreRecordList MultiFactorBase::getScores(const Datetime& date, size_t start, size_t end,
                                           const ScoresFilterPtr& filter) {
    ScoreRecordList ret;
    HKU_IF_RETURN(start >= end, ret);

    const auto& cross = getScoresRef(date);
    HKU_IF_RETURN(cross.empty(), ret);

    if (end == Null<size_t>() || end > cross.size()) {
        end = cross.size();
    }
    HKU_IF_RETURN(start >= end, ret);

    if (!filter) {
        ret.reserve(end - start);
        for (size_t i = start; i < end; i++) {
            ret.emplace_back(cross[i]);
        }
        return ret;
    }

    // A pure C++ filter cannot re-enter and invalidate the cross-section, so a full-range filter
    // consumes the internal list directly and only copies the records it keeps; a Python filter
    // may trigger a recalculation, so the slice is copied out first to keep the reference valid
    if (start == 0 && end == cross.size() && !filter->hasPythonPart()) {
        return filter->filter(cross, date, m_query);
    }

    ret.reserve(end - start);
    for (size_t i = start; i < end; i++) {
        ret.emplace_back(cross[i]);
    }
    return filter->filter(ret, date, m_query);
}

const vector<ScoreRecordList>& MultiFactorBase::getAllScores() {
    calculate();
    return m_stk_factor_by_date;
}

Indicator MultiFactorBase::getIC(int ndays) {
    HKU_WARN_IF_RETURN(!getParam<bool>("save_all_factors"), Indicator(),
                       htr("mf param \"save_all_factors\" is false, can't get all factors!, please "
                           "set it to true if you want to get IC/ICIR!"));

    calculate();

    std::lock_guard<std::mutex> lock(m_mutex);

    // When ndays equals the ic_n parameter, the cached IC result is taken first
    // The IC of a new factor essentially needs no cache (such as the equal weight), but for a new
    // factor whose weight is calculated through IC or ICIR, although the IC of the new factor can
    // be calculated with a different N-day return, it is better to use the same value The weight is
    // often calculated through IC/ICIR, so a copy is cached here directly to reduce the repeated
    // calculation In the actual usage it is better to keep ndays in getIC(ndays) consistent with
    // ic_n
    int ic_n = getParam<int>("ic_n");
    if (ndays <= 0) {
        ndays = ic_n;
    }
    HKU_IF_RETURN(ic_n == ndays && !m_ic.empty(), m_ic.clone());

    auto all_returns = _getAllReturns(ndays);
    Indicator result = IC(m_all_factors, all_returns, ndays, getParam<bool>("use_spearman"));
    result.setParam<DatetimeList>("align_date_list", m_ref_dates);
    result.name("IC");

    // When ndays equals the ic_n parameter, cache the calculation result
    if (ic_n == ndays) {
        m_ic = result;
    }
    return result;
}

Indicator MultiFactorBase::getICIR(int ir_n, int ic_n) {
    HKU_WARN_IF_RETURN(ir_n < 1, Indicator(), htr("ir_n must be >= 1"));
    int effective_ic_n = ic_n > 0 ? ic_n : getParam<int>("ic_n");
    HKU_WARN_IF_RETURN(effective_ic_n < 1, Indicator(), htr("ic_n must be >= 1"));
    Indicator ic = getIC(effective_ic_n);
    Indicator x = (MA(ic, ir_n) / STDEV(ic, ir_n)).getResult(0);
    x.name("ICIR");
    x.setParam<int>("n", effective_ic_n);
    x.setParam<int>("rolling_n", ir_n);
    return x;
}

unordered_map<string, std::pair<PriceList, size_t>> MultiFactorBase::_buildDummyIndex() {
    // When the industry neutralization of a special indicator is given, build its industry
    // membership labels
    unordered_map<string, std::pair<PriceList, size_t>> stock_dummy_index;
    for (const auto& [ind_name, catefory] : m_special_category) {
        auto blks = StockManager::instance().getBlockList(catefory);
        if (blks.empty()) {
            HKU_WARN("Block list ({}) is empty, please check your block category!", catefory);
            continue;
        }

        auto& entry = stock_dummy_index[ind_name];
        size_t blk_count = blks.size();
        entry.second = blk_count;
        auto& dummy = entry.first;
        dummy.resize(m_stks.size(), Null<price_t>());

        for (size_t i = 0; i < m_stks.size(); i++) {
            bool found = false;
            size_t j = 0;
            for (const auto& blk : blks) {
                if (blk.have(m_stks[i])) {
                    dummy[i] = j;
                    found = true;
                    break;
                }
                j++;
            }
            if (!found) {
                // A stock without a membership is assigned blk_count and skipped when the group
                // mean is removed downstream (the residual is set to NaN), avoiding an implicit
                // clustering into a pseudo industry group that would cause a pseudo regression.
                dummy[i] = blk_count;
            }
        }
    }
    return stock_dummy_index;
}

IndicatorList MultiFactorBase::_getAllReturns(int ndays) const {
    bool fill_null = getParam<bool>("fill_null");
    return global_parallel_for_index(0, m_stks.size(), [this, ndays, fill_null](size_t i) {
        auto k = m_stks[i].getKData(m_query);
        return ALIGN(ROCP(CLOSE(), ndays), m_ref_dates, fill_null)(k).getResult(0);
    });
}

// The pure function implementation of the industry neutralization (removing the group mean by
// industry group) is in industry_neutralize.h; it is extracted as an internal inline header so that
// the white box unit tests can include and call it directly. The residual regression implementation
// of the style factor neutralization is in StyleRegression.cpp; it is extracted from this class as
// a serial kernel and no longer modifies the process level Eigen thread configuration at runtime.

vector<IndicatorList> MultiFactorBase::getAllSrcFactors() {
    vector<IndicatorList> all_stk_inds;
    size_t stk_count = m_stks.size();
    HKU_IF_RETURN(stk_count == 0, all_stk_inds);

    size_t ind_count = m_factorset.size();
    HKU_IF_RETURN(ind_count == 0, all_stk_inds);

    size_t days_total = m_ref_dates.size();

    bool fill_null = getParam<bool>("fill_null");

    all_stk_inds = m_factorset.getValues(m_stks, m_query, true, fill_null, true, true, m_ref_dates);

    // The style factors are stored in three dimensions [style factor name][style factor][stock]
    // (vector<IndicatorList>).
    //
    // The original implementation was two-dimensional [style factor name][style factor] and every
    // style factor shared the same time series across the whole market, which caused two
    // independent defects:
    //   1) during a concurrent extraction multiple threads wrote the same Indicator (the non-atomic
    //   assignment to the shared_ptr object itself is
    //      a C++ UB), which is the root cause of the segfault (no Python exception) of get_ic /
    //      get_all_src_factors after the style neutralization is enabled;
    //   2) the independent variable of the cross-sectional regression took the same value for all
    //   the stocks, degenerating into a constant column collinear with the intercept,
    //      so the market value / style neutralization failed completely in mathematics (the
    //      residual is about the original value).
    //
    // After the stock dimension is added:
    //   - Write isolation: every si thread writes per_factor[j][si] only and different si write
    //   different slots;
    //   - Read isolation: every thread first clone()s an independent copy of the template styles[j]
    //   and then calculates ALIGN on it,
    //     so calculate() writes the buffer of the thread copy only and does not touch the shared
    //     styles[j]. (IndicatorImp::clone() is strictly read-only on the root object, which was
    //     verified in the code, so concurrent
    //      styles[j].clone() is safe; here styles[j] is the root Indicator passed in by the user.)
    unordered_map<string, vector<IndicatorList>> use_style_inds;
    for (const auto& [style_ind_name, style_inds] : m_special_style_inds) {
        auto& per_factor = use_style_inds[style_ind_name];
        per_factor.resize(style_inds.size());
        for (auto& v : per_factor) {
            v.resize(stk_count);
        }
    }
    if (!m_special_style_inds.empty()) {
        auto null_ind = PRICELIST(PriceList(days_total, Null<price_t>()), m_ref_dates);
        global_parallel_for_index_void(0, stk_count, [&](size_t si) {
            const auto& stk = m_stks[si];
            auto kdata = stk.getKData(m_query);
            for (auto& [style_ind_name, styles] : m_special_style_inds) {
                auto pf_iter = use_style_inds.find(style_ind_name);
                auto& per_factor = pf_iter->second;
                for (size_t j = 0; j < styles.size(); j++) {
                    if (kdata.size() == 0) {
                        // A fresh clone per slot: sharing one Indicator would leak the name() below
                        // across the slots
                        per_factor[j][si] = null_ind.clone();
                    } else {
                        // An independent clone per thread: calculate writes the buffer of the
                        // thread copy only
                        per_factor[j][si] =
                          ALIGN(styles[j].clone(), m_ref_dates, fill_null)(kdata).getResult(0);
                    }
                    per_factor[j][si].name(style_ind_name);
                }
            }
        });
    }

    // The time cross-sectional standardization / normalization
    if (m_norm || !m_special_norms.empty() || !m_special_category.empty() ||
        !m_special_style_inds.empty()) {
        // The residual regression of the style factor neutralization has been extracted as a serial
        // kernel (StyleRegression.cpp), it no longer modifies the process level Eigen::setNbThreads
        // at runtime, avoiding the concurrent MFs polluting the global config mutually; the outer
        // day-by-day parallelism is naturally reentrant and the regression internals are all stack
        // local objects.
        unordered_map<string, std::pair<PriceList, size_t>> ind_dummy_dict = _buildDummyIndex();
        // Cache the raw factor buffers once (stk x ind) so the hot cross-sectional loop reads and
        // writes through the buffer instead of Indicator::operator[] (a virtual get per element)
        vector<value_t*> ind_data_ptrs(stk_count * ind_count);
        for (size_t si = 0; si < stk_count; si++) {
            for (size_t ii = 0; ii < ind_count; ii++) {
                ind_data_ptrs[si * ind_count + ii] = all_stk_inds[si][ii].data();
            }
        }

        // A Python normalizer's clone()/normalize() need the GIL, which the caller already holds
        // while spinning on the pool; run serially so the work stays on the calling thread.
        bool has_python_norm = (m_norm && m_norm->isPythonObject());
        if (!has_python_norm) {
            for (const auto& sn : m_special_norms) {
                if (sn.second && sn.second->isPythonObject()) {
                    has_python_norm = true;
                    break;
                }
            }
        }

        auto normalize_one_day = [this, stk_count, ind_count, &ind_dummy_dict, &use_style_inds,
                                  &ind_data_ptrs](size_t di) {
            auto sub_norm = m_norm ? m_norm->clone() : m_norm;
            NormPtr special_norm;
            PriceList one_day(stk_count, Null<price_t>());
            PriceList new_value;
            for (size_t ii = 0; ii < ind_count; ii++) {
                auto* one_day_data = one_day.data();
                for (size_t si = 0; si < stk_count; si++) {
                    one_day_data[si] = ind_data_ptrs[si * ind_count + ii][di];
                }

                // getValues wraps every factor in ALIGN, so the returned indicators are named
                // "ALIGN"; the original FactorSet name must be used as the lookup key, otherwise
                // the special norm / category lookups never hit.
                auto ind_name = m_factorset[ii].name();
                auto special_norm_iter = m_special_norms.find(ind_name);
                if (special_norm_iter != m_special_norms.end()) {
                    special_norm = special_norm_iter->second->clone();
                } else {
                    special_norm.reset();
                }

                if (special_norm) {
                    new_value = special_norm->normalize(one_day);
                } else if (sub_norm) {
                    new_value = sub_norm->normalize(one_day);
                } else {
                    new_value = one_day;
                }

                auto category_iter = ind_dummy_dict.find(ind_name);
                if (category_iter != ind_dummy_dict.end()) {
                    // Industry neutralization: removing the group mean, mathematically equivalent
                    // to a multiple regression without intercept on the one-hot encoding.
                    const auto& [labels, blk_count] = category_iter->second;
                    new_value = calculate_industry_residuals(new_value, labels, blk_count);
                }
                auto style_iter = use_style_inds.find(ind_name);
                if (style_iter != use_style_inds.end()) {
                    auto& per_factor = style_iter->second;  // [style factor][stock]
                    vector<PriceList> style_value_day(per_factor.size());
                    for (size_t j = 0; j < per_factor.size(); j++) {
                        auto& style_value = style_value_day[j];
                        style_value.resize(stk_count);
                        for (size_t si = 0; si < stk_count; si++) {
                            // Take the style factor value of the si-th stock itself (the original
                            // implementation wrongly took the shared value)
                            style_value[si] = per_factor[j][si][di];
                        }
                    }
                    new_value = calculate_style_residuals(new_value, style_value_day);
                }

                // normalize() is overridable (Python subclass forwarded via pybind); verify the
                // size before the writeback so a short result cannot write out of bounds
                HKU_CHECK(new_value.size() >= stk_count,
                          "The normalized result size ({}) of factor \"{}\" on date {} is less "
                          "than the stock count ({})!",
                          new_value.size(), ind_name, m_ref_dates[di], stk_count);
                for (size_t si = 0; si < stk_count; si++) {
                    ind_data_ptrs[si * ind_count + ii][di] = new_value[si];
                }
            }
        };

        if (has_python_norm) {
            for (size_t di = 0; di < days_total; ++di) {
                normalize_one_day(di);
            }
        } else {
            global_parallel_for_index_void(0, days_total, normalize_one_day);
        }
    }

    return all_stk_inds;
}

void MultiFactorBase::_buildIndex() {
    size_t stk_count = m_stks.size();
    for (size_t i = 0; i < stk_count; i++) {
        m_stk_map[m_stks[i]] = i;
    }

    size_t days_total = m_ref_dates.size();
    m_stk_factor_by_date.resize(days_total);
    for (size_t i = 0; i < days_total; i++) {
        m_date_index[m_ref_dates[i]] = i;
        m_stk_factor_by_date[i].resize(
          stk_count);  // Pre-allocate one slot per stock for every date
    }

    // Traverse the stock j first and then the date i: each thread reads one whole factor buffer
    // sequentially. (A date-first order was measured slower: it trades the per-date false sharing
    // for scattered reads across every factor buffer at one index, a worse trade.)
    global_parallel_for_index_void(0, stk_count, [this, days_total](size_t j) {
        const auto& stk = m_stks[j];
        const auto* data = m_all_factors[j].data();
        for (size_t i = 0; i < days_total; i++) {
            m_stk_factor_by_date[i][j] = ScoreRecord(stk, data[i]);
        }
    });

    // The deterministic comparator of the cross-sectional sorting: the valid values are sorted in
    // the target direction and, when the values are equal, the market_code lexicographic order is
    // used as the secondary key to break the tie, making the result stable across the input order,
    // the process, the platform and the compiler. NaN is placed at the end and the NaNs are also
    // ordered by the market_code lexicographic order to guarantee the determinism within the NaN
    // section. A lambda is used instead of an anonymous namespace function to avoid the ODR risk
    // under unity_build, and at the same time the closure type is unique at compile time, letting
    // std::sort fully inline the comparison logic.
    auto scoreDescLess = [](const ScoreRecord& a, const ScoreRecord& b) noexcept -> bool {
        const bool a_nan = std::isnan(a.value);
        const bool b_nan = std::isnan(b.value);
        if (a_nan != b_nan) {
            return !a_nan;  // A valid value always comes before NaN
        }
        if (!a_nan) {
            if (a.value > b.value)
                return true;
            if (a.value < b.value)
                return false;
        }
        return a.stock.market_code() < b.stock.market_code();
    };

    auto scoreAscLess = [](const ScoreRecord& a, const ScoreRecord& b) noexcept -> bool {
        const bool a_nan = std::isnan(a.value);
        const bool b_nan = std::isnan(b.value);
        if (a_nan != b_nan) {
            return !a_nan;  // A valid value always comes before NaN
        }
        if (!a_nan) {
            if (a.value < b.value)
                return true;
            if (a.value > b.value)
                return false;
        }
        return a.stock.market_code() < b.stock.market_code();
    };

    int mode = getParam<int>("mode");
    if (0 == mode) {
        global_parallel_for_index_void(
          0, days_total,
          [this, scoreDescLess](size_t i) {
              std::sort(m_stk_factor_by_date[i].begin(), m_stk_factor_by_date[i].end(),
                        scoreDescLess);
          },
          100);

    } else if (1 == mode) {
        global_parallel_for_index_void(
          0, days_total,
          [this, scoreAscLess](size_t i) {
              std::sort(m_stk_factor_by_date[i].begin(), m_stk_factor_by_date[i].end(),
                        scoreAscLess);
          },
          100);
    }
}

void MultiFactorBase::_validateCalculatedFactors() {
    const size_t stk_count = m_stks.size();
    const size_t days_total = m_ref_dates.size();
    HKU_CHECK(m_all_factors.size() == stk_count,
              "The factor count returned by _calculate ({}) does not match the stock count ({})!",
              m_all_factors.size(), stk_count);
    for (size_t i = 0; i < stk_count; i++) {
        const auto& ind = m_all_factors[i];
        HKU_CHECK(ind.data() != nullptr,
                  "The factor of the {}-th stock is null, please return a valid indicator!", i);
        HKU_CHECK(ind.size() >= days_total,
                  "The factor size of the {}-th stock ({}) is less than the reference date count "
                  "({})!",
                  i, ind.size(), days_total);
    }
}

bool MultiFactorBase::hasPythonPart() const noexcept {
    if (m_is_python_object || m_factorset.hasPythonPart() || (m_norm && m_norm->isPythonObject())) {
        return true;
    }
    for (const auto& item : m_special_norms) {
        if (item.second && item.second->isPythonObject()) {
            return true;
        }
    }
    for (const auto& item : m_special_style_inds) {
        for (const auto& ind : item.second) {
            if (ind.hasPythonPart()) {
                return true;
            }
        }
    }
    return false;
}

void MultiFactorBase::calculate() {
    // Fast path: check whether it is already Ready with a lock-free acquire
    if (m_calculated.load(std::memory_order_acquire)) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    // A second check inside the lock: the mutex already provides the slow path synchronization, so
    // relaxed is enough
    if (m_calculated.load(std::memory_order_relaxed)) {
        return;
    }

    // Clean up the old result before the building, ensuring the retry is based on a clean state
    clearCalculatedData();

    try {
        _checkData();

        {  // Get all the aligned original factors of all the securities
            vector<IndicatorList> all_stk_inds = getAllSrcFactors();

            size_t factor_count = m_factorset.size();
            if (factor_count == 1) {
                // Use the original factors directly
                size_t stk_count = m_stks.size();
                HKU_CHECK(
                  all_stk_inds.size() >= stk_count,
                  "The aligned factor list size ({}) is less than the stock count ({})!",
                  all_stk_inds.size(), stk_count);
                m_all_factors.resize(stk_count);
                for (size_t i = 0; i < stk_count; i++) {
                    HKU_CHECK(!all_stk_inds[i].empty(),
                              "The aligned factor list of the {}-th stock is empty!", i);
                    m_all_factors[i] = std::move(all_stk_inds[i][0]);
                }
            } else {
                // Calculate the adjusted composite factor of every security
                m_all_factors = _calculate(all_stk_inds);
            }

            _validateCalculatedFactors();
        }

        // Create the cross-sectional index after the calculation
        _buildIndex();

        if (!getParam<bool>("save_all_factors")) {
            m_all_factors = {};
            m_stk_map = {};
        }
    } catch (...) {
        // The failure cleanup: all the asynchronous subtasks have ended under the wait_and_drain
        // semantics, so the half-finished product of the base class is cleared, the not calculated
        // state is kept, the original exception propagates up and the next caller may retry.
        clearCalculatedData();
        m_calculated.store(false, std::memory_order_relaxed);
        throw;
    }

    // Publish: release guarantees that all the previous writes are visible to the subsequent
    // acquire reads
    m_calculated.store(true, std::memory_order_release);
}

}  // namespace hku