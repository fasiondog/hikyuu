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
#include "hikyuu/StockManager.h"
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
                                 int ic_n, bool spearman, int mode, bool save_all_factors)
: m_name(name), m_inds(inds), m_stks(stks), m_ref_stk(ref_stk), m_query(query) {
    initParam();
    setParam<bool>("use_spearman", spearman);
    setParam<int>("ic_n", ic_n);
    setParam<int>("mode", mode);
    setParam<bool>("save_all_factors", save_all_factors);
    checkParam("ic_n");
    checkParam("mode");
    _checkData();
}

void MultiFactorBase::initParam() {
    setParam<bool>("fill_null", true);
    setParam<int>("ic_n", 1);
    setParam<bool>("use_spearman", true);  // 默认使用SPEARMAN计算相关系数, 否则使用pearson相关系数
    setParam<bool>("parallel", true);
    setParam<int>("mode", 0);                   // 获取截面数据时排序模式: 0-降序, 1-升序, 2-不排序
    setParam<bool>("save_all_factors", false);  // 计算完后保留所有因子数据，否则将被清除，影响
                                                // getAllFactors/getFactor 方法

    setParam<string>("norm_type", "");  // zscore, min_max, quantile, quantile_uniform
    setParam<bool>("zscore_out_extreme", false);
    setParam<bool>("zscore_recursive", false);
    setParam<double>("zscore_nsigma", 3.0);
    setParam<double>("quantile_min", 0.01);
    setParam<double>("quantile_max", 0.99);
}

