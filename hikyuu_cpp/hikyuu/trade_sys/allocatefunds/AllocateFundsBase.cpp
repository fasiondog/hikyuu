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
#include <mutex>

namespace hku {

// The one-shot notice flag: the fixed-amount pricing silently overrides the weights, so the
// surprise is worth exactly one log line per process instead of one per rebalance
static std::once_flag g_fixed_amount_warn_flag;

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

    // The total budget cap: in mode A the aggregated gross target market value of all instruments
    // <= net assets x this value, scaled down proportionally when exceeded; in mode B the sum of
    // the sub-system quotas <= total assets x this value (<=0 disables the cap). The reserved /
    // unallocated asset ratio of the master reserve_percent is expressed by setting it to 1 - r.
    setParam<double>("max-total-position", 1.0);

    // The pricing policy switch: when >0 the fixed amount takes precedence over the weights --
    // the per-instrument target in mode A / the per sub-system quota in mode B are both this
    // absolute value, and the L1 weights are ignored in the pricing
    setParam<double>("fixed-amount", 0.0);

    // Whether to force-adjust the running sub-systems (the ones still holding value funded in a
    // previous period) to the new quota on the rebalancing day. Only effective in mode B: mode A
    // re-targets every bar by design (always adjusts) and mode C has no quota at all (never
    // adjusts). true - the full calibration (reduce the over quota / inject the gap); false - the
    // running sub-systems keep their current holdings, only the new sub-systems get the remaining
    // parent cash
    setParam<bool>("adjust_running_sys", true);
}

AllocateFundsBase::AllocateFundsBase(const string& name) : m_name(name) {
    setParam<double>("max-single-position", 1.0);
    setParam<double>("max-total-position", 1.0);
    setParam<double>("fixed-amount", 0.0);
    setParam<bool>("adjust_running_sys", true);
}

AllocateFundsBase::~AllocateFundsBase() {}

void AllocateFundsBase::baseCheckParam(const string& name) const {
    // The legacy base-class weight-list parameter has been removed (the fixed weights now go
    // through AF_FixedWeightList / its "weights" parameter); setting it would otherwise be
    // silently accepted and ignored, so it is called out explicitly.
    if ("weight-list" == name) {
        HKU_WARN(
          "{}: the parameter \"weight-list\" has been removed and is ignored; use "
          "AF_FixedWeightList (its \"weights\" parameter) instead!",
          m_name);
    }
    // max-single-position / max-total-position / fixed-amount: <=0 means no limit (or no fixed
    // pricing), so negative values are allowed and none of them is bounded from below.
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
    return p;
}

//============================================================================
// The portfolio-level fund allocation (AF L1/L2/L3), called by the aggregate System (MultiSystem)
//============================================================================

void AllocateFundsBase::allocate(const Datetime& date, const TradeManagerPtr& tm,
                                 TradeSuggestionList& suggestions, SubSystemContextList& contexts,
                                 const KQuery& query, const string& mode) {
    auto weights = allocateQuota(date, tm, contexts, query, mode);
    allocateTargets(date, tm, suggestions, weights, query, mode);
}

AllocateFundsBase::Weights AllocateFundsBase::allocateQuota(const Datetime& date,
                                                            const TradeManagerPtr& tm,
                                                            SubSystemContextList& contexts,
                                                            const KQuery& query,
                                                            const string& mode) {
    return _allocate(date, tm, contexts, query, mode);
}

void AllocateFundsBase::allocateTargets(const Datetime& date, const TradeManagerPtr& tm,
                                        TradeSuggestionList& suggestions, const Weights& sys_weight,
                                        const KQuery& query, const string& mode) {
    _toTargets(date, tm, suggestions, sys_weight, query, mode);
    _checkRisk(date, tm, suggestions, query, mode);
}

AllocateFundsBase::Weights AllocateFundsBase::_allocate(const Datetime& date,
                                                        const TradeManagerPtr& tm,
                                                        SubSystemContextList& contexts,
                                                        const KQuery& query, const string& mode) {
    if (contexts.empty()) {
        return Weights();
    }
    // The L1 default: the equal weight 1/N (AF_EqualWeight). The other weight sources are the
    // subclasses overriding _allocate (AF_FixedWeight / AF_FixedWeightList / AF_MultiFactor); the
    // legacy base-class weight-list parameter has been removed, the fixed weights go through
    // AF_FixedWeightList.
    std::vector<double> weights(contexts.size(), 1.0 / contexts.size());
    return _applyWeights(date, tm, contexts, query, weights, mode);
}

