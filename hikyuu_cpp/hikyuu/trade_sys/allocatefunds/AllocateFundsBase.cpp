/*
 * AllocateFundsBase.cpp
 *
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Implementation of the portfolio-level fund allocation (AF) base class. The default
 *  implementations of the three algorithm parts L1/L2/L3 are migrated from the portfolio-level
 *  implementation of MoneyManagerBase.
 *  Created on: 2018-1-30
 *      Author: fasiondog
 */

#include "AllocateFundsBase.h"

#include <algorithm>
#include <cmath>
#include <map>
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
    // L3 single instrument concentration cap: the target market value <= total assets x this
    // value (<=0 means no limit, >1 is clamped to 1)
    setParam<double>("max-single-position", 1.0);

    // L2 total budget cap: the aggregated target market value of all instruments <= total assets
    // x this value, scaled down proportionally when exceeded (<=0 disables the cap)
    setParam<double>("max-total-position", 1.0);

    // L1 fixed weights (comma separated, normalized); empty means the equal weight 1/N
    setParam<string>("weight-list", "");

    // L2 fixed amount: the target market value per instrument in mode A, the quota per sub-system
    // in mode B
    setParam<double>("fixed-amount", 0.0);
}

AllocateFundsBase::AllocateFundsBase(const string& name) : m_name(name) {
    setParam<double>("max-single-position", 1.0);
    setParam<double>("max-total-position", 1.0);
    setParam<string>("weight-list", "");
    setParam<double>("fixed-amount", 0.0);
}

AllocateFundsBase::~AllocateFundsBase() {}

