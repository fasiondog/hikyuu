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

#include "../../utilities/arithmetic.h"

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
    setParam<double>("max_single_position", 1.0);

    // The total budget cap: in mode A the aggregated gross target market value of all instruments
    // <= net assets x this value, scaled down proportionally when exceeded; in mode B the sum of
    // the sub-system quotas <= total assets x this value (<=0 disables the cap). The reserved /
    // unallocated asset ratio of the master reserve_percent is expressed by setting it to 1 - r.
    setParam<double>("max_total_position", 1.0);

    // The pricing policy switch: when >0 the fixed amount takes precedence over the weights --
    // the per-instrument target in mode A / the per sub-system quota in mode B are both this
    // amount, and the L1 weights are ignored in the pricing. The sum is still bounded by
    // max_total_position: the fixed amount is the pricing unit, not a risk-control exemption
    setParam<double>("fixed_amount", 0.0);

    // Whether to force-adjust the running sub-systems (the ones still holding value funded in a
    // previous period) to the new quota on the rebalancing day. Only effective in mode B: mode A
    // re-targets every bar by design (always adjusts) and mode C has no quota at all (never
    // adjusts). true - the full calibration (reduce the over quota / inject the gap); false - the
    // running sub-systems keep their current holdings, only the new sub-systems get the remaining
    // parent cash
    setParam<bool>("adjust_running_sys", true);
}

AllocateFundsBase::AllocateFundsBase(const string& name) : m_name(name) {
    setParam<double>("max_single_position", 1.0);
    setParam<double>("max_total_position", 1.0);
    setParam<double>("fixed_amount", 0.0);
    setParam<bool>("adjust_running_sys", true);
}

AllocateFundsBase::~AllocateFundsBase() {}