void MultiFactorBase::baseCheckParam(const string& name) const {
    if ("norm_type" == name) {
        string norm_type = getParam<string>("norm_type");
        HKU_ASSERT(norm_type == "" || norm_type == "min_max" || norm_type == "zscore" ||
                   norm_type == "quantile" || norm_type == "quantile_uniform");
    } else if ("ic_n" == name) {
        HKU_ASSERT(getParam<int>("ic_n") >= 1);
    } else if ("zscore_nsigma" == name) {
        HKU_ASSERT(getParam<double>("zscore_nsigma") > 0.0);
    } else if ("mode" == name) {
        int mode = getParam<int>("mode");
        HKU_ASSERT(mode == 0 || mode == 1 || mode == 2);
    } else if ("quantile_min" == name) {
        double quantile_min = getParam<double>("quantile_min");
        HKU_ASSERT(quantile_min > 0.0 && quantile_min < 1.0);
        if (haveParam("quantile_max")) {
            double quantile_max = getParam<double>("quantile_max");
            HKU_ASSERT(quantile_min < quantile_max);
        }
    } else if ("quantile_max" == name) {
        double quantile_max = getParam<double>("quantile_max");
        HKU_ASSERT(quantile_max > 0.0 && quantile_max < 1.0);
        if (haveParam("quantile_min")) {
            double quantile_min = getParam<double>("quantile_min");
            HKU_ASSERT(quantile_min < quantile_max);
        }
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

    p->m_name = m_name;
    p->m_params = m_params;
    p->m_stks = m_stks;
    p->m_ref_stk = m_ref_stk;
    p->m_query = m_query;
    p->m_ref_dates = m_ref_dates;

    p->m_inds.reserve(m_inds.size());
    for (const auto& ind : m_inds) {
        p->m_inds.emplace_back(ind.clone());
    }

    for (const auto& [name, norm] : m_special_norms) {
        p->m_special_norms[name] = norm->clone();
    }

    p->m_special_category = m_special_category;

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

void MultiFactorBase::addSpecialNormalize(const string& name, NormalizePtr norm,
                                          const string& category) {
    HKU_CHECK(norm, "The normalize pointer is null!");
    std::lock_guard<std::mutex> lock(m_mutex);
    _reset();

    if (!category.empty()) {
        auto blks = StockManager::instance().getBlockList(category);
        HKU_CHECK(!blks.empty(), "Can't find block category: {}", category);
        m_special_category[name] = category;
    }

    m_special_norms[name] = norm;
    m_calculated = false;
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
    if (getParam<bool>("save_all_factors")) {
        calculate();
    } else {
        HKU_WARN("param \"save_all_factors\" is false, can't get all factors!");
    }
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
    if (ndays <= 0) {
        ndays = ic_n;
    }
    HKU_IF_RETURN(ic_n == ndays && !m_ic.empty(), m_ic.clone());

    size_t days_total = m_ref_dates.size();
    Indicator result = PRICELIST(PriceList(days_total, Null<price_t>()));
    result.name("IC");

    IndicatorList tmp_ref_inds(m_all_factors.size());
    for (size_t i = 0; i < m_all_factors.size(); i++) {
        tmp_ref_inds[i] = REF(m_all_factors[i], ndays);
    }

    auto all_returns = _getAllReturns(ndays);

    size_t discard = ndays;
    size_t ind_count = m_all_factors.size();

    Indicator (*spearman)(const Indicator&, const Indicator&, int, bool) = hku::SPEARMAN;
    if (!getParam<bool>("use_spearman")) {
        spearman = hku::CORR;
    }

    PriceList tmp(ind_count, Null<price_t>());
    PriceList tmp_return(ind_count, Null<price_t>());
    auto* dst = result.data();
    for (size_t i = discard; i < days_total; i++) {
        for (size_t j = 0; j < ind_count; j++) {
            tmp[j] = tmp_ref_inds[j][i];
            tmp_return[j] = all_returns[j][i];
        }
        auto a = PRICELIST(tmp);
        auto b = PRICELIST(tmp_return);
        auto ic = spearman(a, b, ind_count, true);
        if (ic.size() > 0) {
            dst[i] = ic[ic.size() - 1];
        }
    }

    for (size_t i = discard; i < days_total; i++) {
        if (!std::isnan(dst[i])) {
            discard = i;
            break;
        }
    }
    result.setDiscard(discard);

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

unordered_map<string, PriceList> MultiFactorBase::_buildDummyIndex() {
    // 如果指定了特殊的指标的行业中性化处理，则构建其行业哑变量
    unordered_map<string, PriceList> stock_dummy_index;
    for (const auto& [ind_name, catefory] : m_special_category) {
        stock_dummy_index[ind_name] = PriceList(m_stks.size(), Null<price_t>());
        auto blks = StockManager::instance().getBlockList(catefory);
        if (blks.empty()) {
            HKU_WARN("Block list ({}) is empty, please check your block category!", catefory);
            continue;
        }

        auto iter = stock_dummy_index.find(ind_name);
        auto& dummy = iter->second;

        size_t blk_count = blks.size();
        for (size_t i = 0; i < m_stks.size(); i++) {
            bool found = false;
            for (size_t j = 0; j < blk_count; i++) {
                if (blks[j].have(m_stks[i])) {
                    dummy[i] = j;
                    found = true;
                    break;
                }
            }
            if (!found) {
                dummy[i] = blk_count;
            }
        }
    }
    return stock_dummy_index;
}

IndicatorList MultiFactorBase::_getAllReturns(int ndays) const {
    bool fill_null = getParam<bool>("fill_null");
    if (!getParam<bool>("parallel")) {
        vector<Indicator> all_returns;
        all_returns.reserve(m_stks.size());
        for (const auto& stk : m_stks) {
            auto k = stk.getKData(m_query);
            all_returns.emplace_back(ALIGN(ROCP(k.close(), ndays), m_ref_dates, fill_null));
        }
        return all_returns;
    } else {
        return parallel_for_index(0, m_stks.size(), [this, ndays, fill_null](size_t i) {
            auto k = m_stks[i].getKData(m_query);
            return ALIGN(ROCP(k.close(), ndays), m_ref_dates, fill_null);
        });
    }
}

// 计算中性化后的因子，y 为因子，x 为行业哑变量
static PriceList calculate_residuals(const PriceList& y, const PriceList& x) {
    HKU_ASSERT(y.size() == x.size());

    const size_t n = x.size();
    PriceList residuals(n, Null<price_t>());

    // 计算核心求和项（仅需Σxy和Σx²）
    double sum_xy = 0.0, sum_x2 = 0.0;
    for (size_t i = 0; i < n; ++i) {
        if (std::isnan(x[i]) || std::isinf(x[i]) || std::isnan(y[i]) || std::isinf(y[i])) {
            continue;
        }
        sum_xy += x[i] * y[i];
        sum_x2 += x[i] * x[i];
    }

    // 分母不能为0（避免x全为0的无效数据）
    if (std::isnan(sum_x2) || std::isinf(sum_x2) || sum_x2 == 0) {
        return residuals;
    }

    // 计算斜率β₁（无截距项）
    double beta1 = sum_xy / sum_x2;
    if (std::isnan(beta1) || std::isinf(beta1)) {
        return residuals;
    }

    PriceList y_hat(x.size());
    for (size_t i = 0; i < x.size(); ++i) {
        if (std::isnan(x[i]) || std::isinf(x[i]) || std::isnan(y[i]) || std::isinf(y[i])) {
            continue;
        }
        y_hat[i] = beta1 * x[i];         // 拟合值 = β₁x（无截距）
        residuals[i] = y[i] - y_hat[i];  // 残差 = 观测值 - 拟合值
    }

    return residuals;
}

vector<IndicatorList> MultiFactorBase::getAllSrcFactors() {
    vector<IndicatorList> all_stk_inds;
    size_t stk_count = m_stks.size();
    HKU_IF_RETURN(stk_count == 0, all_stk_inds);
    all_stk_inds.resize(stk_count);

    size_t days_total = m_ref_dates.size();
    auto null_ind = PRICELIST(PriceList(days_total, Null<price_t>()), m_ref_dates);

    bool fill_null = getParam<bool>("fill_null");
    size_t ind_count = m_inds.size();

    bool parallel = getParam<bool>("parallel");
    if (parallel) {
        parallel_for_index_void(
          0, stk_count, [this, &all_stk_inds, &null_ind, &ind_count, &fill_null](size_t i) {
              const auto& stk = m_stks[i];
              auto kdata = stk.getKData(m_query);
              auto& cur_stk_inds = all_stk_inds[i];
              cur_stk_inds.resize(ind_count);
              for (size_t j = 0; j < ind_count; j++) {
                  if (kdata.size() == 0) {
                      cur_stk_inds[j] = null_ind;
                  } else {
                      cur_stk_inds[j] = ALIGN(m_inds[j](kdata), m_ref_dates, fill_null);
                  }
                  cur_stk_inds[j].name(m_inds[j].name());
              }
          });

    } else {
        for (size_t i = 0; i < stk_count; i++) {
            const auto& stk = m_stks[i];
            auto kdata = stk.getKData(m_query);
            auto& cur_stk_inds = all_stk_inds[i];
            cur_stk_inds.resize(ind_count);
            for (size_t j = 0; j < ind_count; j++) {
                if (kdata.size() == 0) {
                    cur_stk_inds[j] = null_ind;
                } else {
                    cur_stk_inds[j] = ALIGN(m_inds[j](kdata), m_ref_dates, fill_null);
                }
                cur_stk_inds[j].name(m_inds[j].name());
            }
        }
    }

    // 时间截面标准化
    NormalizePtr norm;
    string norm_type = getParam<string>("norm_type");
    if ("min_max" == norm_type) {
        norm = NORM_MIN_MAX();
    } else if ("zscore" == norm_type) {
        norm = NORM_ZSCORE(getParam<bool>("zscore_out_extreme"), getParam<double>("zscore_nsigma"),
                           getParam<bool>("zscore_recursive"));
    } else if ("quantile" == norm_type) {
        norm = NORM_QUANTILE(getParam<double>("quantile_min"), getParam<double>("quantile_max"));

    } else if ("quantile_uniform" == norm_type) {
        norm =
          NORM_QUANTILE_UNIFORM(getParam<double>("quantile_min"), getParam<double>("quantile_max"));
    }

    if (norm) {
        unordered_map<string, PriceList> ind_dummy_dict = _buildDummyIndex();
        if (parallel) {
            parallel_for_index_void(
              0, days_total,
              [this, stk_count, ind_count, sub_norm = norm->clone(), &all_stk_inds,
               &ind_dummy_dict](size_t di) {
                  NormPtr special_norm;
                  PriceList one_day(stk_count, Null<price_t>());
                  PriceList new_value;
                  for (size_t ii = 0; ii < ind_count; ii++) {
                      auto* one_day_data = one_day.data();
                      for (size_t si = 0; si < stk_count; si++) {
                          one_day_data[si] = all_stk_inds[si][ii][di];
                      }

                      auto special_norm_iter = m_special_norms.find(all_stk_inds[0][ii].name());
                      if (special_norm_iter != m_special_norms.end()) {
                          special_norm = special_norm_iter->second->clone();
                      } else {
                          special_norm.reset();
                      }

                      if (special_norm) {
                          new_value = special_norm->normalize(one_day);
                          auto category_iter = ind_dummy_dict.find(special_norm_iter->first);
                          if (category_iter != ind_dummy_dict.end()) {
                              new_value = calculate_residuals(new_value, category_iter->second);
                          }
                      } else {
                          new_value = sub_norm->normalize(one_day);
                      }

                      for (size_t si = 0; si < stk_count; si++) {
                          auto* dst = all_stk_inds[si][ii].data();
                          dst[di] = new_value[si];
                      }
                  }
              });
        } else {
            NormPtr special_norm;
            PriceList new_value;
            PriceList one_day(stk_count, Null<price_t>());
            for (size_t di = 0; di < days_total; di++) {
                for (size_t ii = 0; ii < ind_count; ii++) {
                    auto* one_day_data = one_day.data();
                    for (size_t si = 0; si < stk_count; si++) {
                        one_day_data[si] = all_stk_inds[si][ii][di];
                    }

                    auto special_norm_iter = m_special_norms.find(all_stk_inds[0][ii].name());
                    if (special_norm_iter != m_special_norms.end()) {
                        special_norm = special_norm_iter->second;
                    } else {
                        special_norm.reset();
                    }

                    if (special_norm) {
                        new_value = special_norm->normalize(one_day);
                        auto category_iter = ind_dummy_dict.find(special_norm_iter->first);
                        if (category_iter != ind_dummy_dict.end()) {
                            new_value = calculate_residuals(new_value, category_iter->second);
                        }
                    } else {
                        new_value = norm->normalize(one_day);
                    }

                    new_value = norm->normalize(one_day);
                    for (size_t si = 0; si < stk_count; si++) {
                        auto* dst = all_stk_inds[si][ii].data();
                        dst[di] = new_value[si];
                    }
                }
            }
        }
    }

    return all_stk_inds;
}

void MultiFactorBase::_buildIndex() {
    int mode = getParam<int>("mode");
    if (0 == mode) {
        _buildIndexDesc();
    } else if (1 == mode) {
        _buildIndexAsc();
    } else {
        _buildIndexNone();
    }
}

void MultiFactorBase::_buildIndexDesc() {
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

void MultiFactorBase::_buildIndexAsc() {
    size_t stk_count = m_stks.size();
    HKU_ASSERT(stk_count == m_all_factors.size());
    for (size_t i = 0; i < stk_count; i++) {
        m_stk_map[m_stks[i]] = i;
    }

    // 建立每日截面的索引，并每日升序排序
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
            return a.value < b.value;
        });
        m_stk_factor_by_date[i] = std::move(one_day);
        m_date_index[m_ref_dates[i]] = i;
    }
}

void MultiFactorBase::_buildIndexNone() {
    size_t stk_count = m_stks.size();
    HKU_ASSERT(stk_count == m_all_factors.size());
    for (size_t i = 0; i < stk_count; i++) {
        m_stk_map[m_stks[i]] = i;
    }

    // 建立每日截面的索引，不排序
    size_t days_total = m_ref_dates.size();
    m_stk_factor_by_date.resize(days_total);
    ScoreRecordList one_day;
    for (size_t i = 0; i < days_total; i++) {
        one_day.resize(stk_count);
        for (size_t j = 0; j < stk_count; j++) {
            one_day[j] = ScoreRecord(m_stks[j], m_all_factors[j][i]);
        }
        m_stk_factor_by_date[i] = std::move(one_day);
        m_date_index[m_ref_dates[i]] = i;
    }
}

void MultiFactorBase::calculate() {
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

    if (!getParam<bool>("save_all_factors")) {
        IndicatorList().swap(m_all_factors);
        unordered_map<Stock, size_t>().swap(m_stk_map);
    }

    // 更新计算状态
    m_calculated = true;
}

}  // namespace hku