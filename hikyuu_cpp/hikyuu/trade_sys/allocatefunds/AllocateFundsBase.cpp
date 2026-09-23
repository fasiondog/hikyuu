/*
 * AllocateFundsBase.cpp
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Implementation of the portfolio-level fund allocation (AF) base class. The default
 *  implementations of the three algorithm parts L1/L2/L3 are migrated from the portfolio-level
 *  implementation of MoneyManagerBase (see docs/design/pf_af_compat/design.md §5).
 *  Created on: 2018-1-30
 *      Author: fasiondog
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
    // The portfolio-level allocation parameters: weight-list fixed weights (L1), fixed-amount fixed amount (L2)
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
// The portfolio-level fund allocation (AF L1/L2/L3), called by the aggregate System (MultiSystem)
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
    // The L1 weight source:
    //   1) the parameter weight-list is not empty: parse it into the fixed weights of every sub-system in order and normalize them (the AF_FixedWeightList semantics);
    //   2) otherwise the equal weight 1/N (AF_EqualWeight, the default).
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
    // The mode B quota: a fixed quota per sub-system when fixed-amount>0 (AF_FixedAmount), otherwise weight x the parent total assets.
    double fixed_amount = getParam<double>("fixed-amount");
    double total_assets = tm ? tm->getFunds(date, query.kType()).total_assets() : 0.0;
    double eq = 1.0 / contexts.size();
    for (size_t i = 0; i < contexts.size(); ++i) {
        auto& ctx = contexts[i];
        double w = (i < weights.size()) ? weights[i] : eq;
        result[ctx.sys] = w;
        if (m_mode == "B" && tm) {
            // Mode B: L1 produces the "real quota" and writes it into contexts[i].quota, the parent writes it back to the sub-system on the rebalancing day
            // (for the next period, the quota lags one period behind).
            ctx.quota = (fixed_amount > 0.0) ? fixed_amount : (w * total_assets);
        }
    }
    return result;
}

std::vector<double> AllocateFundsBase::_parseWeightList(size_t expect_n) const {
    std::vector<double> result;
    string wl = getParam<string>("weight-list");
    if (wl.empty()) {
        return result;  // Empty -> the caller falls back to the equal weight
    }
    std::stringstream ss(wl);
    string item;
    double sum = 0.0;
    while (std::getline(ss, item, ',')) {
        size_t b = item.find_first_not_of(" \t\r\n");
        if (b == string::npos) {
            continue;  // Skip the pure blank items
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
            v = 0.0;  // The negative weights are treated as 0 (the short quota allocation is not supported)
        }
        result.push_back(v);
        sum += v;
    }
    // The quantity must match the sub-systems and the sum must be > 0, otherwise it is invalid and falls back to the equal weight
    if (result.size() != expect_n || sum <= 0.0) {
        HKU_WARN_IF(result.size() != expect_n,
                    "weight-list size({}) != subsystems({}), fallback to equal weight!",
                    result.size(), expect_n);
        result.clear();
        return result;
    }
    for (auto& v : result) {
        v /= sum;  // Normalize so that the weight sum is 1
    }
    return result;
}

void AllocateFundsBase::_toTargets(const Datetime& date, const TradeManagerPtr& tm,
                                   TradeSuggestionList& suggestions, const Weights& sys_weight,
                                   const KQuery& query) {
    KQuery::KType ktype = query.kType();
    if (m_mode == "B") {
        // Mode B: pass through the real instruction of the sub-system (number is the order quantity of the sub-manager), the parent does not convert it.
        // Only the SELL suggestions are defensively clipped to not exceed the current position of the parent.
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
    // Mode A + fixed-amount>0: the target market value of every instrument takes the fixed amount (the AF_FixedAmount behavior-level semantics), it takes precedence over the conversion by proportion.
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
            // Mode A (the default): target position market value = sub-system weight x the sub-suggestion position ratio x the parent total assets.
            //   - fixed-amount>0: a fixed amount per instrument (AF_FixedAmount), it takes precedence over the conversion by proportion;
            //   - assets_ratio>0: respect the internal position ratio submitted by the sub-system (e.g. the parent is mapped by half position when the sub-system is at half position);
            //   - assets_ratio<=0 (no ratio information): fall back to the full position (ratio=1) equal weight to position.
            // Convert into the target share quantity, the net rebalancing quantity = (target - current); when it is negative (the current position is over-allocated), turn to SELL to reduce to the target,
            // to avoid the "negative BUY" being discarded at the execution stage causing the over-allocated position to be unable to rebalance (aligned with the PF periodic rebalancing semantics).
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
                s.number = -delta;  // The position-reducing quantity (a positive number), sold by SELL at the execution stage
            } else {
                s.number = delta;
            }
        } else {
            // SELL / CLEAR: exit the instrument (sell all the current holdings)
            s.number = -current;
        }
    }
}

void AllocateFundsBase::_checkRisk(const Datetime& date, const TradeManagerPtr& tm,
                                   TradeSuggestionList& suggestions, const KQuery& query) {
    // L3 portfolio risk control (enabled by default in mode A; mode B respects the autonomy of the sub-manager, only the total amount check is performed = no clipping).
    if (m_mode == "B") {
        return;
    }
    // The concentration upper limit: the target position market value of a single instrument <= total assets x max-single-position (<=0 or >=1 means no limit).
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
