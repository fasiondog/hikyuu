/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-12
 *      Author: fasiondog
 */

#include <cmath>
#include <Eigen/Dense>
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
    setParam<int>("mode", 0);              // 获取截面数据时排序模式: 0-降序, 1-升序, 2-不排序
    setParam<bool>("save_all_factors", false);  // 计算完后保留所有因子数据，否则将被清除，影响
                                                // getAllFactors/getFactor 方法
}

void MultiFactorBase::baseCheckParam(const string& name) const {
    if ("ic_n" == name) {
        HKU_ASSERT(getParam<int>("ic_n") >= 1);
    } else if ("zscore_nsigma" == name) {
        HKU_ASSERT(getParam<double>("zscore_nsigma") > 0.0);
    } else if ("mode" == name) {
        int mode = getParam<int>("mode");
        HKU_ASSERT(mode == 0 || mode == 1 || mode == 2);
    }
}

void MultiFactorBase::paramChanged() {
    m_calculated = false;
}

void MultiFactorBase::_checkData() {
    HKU_CHECK(!m_inds.empty(), "Input source factor list is empty!");

    // 后续计算需要保持对齐，夹杂 Null stock 处理麻烦，直接抛出异常屏蔽
    for (const auto& stk : m_stks) {
        HKU_CHECK(!stk.isNull(), "Exist null stock in stks!");
    }

    // 获取用于对齐的参考日期
    if (m_ref_stk.isNull()) {
        m_ref_stk = StockManager::instance().getMarketStock("SH");
    }
    m_ref_dates = m_ref_stk.getDatetimeList(m_query);
    HKU_CHECK(m_ref_dates.size() >= 2, "The dates len is insufficient! current len: {}",
              m_ref_dates.size());

    HKU_CHECK(m_stks.size() >= 2, "The number of stock is insufficient! current stock number: {}",
              m_stks.size());
}

void MultiFactorBase::reset() {
    _reset();

    std::lock_guard<std::mutex> lock(m_mutex);
    m_ref_dates = {};
    m_stk_map = {};
    m_all_factors = {};
    m_date_index = {};
    m_stk_factor_by_date = {};
    m_ic = {};
    m_calculated = false;
}