void AllocateFundsBase::baseCheckParam(const string& name) const {
    if ("max-single-position" == name) {
        HKU_ASSERT(getParam<double>("max-single-position") >= 0.0);
    }
    if ("max-total-position" == name) {
        HKU_ASSERT(getParam<double>("max-total-position") >= 0.0);
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

AFPtr AllocateFundsBase::clone() {
    AFPtr p = _clone();
    HKU_CHECK(p && p.get() != this,
              "Failed clone! The subclass _clone of {} must return a new object!", m_name);

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
    auto weights = allocateQuota(date, tm, contexts, query);
    allocateTargets(date, tm, suggestions, weights, query);
}

AllocateFundsBase::Weights AllocateFundsBase::allocateQuota(const Datetime& date,
                                                            const TradeManagerPtr& tm,
                                                            SubSystemContextList& contexts,
                                                            const KQuery& query) {
    return _allocate(date, tm, contexts, query);
}

void AllocateFundsBase::allocateTargets(const Datetime& date, const TradeManagerPtr& tm,
                                        TradeSuggestionList& suggestions, const Weights& sys_weight,
                                        const KQuery& query) {
    _toTargets(date, tm, suggestions, sys_weight, query);
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
    //   1) the parameter weight-list is not empty: parse it into the fixed weights of every
    //   sub-system in order and normalize them (the AF_FixedWeightList semantics); 2) otherwise the
    //   equal weight 1/N (AF_EqualWeight, the default).
    std::vector<double> weights = _parseWeightList(contexts.size());
    if (weights.size() != contexts.size()) {
        weights.assign(contexts.size(), 1.0 / contexts.size());
    }
    return _applyWeights(date, tm, contexts, query, weights);
}

AllocateFundsBase::Weights AllocateFundsBase::_applyWeights(
  const Datetime& date, const TradeManagerPtr& tm, SubSystemContextList& contexts,
  const KQuery& query, const std::vector<double>& weights) const {
    Weights result;
    if (contexts.empty()) {
        return result;
    }
    // The mode B quota: a fixed quota per sub-system when fixed-amount>0 (AF_FixedAmount),
    // otherwise weight x the parent total assets (the parent valuation is needed only there).
    double fixed_amount = getParam<double>("fixed-amount");
    double eq = 1.0 / contexts.size();
    double total_assets =
      (m_mode == "B" && tm) ? tm->getFunds(date, query.kType()).total_assets() : 0.0;
    for (size_t i = 0; i < contexts.size(); ++i) {
        auto& ctx = contexts[i];
        if (!ctx.sys) {
            // A missing key would be silently looked up as the full position downstream
            HKU_WARN("The context[{}] has no sub-system, skipped in the allocation!", i);
            continue;
        }
        double w = (i < weights.size()) ? weights[i] : eq;
        result[ctx.sys] = w;
        // Mode C takes no quota at all: the order quantity of every sub-system comes from its own
        // MM working directly on the shared real account.
        if (m_mode == "B" && tm) {
            // Mode B: L1 produces the "real quota" and writes it into contexts[i].quota, the parent
            // writes it back to the sub-system on the rebalancing day (for the next period, the
            // quota lags one period behind).
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
        // Reject the whole list on a non-finite value (std::stod parses "nan" / "inf" without
        // throwing and they poison every downstream quota)
        if (!std::isfinite(v)) {
            HKU_WARN("weight-list contains non-finite item '{}', fallback to equal weight!", item);
            return std::vector<double>();
        }
        if (v < 0.0) {
            v = 0.0;  // The negative weights are treated as 0 (the short quota allocation is not
                      // supported)
        }
        result.push_back(v);
        sum += v;
    }
    // The quantity must match the sub-systems and the sum must be finite and > 0 (an overflowing
    // sum normalizes every weight to 0), otherwise it is invalid and falls back to the equal weight
    if (result.size() != expect_n || !(sum > 0.0) || !std::isfinite(sum)) {
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
    if (m_mode == "B" || m_mode == "C") {
        // Mode B / C: pass through the real instruction of the sub-system (number is the order
        // quantity of the sub-manager), the parent does not convert it. Only the long-side SELL
        // suggestions are defensively clipped to not exceed the current position of the parent; a
        // short-side SELL opens/increases a short and has no long position to clip against.
        for (auto& s : suggestions) {
            if (s.type == SuggestionType::SELL && !s.short_side) {
                double current = tm ? tm->getPosition(date, s.stock).number : 0.0;
                if (s.number > current) {
                    s.number = current;
                }
            }
        }
        return;
    }

    // The mode A conversion works on the parent account, it is undefined without it
    HKU_WARN_IF_RETURN(!tm, void(), "{}::_toTargets: the trade manager is null, skip!", m_name);

    // Mode A: the parent synthesizes one net target per instrument from the sub-system snapshots.
    // Each suggestion carries the signed net exposure ratio (target_position_ratio) of one
    // sub-system for one instrument; the parent target value = sum(weight x ratio) x parent net.
    double parent_net = tm->getFunds(date, ktype).net_assets();
    double fixed_amount = getParam<double>("fixed-amount");

    std::map<Stock, double> target_value;  // instrument -> signed net target market value
    std::map<Stock, price_t> price;        // instrument -> the close price used for the shares
    double gross = 0.0;
    for (const auto& s : suggestions) {
        if (s.stock.isNull() || s.plan_price <= 0.0) {
            continue;
        }
        double weight = 1.0;
        auto it = sys_weight.find(s.sys);
        if (it != sys_weight.end()) {
            weight = it->second;
        } else if (!sys_weight.empty()) {
            weight = 1.0 / sys_weight.size();
        }
        double v;
        if (fixed_amount > 0.0) {
            // fixed-amount: the per-instrument target is the fixed amount on its own side
            v = (s.target_position_ratio > 0.0)
                  ? fixed_amount
                  : (s.target_position_ratio < 0.0 ? -fixed_amount : 0.0);
        } else {
            v = weight * s.target_position_ratio * parent_net;
        }
        if (!std::isfinite(v)) {
            HKU_WARN("{}::_toTargets: non-finite target value of {}, skipped!", m_name,
                     s.stock.market_code());
            continue;
        }
        target_value[s.stock] += v;
        price[s.stock] = s.plan_price;
        gross += std::fabs(v);
    }

    // The total budget cap: the aggregated gross target market value <= net assets x
    // max-total-position, scaled down proportionally (<=0 disables the cap)
    double max_total = getParam<double>("max-total-position");
    if (max_total > 0.0 && parent_net > 0.0 && gross > parent_net * max_total) {
        double scale = parent_net * max_total / gross;
        HKU_WARN(
          "{}::_toTargets: the aggregated gross target({:.2f}) exceeds max-total-position({}) x "
          "net assets({:.2f}), scaled by {:.4f}!",
          m_name, gross, max_total, parent_net, scale);
        for (auto& kv : target_value) {
            kv.second *= scale;
        }
    }

    // Rebuild into the net delta orders of the parent account (one or two legs per instrument:
    // close the current side first, then open the target side)
    TradeSuggestionList orders;
    for (const auto& kv : target_value) {
        const Stock& stock = kv.first;
        price_t p = price[stock];
        double unit = stock.unit();
        if (p <= 0.0 || unit <= 0.0) {
            continue;
        }
        double target_shares = kv.second / (p * unit);
        double cur_long = tm->getPosition(date, stock).number;
        double cur_short = tm->getShortPosition(stock).number;
        double cur_net = cur_long - cur_short;
        double delta = target_shares - cur_net;
        double min_trade = stock.minTradeNumber();
        auto make = [&](SuggestionType type, double number, bool short_side) {
            TradeSuggestion o;
            o.stock = stock;
            o.from = PART_PORTFOLIO;
            o.type = type;
            o.number = number;
            o.short_side = short_side;
            o.plan_price = p;
            o.plan_cash = number * p * unit;
            return o;
        };
        if (min_trade > 0.0 && std::fabs(delta) < min_trade) {
            // Below one lot: no trade — except a clearance (a flat target), otherwise a residual
            // position below one lot would stay untradable forever (the skip would always fire)
            if (delta == 0.0 || target_shares != 0.0) {
                continue;
            }
            if (cur_net > 0.0) {
                orders.push_back(make(SuggestionType::CLEAR, cur_long, false));
            } else if (cur_net < 0.0) {
                orders.push_back(make(SuggestionType::CLEAR, cur_short, true));
            }
            continue;
        }
        if (cur_net >= 0.0 && target_shares >= 0.0) {
            if (delta > 0.0) {
                orders.push_back(make(SuggestionType::BUY, delta, false));
            } else {
                orders.push_back(make(SuggestionType::SELL, -delta, false));
            }
        } else if (cur_net <= 0.0 && target_shares <= 0.0) {
            if (delta > 0.0) {
                orders.push_back(make(SuggestionType::BUY, delta, true));  // cover
            } else {
                orders.push_back(make(SuggestionType::SELL, -delta, true));  // add short
            }
        } else if (cur_net > 0.0 && target_shares < 0.0) {
            // Long -> short: sell the long, then open the short
            orders.push_back(make(SuggestionType::SELL, cur_long, false));
            orders.push_back(make(SuggestionType::SELL, -target_shares, true));
        } else {
            // Short -> long: cover the short, then buy the long
            orders.push_back(make(SuggestionType::BUY, cur_short, true));
            orders.push_back(make(SuggestionType::BUY, target_shares, false));
        }
    }
    suggestions = orders;
}

void AllocateFundsBase::_checkRisk(const Datetime& date, const TradeManagerPtr& tm,
                                   TradeSuggestionList& suggestions, const KQuery& query) {
    // L3 portfolio risk control (enabled by default in mode A; mode B and mode C respect the
    // autonomy of the sub-strategy, only the total amount check is performed = no clipping).
    if (m_mode == "B" || m_mode == "C") {
        return;
    }
    // The concentration upper limit: the target position market value of a single instrument <=
    // net assets x max-single-position (<=0 means no limit; >1 is clamped to 1). The net assets
    // are the denominator so that a levered account is not granted a larger cap by the borrowed
    // funds.
    double max_ratio = getParam<double>("max-single-position");
    if (max_ratio <= 0.0) {
        return;
    }
    if (max_ratio > 1.0) {
        max_ratio = 1.0;
    }
    KQuery::KType ktype = query.kType();
    double net_assets = tm ? tm->getFunds(date, ktype).net_assets() : 0.0;
    double cap = net_assets * max_ratio;
    for (auto& s : suggestions) {
        if (s.number <= 0.0 || s.plan_price <= 0.0 || s.stock.isNull()) {
            continue;
        }
        // Only an order that GROWS the absolute exposure can breach the cap: a long buy or an open
        // short; a long sell or a cover reduces it
        const bool grows = (s.type == SuggestionType::BUY && !s.short_side) ||
                           (s.type == SuggestionType::SELL && s.short_side);
        if (!grows) {
            continue;
        }
        double current = tm ? (s.short_side ? tm->getShortPosition(s.stock).number
                                            : tm->getPosition(date, s.stock).number)
                            : 0.0;
        price_t unit = s.stock.unit();
        double u = (unit > 0.0) ? unit : 1.0;
        double target_value = (current + s.number) * s.plan_price * u;
        if (target_value > cap) {
            double max_shares = (s.plan_price > 0.0) ? cap / (s.plan_price * u) : 0.0;
            s.number = max_shares > current ? max_shares - current : 0.0;
        }
    }
}

} /* namespace hku */
