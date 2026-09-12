/*
 * AllocateFundsBase.cpp
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  组合级资金分配（AF）基类实现。L1/L2/L3 三个算法部件的默认实现由
 *  MoneyManagerBase 的组合级实现迁移而来（见 docs/design/pf_af_compat/design.md §5）。
 */

#include "AllocateFundsBase.h"

#include <sstream>

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const AllocateFundsBase& af) {
    os << "AllocateFunds(" << af.name() << ", " << af.getParameter() << ")";
    return os;
}

HKU_API std::ostream& operator<<(std::ostream& os, const AllocateFundsPtr& af) {
    if (af) {
        os << *af;
    } else {
        os << "AllocateFunds(NULL)";
    }
    return os;
}

AllocateFundsBase::AllocateFundsBase() : m_name("AllocateFundsBase") {
    setParam<double>("max-single-position", 1.0);
    // 组合级分配参数：weight-list 固定权重（L1）、fixed-amount 固定金额（L2）
    setParam<string>("weight-list", "");
    setParam<double>("fixed-amount", 0.0);
}

AllocateFundsBase::AllocateFundsBase(const string& name) : m_name(name) {
    setParam<double>("max-single-position", 1.0);
    setParam<string>("weight-list", "");
    setParam<double>("fixed-amount", 0.0);
}

AllocateFundsBase::~AllocateFundsBase() {}

void AllocateFundsBase::baseCheckParam(const string& name) const {
    if ("max-single-position" == name) {
        HKU_ASSERT(getParam<double>("max-single-position") >= 0.0);
    }
    if ("fixed-amount" == name) {
        HKU_ASSERT(getParam<double>("fixed-amount") >= 0.0);
    }
}

void AllocateFundsBase::paramChanged() {}

void AllocateFundsBase::reset() {
    m_query = Null<KQuery>();
    m_tm.reset();
    _reset();
}

AllocateFundsPtr AllocateFundsBase::clone() {
    AllocateFundsPtr p;
    try {
        p = _clone();
    } catch (...) {
        HKU_ERROR("Subclass _clone failed!");
        p = AllocateFundsPtr();
    }

    if (!p || p.get() == this) {
        HKU_ERROR("Failed clone! Will use self-ptr!");
        return shared_from_this();
    }

    p->m_params = m_params;
    p->m_name = m_name;
    p->m_is_python_object = m_is_python_object;
    p->m_tm = m_tm;
    p->m_query = m_query;
    p->m_mode = m_mode;
    return p;
}

//============================================================================
// 组合级资金分配（AF L1/L2/L3），供聚合 System（MultiSystem）调用
//============================================================================

void AllocateFundsBase::allocate(const Datetime& date, const TradeManagerPtr& tm,
                                 TradeSuggestionList& suggestions, SubSystemContextList& contexts,
                                 const KQuery& query) {
    auto weights = _allocate(date, tm, contexts, query);
    _toTargets(date, tm, suggestions, weights, query);
    _checkRisk(date, tm, suggestions, query);
}

AllocateFundsBase::Weights AllocateFundsBase::_allocate(const Datetime& date,
                                                        const TradeManagerPtr& tm,
                                                        SubSystemContextList& contexts,
                                                        const KQuery& query) {
    if (contexts.empty()) {
        return Weights();
    }
    // L1 权重来源：
    //   1) 参数 weight-list 非空：按序解析为各子系统固定权重并归一化（AF_FixedWeightList 语义）；
    //   2) 否则等权 1/N（AF_EqualWeight，默认）。
    std::vector<double> weights = _parseWeightList(contexts.size());
    if (weights.size() != contexts.size()) {
        weights.assign(contexts.size(), 1.0 / contexts.size());
    }
    return _applyWeights(date, tm, contexts, query, weights);
}

AllocateFundsBase::Weights AllocateFundsBase::_applyWeights(const Datetime& date,
                                                            const TradeManagerPtr& tm,
                                                            SubSystemContextList& contexts,
                                                            const KQuery& query,
                                                            const std::vector<double>& weights) const {
    Weights result;
    if (contexts.empty()) {
        return result;
    }
    // 模式 B 额度：fixed-amount>0 时每子系统固定额度（AF_FixedAmount），否则权重 × 父总资产。
    double fixed_amount = getParam<double>("fixed-amount");
    double total_assets = tm ? tm->getFunds(date, query.kType()).total_assets() : 0.0;
    double eq = 1.0 / contexts.size();
    for (size_t i = 0; i < contexts.size(); ++i) {
        auto& ctx = contexts[i];
        double w = (i < weights.size()) ? weights[i] : eq;
        result[ctx.sys] = w;
        if (m_mode == "B" && tm) {
            // 模式 B：L1 产出「真实额度」写入 contexts[i].quota，由父在调仓日回写给子系统
            //（供下期运行，额度滞后一期）。
            ctx.quota = (fixed_amount > 0.0) ? fixed_amount : (w * total_assets);
        }
    }
    return result;
}