void AllocateFundsBase::baseCheckParam(const string& name) const {
    string fixed_name;
    if (name == "max-single-position") {
        fixed_name = "max_single_position";
    } else if (name == "max-total-position") {
        fixed_name = "max_total_position";
    } else if (name == "fixed-amount") {
        fixed_name = "fixed_amount";
    }
    HKU_CHECK(fixed_name.empty(), "Deprecated hyphenated parameter name '{}'! use '{}' instead",
              name, fixed_name);

    if (name == "max_single_position" || name == "max_total_position") {
        // <=0 disables the cap and >1 is clamped in use, but a NaN silently disables both risk
        // controls
        HKU_ASSERT(!std::isnan(getParam<double>(name)));
    } else if (name == "fixed_amount") {
        // 0 disables the fixed amount; a negative or non-finite value would silently fall back to
        // the weight pricing
        double v = getParam<double>(name);
        HKU_ASSERT(std::isfinite(v) && v >= 0.0);
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
    // The pricing policy: a fixed quota per sub-system when fixed_amount>0 (AF_FixedAmount),
    // otherwise weight x the parent net assets (the parent valuation is needed only there).
    double fixed_amount = getParam<double>("fixed_amount");
    if (fixed_amount > 0.0 && !m_warned_fixed_amount) {
        m_warned_fixed_amount = true;
        HKU_WARN("{}: fixed_amount > 0 takes precedence over the weights in the pricing!", m_name);
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
        // The mode B budget cap: the weight pricing is a proportion, so the quotas are scaled down
        // proportionally; the fixed_amount pricing is an absolute amount per sub-system, so the
        // number of funded sub-systems is truncated instead and every funded one keeps its full
        // amount (<=0 disables the cap)
        double max_total = getParam<double>("max_total_position");
        double cap = (max_total > 0.0 && net_assets > 0.0) ? net_assets * max_total : -1.0;
        size_t quota_count = 0;
        for (const auto& ctx : contexts) {
            if (ctx.sys) {
                ++quota_count;
            }
        }
        size_t fixed_fit = quota_count;
        if (cap >= 0.0) {
            if (fixed_amount > 0.0) {
                // The epsilon keeps an exact multiple from losing a slot to the float noise
                double n = std::floor(cap / fixed_amount + 1e-9);
                if (n < static_cast<double>(quota_count)) {
                    fixed_fit = static_cast<size_t>(n);
                    HKU_WARN(
                      "{}: max_total_position({}) x net assets({:.2f}) funds only {} of the {} "
                      "sub-systems at fixed_amount({}), the rest get a zero quota!",
                      m_name, max_total, net_assets, fixed_fit, quota_count, fixed_amount);
                }
            } else if (std::isfinite(sum_w) && sum_w * net_assets > cap * (1.0 + 1e-9)) {
                double scale = cap / (sum_w * net_assets);
                HKU_WARN(
                  "{}: the aggregated quota({:.2f}) exceeds max_total_position({}) x net "
                  "assets({:.2f}), scaled by {:.4f}!",
                  m_name, sum_w * net_assets, max_total, net_assets, scale);
                for (auto& kv : result) {
                    kv.second *= scale;
                }
            }
        }
        size_t funded = 0;
        for (auto& ctx : contexts) {
            if (!ctx.sys) {
                continue;
            }
            // Mode B: L1 produces the "real quota" and writes it into contexts[i].quota, the parent
            // writes it back to the sub-system on the rebalancing day (for the next period, the
            // quota lags one period behind).
            auto it = result.find(ctx.sys);
            double w = (it != result.end()) ? it->second : 0.0;
            if (fixed_amount > 0.0) {
                ctx.quota = (funded < fixed_fit) ? fixed_amount : 0.0;
                ++funded;
            } else {
                ctx.quota = w * net_assets;
            }
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
        // short-side SELL opens/increases a short and has no long position to clip against. The
        // same-batch BUY legs of the instrument are part of the available quantity: the batch is
        // netted at execution, so a same-bar open fill followed by a close reversal is legal
        std::map<Stock, double> batch_buy;
        for (const auto& s : suggestions) {
            if (s.type == SuggestionType::BUY && !s.short_side) {
                batch_buy[s.stock] += std::abs(s.number);
            }
        }
        for (auto& s : suggestions) {
            if (s.type == SuggestionType::SELL && !s.short_side) {
                double current = tm ? tm->getPosition(date, s.stock).number : 0.0;
                current += batch_buy[s.stock];
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
    // Only the instruments present in the suggestions are re-targeted: an instrument with no
    // suggestion keeps the parent's current position untouched (the parent book is assumed to
    // mirror the sub books, so AF does not chase orphan holdings).
    double parent_net = tm->getFunds(date, ktype).net_assets();
    double fixed_amount = getParam<double>("fixed_amount");
    double max_total = getParam<double>("max_total_position");

    std::map<Stock, double> target_value;  // instrument -> signed net target market value
    std::map<Stock, double> fixed_dir;  // instrument -> aggregated signed direction (fixed_amount)
    std::map<Stock, price_t> price;     // instrument -> the close price used for the shares
    std::vector<Stock> fixed_order;  // first-appearance order, drives the fixed_amount truncation
    for (const auto& s : suggestions) {
        if (s.stock.isNull() || s.plan_price <= 0.0) {
            continue;
        }
        // The first suggestion of an instrument fixes the price used for the share conversion: a
        // later one carrying another price is ignored, so the outcome never depends on the order of
        // the tail. A mismatch means the sub-systems priced different bars, only a debug notice
        auto pit = price.find(s.stock);
        if (pit == price.end()) {
            price[s.stock] = s.plan_price;
        } else if (!nearly_equal(pit->second, s.plan_price)) {
            HKU_DEBUG("{}::_toTargets: {} has conflicting plan prices ({}, {}), the first is used!",
                      m_name, s.stock.market_code(), pit->second, s.plan_price);
        }
        if (fixed_amount > 0.0) {
            // fixed_amount: the pricing is per instrument, not per suggestion. The same instrument
            // held by N sub-systems must not be scaled to N x fixed_amount, so only the aggregate
            // signed direction is accumulated here and the ±fixed_amount target is applied once
            // per instrument below.
            if (fixed_dir.find(s.stock) == fixed_dir.end()) {
                fixed_order.push_back(s.stock);
            }
            fixed_dir[s.stock] +=
              (s.target_position_ratio > 0.0) ? 1.0 : (s.target_position_ratio < 0.0 ? -1.0 : 0.0);
            continue;
        }
        // The table is keyed by the System instance: an unlisted sub-system (missing key, empty
        // table or clone) is dropped, a parent-synthesized one (no sys) keeps 1.0
        double weight = 1.0;
        if (s.sys) {
            auto it = sys_weight.find(s.sys);
            if (it == sys_weight.end()) {
                HKU_WARN(
                  "{}::_toTargets: no weight for the sub-system of {}, the suggestion is "
                  "skipped!",
                  m_name, s.stock.market_code());
                continue;
            }
            weight = it->second;
        }
        double v = weight * s.target_position_ratio * parent_net;
        if (!std::isfinite(v)) {
            HKU_WARN("{}::_toTargets: non-finite target value of {}, skipped!", m_name,
                     s.stock.market_code());
            continue;
        }
        target_value[s.stock] += v;
    }
    // The parent cannot run a short book without a securities-lending facility: clamp the negative
    // net targets to zero and warn, so a short order is never silently rejected by the TM
    const bool support_borrow =
      tm->haveParam("support_borrow_stock") && tm->getParam<bool>("support_borrow_stock");

    if (fixed_amount > 0.0) {
        // The aggregate-then-apply step: one ±fixed_amount target per instrument (a flat aggregate
        // means the conflicting intents cancel out, so the instrument is driven to zero). The cap
        // truncates the number of funded instruments in first-appearance order instead of shrinking
        // every target. Only executable targets are charged: an unexecutable short leg (clamped
        // away below) and a target below one board lot (dropped by the opening-leg filter later)
        // must not consume a quota that another instrument could use
        double remaining = (max_total > 0.0 && parent_net > 0.0) ? parent_net * max_total : -1.0;
        size_t eligible = 0;
        size_t funded = 0;
        for (const auto& stk : fixed_order) {
            double dir = fixed_dir[stk];
            if (dir == 0.0) {
                target_value[stk] = 0.0;
                continue;
            }
            if (dir < 0.0 && !support_borrow) {
                if (!m_warned_short_unsupported) {
                    m_warned_short_unsupported = true;
                    HKU_WARN(
                      "{}::_toTargets: the parent account does not support borrowing securities, "
                      "the negative net targets are clamped to zero! Set support_borrow_stock=true "
                      "on the system to allow shorting.",
                      m_name);
                }
                target_value[stk] = 0.0;
                continue;
            }
            auto pit = price.find(stk);
            double p = (pit != price.end()) ? pit->second : 0.0;
            double unit = stk.unit();
            double min_trade = stk.minTradeNumber();
            if (p <= 0.0 || unit <= 0.0 ||
                (min_trade > 0.0 && fixed_amount < min_trade * p * unit)) {
                target_value[stk] = 0.0;
                continue;
            }
            ++eligible;
            double target = (dir > 0.0) ? fixed_amount : -fixed_amount;
            if (remaining >= 0.0 && std::fabs(target) > remaining * (1.0 + 1e-9)) {
                target_value[stk] = 0.0;
                continue;
            }
            if (remaining >= 0.0) {
                remaining -= std::fabs(target);
            }
            ++funded;
            target_value[stk] = target;
        }
        if (funded < eligible) {
            HKU_WARN(
              "{}::_toTargets: max_total_position({}) x net assets({:.2f}) funds only {} of the "
              "{} fixed_amount targets, the rest are driven flat!",
              m_name, max_total, parent_net, funded, eligible);
        }
    }

    if (!support_borrow) {
        for (auto& kv : target_value) {
            if (kv.second < 0.0) {
                if (!m_warned_short_unsupported) {
                    m_warned_short_unsupported = true;
                    HKU_WARN(
                      "{}::_toTargets: the parent account does not support borrowing securities, "
                      "the negative net targets are clamped to zero! Set support_borrow_stock=true "
                      "on the system to allow shorting.",
                      m_name);
                }
                kv.second = 0.0;
            }
        }
    }

    // Measured on the aggregated target so hedging legs of one instrument cannot inflate the
    // gross; the clamped and the plain path share this computation
    double gross = 0.0;
    for (const auto& kv : target_value) {
        gross += std::fabs(kv.second);
    }

    // The total budget cap: the aggregated gross target market value <= net assets x
    // max_total_position, scaled down proportionally (<=0 disables the cap). Only the weight
    // pricing reaches here with an over-budget gross: the fixed_amount targets are already
    // truncated above, so their gross never exceeds the cap
    if (max_total > 0.0 && parent_net > 0.0 && gross > parent_net * max_total) {
        double scale = parent_net * max_total / gross;
        HKU_WARN(
          "{}::_toTargets: the aggregated gross target({:.2f}) exceeds max_total_position({}) x "
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
            HKU_WARN("{}::_toTargets: skip {}, no quote or no contract unit!", m_name,
                     stock.market_code());
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
    // The exchange rules: an opening order (a long buy / a short sell / a short cover) must be a
    // whole board lot while a long sell may be an odd lot; drop the sub-lot opening legs (the TM
    // would refuse them anyway) instead of leaving a silent hole
    TradeSuggestionList filtered;
    filtered.reserve(orders.size());
    for (auto& o : orders) {
        const bool needs_lot =
          (o.type == SuggestionType::BUY) || (o.type == SuggestionType::SELL && o.short_side);
        double min_trade = o.stock.minTradeNumber();
        if (needs_lot && min_trade > 0.0 && o.number < min_trade) {
            HKU_WARN("Skip the opening leg of {} ({:.0f}), it is below one lot!",
                     o.stock.market_code(), o.number);
            continue;
        }
        filtered.push_back(o);
    }
    suggestions = filtered;
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
    // net assets x max_single_position (<=0 means no limit; >1 is clamped to 1). The net assets
    // are the denominator so that a levered account is not granted a larger cap by the borrowed
    // funds.
    double max_ratio = getParam<double>("max_single_position");
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
            s.plan_cash = s.number * s.plan_price * u;  // Keep the exposed suggestion consistent
        }
    }
}

} /* namespace hku */