MultiFactorPtr MultiFactorBase::clone() {
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
    p->m_is_python_object = m_is_python_object;
    p->m_stks = m_stks;
    p->m_ref_stk = m_ref_stk;
    // p->m_ref_dates = m_ref_dates;
    p->m_query = m_query;

    p->m_inds.reserve(m_inds.size());
    for (const auto& ind : m_inds) {
        p->m_inds.emplace_back(ind.clone());
    }

    if (m_norm) {
        p->m_norm = m_norm->clone();
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
    m_query = query;
    m_calculated = false;
}

void MultiFactorBase::setRefStock(const Stock& stk) {
    HKU_CHECK(!stk.isNull(), "The reference stock must be set!");
    DatetimeList ref_dates = stk.getDatetimeList(m_query);
    HKU_CHECK(ref_dates.size() >= 2, "The dates len is insufficient! current len: {}",
              ref_dates.size());
    m_ref_stk = stk;
    m_calculated = false;
}

void MultiFactorBase::setStockList(const StockList& stks) {
    // 后续计算需要保持对齐，夹杂 Null stock 处理麻烦，直接抛出异常屏蔽
    for (const auto& stk : stks) {
        HKU_CHECK(!stk.isNull(), "Exist null stock in stks!");
    }

    m_stks = stks;
    m_calculated = false;
}

void MultiFactorBase::setRefIndicators(const IndicatorList& inds) {
    HKU_CHECK(!inds.empty(), "Input source factor list is empty!");
    m_inds = inds;
    m_calculated = false;
}

void MultiFactorBase::setNormalize(NormPtr norm) {
    m_norm = norm;
    m_calculated = false;
}

void MultiFactorBase::addSpecialNormalize(const string& name, NormalizePtr norm,
                                          const string& category, const IndicatorList& style_inds) {
    // 未指定任何特殊处理
    HKU_WARN_IF(!norm && category.empty() && style_inds.empty(),
                "No special handling is specified!");

    bool found = false;
    for (const auto& ind : m_inds) {
        if (ind.name() == name) {
            found = true;
            break;
        }
    }
    HKU_CHECK(found, "Can't find factor: {}", name);

    if (!category.empty()) {
        auto blks = StockManager::instance().getBlockList(category);
        HKU_CHECK(!blks.empty(), "Can't find category block list: {}", category);
    }

    if (norm) {
        m_special_norms[name] = norm;
    }

    if (!category.empty()) {
        m_special_category[name] = category;
    }

    if (!style_inds.empty()) {
        m_special_style_inds[name] = style_inds;
    }

    m_calculated = false;
}

const DatetimeList& MultiFactorBase::getDatetimeList() {
    if (!m_calculated) {
        calculate();
    }
    return m_ref_dates;
}

const Indicator& MultiFactorBase::getFactor(const Stock& stk) {
    HKU_CHECK(getParam<bool>("save_all_factors"),
              "param \"save_all_factors\" is false, can't get all factors!");
    if (!m_calculated) {
        calculate();
    }
    const auto iter = m_stk_map.find(stk);
    HKU_CHECK(iter != m_stk_map.cend(), "Could not find this stock: {}", stk);
    return m_all_factors[iter->second];
}

const IndicatorList& MultiFactorBase::getAllFactors() {
    if (getParam<bool>("save_all_factors") && !m_calculated) {
        calculate();
    } else {
        HKU_WARN("param \"save_all_factors\" is false, can't get all factors!");
    }
    return m_all_factors;
}

ScoreRecordList MultiFactorBase::getScores(const Datetime& d) {
    if (!m_calculated) {
        calculate();
    }
    ScoreRecordList ret;
    const auto iter = m_date_index.find(d);
    HKU_IF_RETURN(iter == m_date_index.cend(), ret);
    ret = m_stk_factor_by_date[iter->second];
    return ret;
}

ScoreRecordList MultiFactorBase::getScores(const Datetime& date, size_t start, size_t end) {
    ScoreRecordList ret;
    HKU_IF_RETURN(start >= end, ret);

    auto cross = getScores(date);
    if (end == Null<size_t>() || end > cross.size()) {
        end = cross.size();
    }

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

    auto cross = getScores(date);
    HKU_IF_RETURN(cross.empty(), ret);

    if (end == Null<size_t>() || end > cross.size()) {
        end = cross.size();
    }

    ret.reserve(end - start);
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

    auto cross = getScores(date);
    HKU_IF_RETURN(cross.empty(), ret);

    if (end == Null<size_t>() || end > cross.size()) {
        end = cross.size();
    }

    ret.reserve(end - start);
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

ScoreRecordList MultiFactorBase::getScores(const Datetime& date, size_t start, size_t end,
                                           const ScoresFilterPtr& filter) {
    ScoreRecordList ret;
    HKU_IF_RETURN(start >= end, ret);

    auto cross = getScores(date);
    HKU_IF_RETURN(cross.empty(), ret);

    if (end == Null<size_t>() || end > cross.size()) {
        end = cross.size();
    }

    ret.reserve(end - start);
    for (size_t i = start; i < end; i++) {
        ret.emplace_back(cross[i]);
    }

    if (filter) {
        ret = filter->filter(ret, date, m_query);
    }

    return ret;
}

const vector<ScoreRecordList>& MultiFactorBase::getAllScores() {
    if (!m_calculated) {
        calculate();
    }
    return m_stk_factor_by_date;
}

Indicator MultiFactorBase::getIC(int ndays) {
    if (!m_calculated) {
        calculate();
    }

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

    auto all_returns = _getAllReturns(ndays);
    Indicator result = IC(m_all_factors, all_returns, ndays, getParam<bool>("use_spearman"));
    result.setParam<DatetimeList>("align_date_list", m_ref_dates);
    result.name("IC");

    // 如果 ndays 和 ic_n 参数相同，缓存计算结果
    if (ic_n == ndays) {
        m_ic = result;
    }
    return result;
}

Indicator MultiFactorBase::getICIR(int ir_n, int ic_n) {
    Indicator ic = getIC(ic_n);
    Indicator x = (MA(ic, ir_n) / STDEV(ic, ir_n)).getResult(0);
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

// 计算中性化后的因子，y 为因子，x 为行业哑变量（包含常数项）
static PriceList calculate_residuals(const PriceList& y, const PriceList& x) {
    HKU_ASSERT(y.size() == x.size());

    const size_t n = x.size();
    PriceList residuals(n, Null<price_t>());

    // 计算线性回归系数（带常数项）
    double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_x2 = 0.0;
    size_t count = 0;

    for (size_t i = 0; i < n; ++i) {
        if (std::isnan(x[i]) || std::isinf(x[i]) || std::isnan(y[i]) || std::isinf(y[i])) {
            continue;
        }
        sum_x += x[i];
        sum_y += y[i];
        sum_xy += x[i] * y[i];
        sum_x2 += x[i] * x[i];
        count++;
    }

    // 数据点不足或分母为0
    if (count < 2 || sum_x * sum_x - count * sum_x2 == 0) {
        return residuals;
    }

    // 计算回归系数 β₀ (截距) 和 β₁ (斜率)
    double beta1 = (sum_x * sum_y - count * sum_xy) / (sum_x * sum_x - count * sum_x2);
    double beta0 = (sum_y - beta1 * sum_x) / count;

    if (std::isnan(beta0) || std::isinf(beta0) || std::isnan(beta1) || std::isinf(beta1)) {
        return residuals;
    }

    // 计算拟合值和残差
    for (size_t i = 0; i < n; ++i) {
        if (std::isnan(x[i]) || std::isinf(x[i]) || std::isnan(y[i]) || std::isinf(y[i])) {
            continue;
        }
        double y_hat = beta0 + beta1 * x[i];  // 拟合值 = β₀ + β₁x
        residuals[i] = y[i] - y_hat;          // 残差 = 观测值 - 拟合值
    }

    return residuals;
}

// 计算多元回归中性化后的因子，y为因子，x为多个解释变量（包含常数项）- Eigen版本
static PriceList calculate_residuals(const PriceList& y, const std::vector<PriceList>& x) {
    HKU_ASSERT(!x.empty());
    size_t n = y.size();
    for (const auto& xi : x) {
        HKU_ASSERT(xi.size() == n);
    }

    PriceList residuals(n, Null<price_t>());
    size_t k = x.size();  // 解释变量个数

    // 构建设计矩阵和因变量向量
    Eigen::MatrixXd Xmat(n, k + 1);
    Eigen::VectorXd Yvec(n);

    // 填充数据 - 第一列为常数项（全1）
    Xmat.col(0).setConstant(1.0);

    // 标记有效数据点
    std::vector<bool> valid(n, true);

    for (size_t i = 0; i < n; ++i) {
        Yvec(i) = y[i];

        // 检查因变量是否有效
        if (std::isnan(y[i]) || std::isinf(y[i])) {
            valid[i] = false;
            continue;
        }

        // 填充自变量并检查有效性
        for (size_t j = 0; j < k; ++j) {
            Xmat(i, j + 1) = x[j][i];
            if (std::isnan(x[j][i]) || std::isinf(x[j][i])) {
                valid[i] = false;
                break;
            }
        }
    }

    // 计算有效数据点数量
    size_t valid_count = std::count(valid.begin(), valid.end(), true);

    // 数据点不足
    if (valid_count <= k + 1) {
        return residuals;
    }

    // 创建有效数据的子矩阵
    Eigen::MatrixXd X_valid(valid_count, k + 1);
    Eigen::VectorXd Y_valid(valid_count);

    size_t valid_idx = 0;
    for (size_t i = 0; i < n; ++i) {
        if (valid[i]) {
            X_valid.row(valid_idx) = Xmat.row(i);
            Y_valid(valid_idx) = Yvec(i);
            valid_idx++;
        }
    }

    // 使用QR分解求解线性回归 β = (X'X)^(-1)X'Y
    Eigen::VectorXd beta = X_valid.colPivHouseholderQr().solve(Y_valid);

    // 检查解是否有效
    if (beta.hasNaN()) {
        return residuals;
    }

    // 计算拟合值和残差
    Eigen::VectorXd fitted = Xmat * beta;

    for (size_t i = 0; i < n; ++i) {
        if (valid[i]) {
            residuals[i] = y[i] - fitted(i);
        }
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

    unordered_map<string, IndicatorList> use_style_inds;
    for (const auto& [name, style_inds] : m_special_style_inds) {
        use_style_inds[name] = IndicatorList(style_inds.size());
    }

    global_parallel_for_index_void(
      0, stk_count,
      [this, &all_stk_inds, &null_ind, &use_style_inds, ind_count, fill_null](size_t i) {
          const auto& stk = m_stks[i];
          auto kdata = stk.getKData(m_query);
#if 0
          auto& cur_stk_inds = all_stk_inds[i];
          cur_stk_inds.resize(ind_count);
          if (kdata.size() == 0) {
              for (size_t j = 0; j < ind_count; j++) {
                  cur_stk_inds[j] = null_ind;
                  cur_stk_inds[j].name(m_inds[j].name());
              }
          } else {
              cur_stk_inds.resize(ind_count);
              for (size_t j = 0; j < ind_count; j++) {
                  cur_stk_inds[j] = ALIGN(m_inds[j], m_ref_dates, fill_null);
              }
              cur_stk_inds = combineCalculateIndicators(cur_stk_inds, kdata, true);
          }

          for (auto& [name, styles] : m_special_style_inds) {
              auto& cur_style_inds = use_style_inds[name];
              if (kdata.size() == 0) {
                  for (size_t j = 0; j < styles.size(); j++) {
                      cur_style_inds[j] = null_ind;
                      cur_style_inds[j].name(name);
                  }
              } else {
                  for (size_t j = 0; j < styles.size(); j++) {
                      cur_style_inds[j] = ALIGN(styles[j], m_ref_dates, fill_null);
                  }
                  cur_style_inds = combineCalculateIndicators(cur_style_inds, kdata, true);
              }
          }
#else
          auto& cur_stk_inds = all_stk_inds[i];
          cur_stk_inds.resize(ind_count);
          if (kdata.size() == 0) {
              for (size_t j = 0; j < ind_count; j++) {
                  cur_stk_inds[j] = null_ind;
                  cur_stk_inds[j].name(m_inds[j].name());
              }
          } else {
              for (size_t j = 0; j < ind_count; j++) {
                  cur_stk_inds[j] = ALIGN(m_inds[j], m_ref_dates, fill_null)(kdata).getResult(0);
                  cur_stk_inds[j].name(m_inds[j].name());
              }
          }

          for (auto& [name, styles] : m_special_style_inds) {
              auto& cur_style_inds = use_style_inds[name];
              if (kdata.size() == 0) {
                  for (size_t j = 0; j < styles.size(); j++) {
                      cur_style_inds[j] = null_ind;
                      cur_style_inds[j].name(name);
                  }
              } else {
                  for (size_t j = 0; j < styles.size(); j++) {
                      cur_style_inds[j] =
                        ALIGN(styles[j], m_ref_dates, fill_null)(kdata).getResult(0);
                      cur_style_inds[j].name(name);
                  }
              }
          }
#endif
      });

    // 时间截面标准化/归一化
    if (m_norm || !m_special_category.empty() || !m_special_style_inds.empty()) {
        unordered_map<string, PriceList> ind_dummy_dict = _buildDummyIndex();
        global_parallel_for_index_void(
          0, days_total,
          [this, stk_count, ind_count, sub_norm = m_norm ? m_norm->clone() : m_norm, &all_stk_inds,
           &ind_dummy_dict, &use_style_inds](size_t di) {
              NormPtr special_norm;
              PriceList one_day(stk_count, Null<price_t>());
              PriceList new_value;
              for (size_t ii = 0; ii < ind_count; ii++) {
                  auto* one_day_data = one_day.data();
                  for (size_t si = 0; si < stk_count; si++) {
                      one_day_data[si] = all_stk_inds[si][ii][di];
                  }

                  auto ind_name = all_stk_inds[0][ii].name();
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
                      new_value = calculate_residuals(new_value, category_iter->second);
                  }
                  auto style_iter = use_style_inds.find(ind_name);
                  if (style_iter != use_style_inds.end()) {
                      vector<PriceList> style_value_day(style_iter->second.size());
                      for (size_t j = 0; j < style_iter->second.size(); j++) {
                          auto& style_value = style_value_day[j];
                          style_value.resize(stk_count);
                          for (size_t si = 0; si < stk_count; si++) {
                              style_value[si] = style_iter->second[j][di];
                          }
                      }
                      new_value = calculate_residuals(new_value, style_value_day);
                  }

                  for (size_t si = 0; si < stk_count; si++) {
                      auto* dst = all_stk_inds[si][ii].data();
                      dst[di] = new_value[si];
                  }
              }
          });
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
        m_stk_factor_by_date[i].resize(stk_count);  // 每个日期预分配股票数量的空间
    }

    // 先遍历股票j，再遍历日期i，默认不排序
    global_parallel_for_index_void(0, stk_count, [this, days_total](size_t j) {
        const auto& stk = m_stks[j];
        const auto* data = m_all_factors[j].data();
        for (size_t i = 0; i < days_total; i++) {
            m_stk_factor_by_date[i][j] = ScoreRecord(stk, data[i]);
        }
    });

    int mode = getParam<int>("mode");
    if (0 == mode) {
        global_parallel_for_index_void(
          0, days_total,
          [this](size_t i) {
              std::sort(m_stk_factor_by_date[i].begin(), m_stk_factor_by_date[i].end(),
                        [](const ScoreRecord& a, const ScoreRecord& b) {
                            if (std::isnan(a.value) && std::isnan(b.value)) {
                                return false;
                            } else if (!std::isnan(a.value) && std::isnan(b.value)) {
                                return true;
                            } else if (std::isnan(a.value) && !std::isnan(b.value)) {
                                return false;
                            }
                            return a.value > b.value;
                        });
          },
          100);

    } else if (1 == mode) {
        global_parallel_for_index_void(
          0, days_total,
          [this](size_t i) {
              std::sort(m_stk_factor_by_date[i].begin(), m_stk_factor_by_date[i].end(),
                        [](const ScoreRecord& a, const ScoreRecord& b) {
                            if (std::isnan(a.value) && std::isnan(b.value)) {
                                return false;
                            } else if (!std::isnan(a.value) && std::isnan(b.value)) {
                                return true;
                            } else if (std::isnan(a.value) && !std::isnan(b.value)) {
                                return false;
                            }
                            return a.value < b.value;
                        });
          },
          100);
    }
}

void MultiFactorBase::calculate() {
    HKU_IF_RETURN(m_calculated, void());

    std::lock_guard<std::mutex> lock(m_mutex);
    _checkData();

    try {
        {  // 获取所有证券所有对齐后的原始因子
            vector<IndicatorList> all_stk_inds = getAllSrcFactors();

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
        }

        // 计算完成后创建截面索引
        _buildIndex();

    } catch (const std::exception& e) {
        HKU_ERROR(e.what());
    } catch (...) {
        HKU_ERROR_UNKNOWN;
    }

    if (!getParam<bool>("save_all_factors")) {
        m_all_factors = {};
        m_stk_map = {};
    }

    // 更新计算状态
    m_calculated = true;
}

}  // namespace hku