std::vector<double> AllocateFundsBase::_parseWeightList(size_t expect_n) const {
    std::vector<double> result;
    string wl = getParam<string>("weight-list");
    if (wl.empty()) {
        return result;  // 空 → 调用方回退等权
    }
    std::stringstream ss(wl);
    string item;
    double sum = 0.0;
    while (std::getline(ss, item, ',')) {
        size_t b = item.find_first_not_of(" \t\r\n");
        if (b == string::npos) {
            continue;  // 纯空白项跳过
        }
        size_t e = item.find_last_not_of(" \t\r\n");
        item = item.substr(b, e - b + 1);
        double v = 0.0;
        try {
            v = std::stod(item);
        } catch (...) {
            HKU_WARN("weight-list contains invalid item '{}', treated as 0!", item);
            v = 0.0;
        }
        if (v < 0.0) {
            v = 0.0;  // 负权重按 0 处理（不支持做空额度分配）
        }
        result.push_back(v);
        sum += v;
    }
    // 数量须与子系统一致且总和 >0，否则视为无效，回退等权
    if (result.size() != expect_n || sum <= 0.0) {
        HKU_WARN_IF(result.size() != expect_n,
                    "weight-list size({}) != subsystems({}), fallback to equal weight!",
                    result.size(), expect_n);
        result.clear();
        return result;
    }
    for (auto& v : result) {
        v /= sum;  // 归一化，使权重和为 1
    }
    return result;
}

void AllocateFundsBase::_toTargets(const Datetime& date, const TradeManagerPtr& tm,
                                   TradeSuggestionList& suggestions, const Weights& sys_weight,
                                   const KQuery& query) {
    KQuery::KType ktype = query.kType();
    if (m_mode == "B") {
        // 模式 B：透传子系统真实指令（number 即子管理人的下单量），父不换算。
        // 只对 SELL 建议做「不超父当前持仓」的防御性裁剪。
        for (auto& s : suggestions) {
            if (s.type == SuggestionType::SELL) {
                double current = tm ? tm->getPosition(date, s.stock).number : 0.0;
                if (s.number > current) {
                    s.number = current;
                }
            }
        }
        return;
    }

    FundsRecord funds = tm->getFunds(date, ktype);
    double total_assets = funds.total_assets();
    // 模式 A + fixed-amount>0：每标的目标市值取固定金额（AF_FixedAmount 的行为级语义），优先于按比重换算。
    double fixed_amount = getParam<double>("fixed-amount");
    for (auto& s : suggestions) {
        if (s.plan_price <= 0.0) {
            s.number = 0.0;
            continue;
        }
        double weight = 1.0;
        auto it = sys_weight.find(s.sys);
        if (it != sys_weight.end()) {
            weight = it->second;
        }
        double current = tm->getPosition(date, s.stock).number;
        if (s.type == SuggestionType::BUY) {
            // 模式 A（默认）：目标持仓市值 = 子系统权重 × 子建议仓位比重 × 父总资产。
            //   - fixed-amount>0：每标的固定金额（AF_FixedAmount），优先于按比重换算；
            //   - assets_ratio>0：尊重子系统上送的内部仓位比重（如子半仓则父也按半仓映射）；
            //   - assets_ratio<=0（无比重信息）：回退按满仓（ratio=1）等权到仓。
            // 换算为目标股数，净调仓量 =（目标 - 当前）；若为负（当前已超配），转 SELL 减仓至目标，
            // 避免“负 BUY”在执行阶段被丢弃导致超配仓位无法再平衡（与 PF 周期性再平衡语义对齐）。
            double target_value;
            if (fixed_amount > 0.0) {
                target_value = fixed_amount;
            } else {
                double ratio = (s.assets_ratio > 0.0) ? s.assets_ratio : 1.0;
                target_value = weight * ratio * total_assets;
            }
            double target_shares = target_value / s.plan_price;
            double delta = target_shares - current;
            if (delta < 0.0) {
                s.type = SuggestionType::SELL;
                s.number = -delta;  // 减仓数量（正数），执行阶段按 SELL 卖出
            } else {
                s.number = delta;
            }
        } else {
            // SELL / CLEAR：退出该标的（卖出当前全部持仓）
            s.number = -current;
        }
    }
}

void AllocateFundsBase::_checkRisk(const Datetime& date, const TradeManagerPtr& tm,
                                   TradeSuggestionList& suggestions, const KQuery& query) {
    // L3 组合风控（模式 A 默认启用；模式 B 尊重子管理人自主权，仅做总量校验 = 不裁剪）。
    if (m_mode == "B") {
        return;
    }
    // 集中度上限：单标的目标持仓市值 ≤ 总资产 × max-single-position（<=0 或 >=1 表示不限制）。
    double max_ratio = getParam<double>("max-single-position");
    if (max_ratio <= 0.0 || max_ratio >= 1.0) {
        return;
    }
    KQuery::KType ktype = query.kType();
    double total_assets = tm ? tm->getFunds(date, ktype).total_assets() : 0.0;
    double cap = total_assets * max_ratio;
    for (auto& s : suggestions) {
        if (s.type != SuggestionType::BUY || s.number <= 0.0 || s.plan_price <= 0.0) {
            continue;
        }
        double current = tm ? tm->getPosition(date, s.stock).number : 0.0;
        double target_value = (current + s.number) * s.plan_price;
        if (target_value > cap) {
            double max_shares = cap / s.plan_price;
            s.number = max_shares > current ? max_shares - current : 0.0;
        }
    }
}

} /* namespace hku */