AllocateFundsBase::Weights AllocateFundsBase::_applyWeights(
  const Datetime& date, const TradeManagerPtr& tm, SubSystemContextList& contexts,
  const KQuery& query, const std::vector<double>& weights, const string& mode) const {
    Weights result;
    if (contexts.empty()) {
        return result;
    }
    // The pricing policy: a fixed quota per sub-system when fixed-amount>0 (AF_FixedAmount),
    // otherwise weight x the parent net assets (the parent valuation is needed only there).
    double fixed_amount = getParam<double>("fixed-amount");
    if (fixed_amount > 0.0) {
        std::call_once(g_fixed_amount_warn_flag, [this]() {
            HKU_WARN("{}: fixed-amount > 0 takes precedence over the weights in the pricing!",
                     m_name);
        });
    }
    double eq = 1.0 / contexts.size();
    double net_assets = (mode == "B" && tm) ? tm->getFunds(date, query.kType()).net_assets() : 0.0;
    double sum_w = 0.0;
    for (size_t i = 0; i < contexts.size(); ++i) {
        auto& ctx = contexts[i];
        if (!ctx.sys) {
            // A missing key would be silently looked up as the full position downstream
            HKU_WARN("The context[{}] has no sub-system, skipped in the allocation!", i);
            continue;
        }
        double w = (i < weights.size()) ? weights[i] : eq;
        // A non-finite weight (e.g. a broken score chain) must not reach the quota, where it would
        // poison the downstream calculation; it is skipped like a missing context.
        if (!std::isfinite(w)) {
            HKU_WARN("{}::_applyWeights: the non-finite weight of the context[{}] is skipped!",
                     name(), i);
            continue;
        }
        result[ctx.sys] = w;
        sum_w += w;
    }
    // Mode C takes no quota at all: the order quantity of every sub-system comes from its own
    // MM working directly on the shared real account.
    if (mode == "B" && tm) {
        // The mode B budget cap: the sum of the quotas <= net assets x max-total-position,
        // scaled down proportionally when exceeded (<=0 disables the cap). The fixed-amount
        // pricing is exempt: the quota there is the absolute amount the user asked for, not a
        // proportion to be scaled.
        double max_total = getParam<double>("max-total-position");
        // The relative epsilon absorbs the floating-point noise of a legitimate full allocation
        // (a sum of normalized weights worth 1.0 + 2.2e-16 must not trigger a spurious scaling)
        if (fixed_amount <= 0.0 && max_total > 0.0 && std::isfinite(sum_w) &&
            sum_w > max_total * (1.0 + 1e-9)) {
            double scale = max_total / sum_w;
            HKU_WARN(
              "{}: the aggregated quota weights({:.4f}) exceed max-total-position({}), scaled by "
              "{:.4f}!",
              m_name, sum_w, max_total, scale);
            for (auto& kv : result) {
                kv.second *= scale;
            }
        }
        for (auto& ctx : contexts) {
            if (!ctx.sys) {
                continue;
            }
            // Mode B: L1 produces the "real quota" and writes it into contexts[i].quota, the parent
            // writes it back to the sub-system on the rebalancing day (for the next period, the
            // quota lags one period behind).
            auto it = result.find(ctx.sys);
            double w = (it != result.end()) ? it->second : 0.0;
            ctx.quota = (fixed_amount > 0.0) ? fixed_amount : (w * net_assets);
        }
    }
    return result;
}

void AllocateFundsBase::_toTargets(const Datetime& date, const TradeManagerPtr& tm,
                                   TradeSuggestionList& suggestions, const Weights& sys_weight,
                                   const KQuery& query, const string& mode) {
    KQuery::KType ktype = query.kType();
    if (mode == "B" || mode == "C") {
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
    std::map<Stock, double> fixed_dir;  // instrument -> aggregated signed direction (fixed-amount)
    std::map<Stock, price_t> price;     // instrument -> the close price used for the shares
    double gross = 0.0;
    for (const auto& s : suggestions) {
        if (s.stock.isNull() || s.plan_price <= 0.0) {
            continue;
        }
        price[s.stock] = s.plan_price;
        if (fixed_amount > 0.0) {
            // fixed-amount: the pricing is per instrument, not per suggestion. The same instrument
            // held by N sub-systems must not be scaled to N x fixed-amount, so only the aggregate
            // signed direction is accumulated here and the ±fixed-amount target is applied once
            // per instrument below.
            fixed_dir[s.stock] +=
              (s.target_position_ratio > 0.0) ? 1.0 : (s.target_position_ratio < 0.0 ? -1.0 : 0.0);
            continue;
        }
        double weight = 1.0;
        auto it = sys_weight.find(s.sys);
        if (it != sys_weight.end()) {
            weight = it->second;
        } else if (!sys_weight.empty()) {
            weight = 1.0 / sys_weight.size();
        }
        double v = weight * s.target_position_ratio * parent_net;
        if (!std::isfinite(v)) {
            HKU_WARN("{}::_toTargets: non-finite target value of {}, skipped!", m_name,
                     s.stock.market_code());
            continue;
        }
        target_value[s.stock] += v;
        gross += std::fabs(v);
    }
    if (fixed_amount > 0.0) {
        // The aggregate-then-apply step: one ±fixed-amount target per instrument (a flat aggregate
        // means the conflicting intents cancel out, so the instrument is driven to zero)
        for (const auto& kv : fixed_dir) {
            if (kv.second == 0.0) {
                continue;
            }
            double target = (kv.second > 0.0) ? fixed_amount : -fixed_amount;
            target_value[kv.first] = target;
            gross += std::fabs(target);
        }
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
                                   TradeSuggestionList& suggestions, const KQuery& query,
                                   const string& mode) {
    // L3 portfolio risk control (enabled by default in mode A; mode B and mode C respect the
    // autonomy of the sub-strategy, only the total amount check is performed = no clipping).
    if (mode == "B" || mode == "C") {
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
