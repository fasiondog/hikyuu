/*
 * MoneyManagerBase.cpp
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#include "MoneyManagerBase.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const MoneyManagerBase& mm) {
    os << "MoneyManager(" << mm.name() << ", " << mm.getParameter() << ")";
    return os;
}

HKU_API std::ostream& operator<<(std::ostream& os, const MoneyManagerPtr& mm) {
    if (mm) {
        os << *mm;
    } else {
        os << "MoneyManager(NULL)";
    }

    return os;
}

MoneyManagerBase::MoneyManagerBase() : m_name("MoneyManagerBase") {
    setParam<bool>("auto-checkin", false);
    setParam<int>("max-stock", 20000);
    setParam<bool>("disable_ev_force_clean_position", false);
    setParam<bool>("disable_cn_force_clean_position", false);
    setParam<double>("max-single-position", 1.0);
}

MoneyManagerBase::MoneyManagerBase(const string& name) : m_name(name) {
    setParam<bool>("auto-checkin", false);
    setParam<int>("max-stock", 20000);
    setParam<bool>("disable_ev_force_clean_position", false);
    setParam<bool>("disable_cn_force_clean_position", false);
    setParam<double>("max-single-position", 1.0);
}

MoneyManagerBase::~MoneyManagerBase() {}

void MoneyManagerBase::baseCheckParam(const string& name) const {
    if ("max-stock" == name) {
        HKU_ASSERT(getParam<int>("max-stock") >= 1);
    }
}

void MoneyManagerBase::paramChanged() {}

void MoneyManagerBase::reset() {
    m_query = Null<KQuery>();
    m_tm.reset();
    m_buy_sell_counts.clear();
    _reset();
}

MoneyManagerPtr MoneyManagerBase::clone() {
    MoneyManagerPtr p;
    try {
        p = _clone();
    } catch (...) {
        HKU_ERROR("Subclass _clone failed!");
        p = MoneyManagerPtr();
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
    p->m_buy_sell_counts = m_buy_sell_counts;
    return p;
}

double MoneyManagerBase::getSellNumber(const Datetime& datetime, const Stock& stock, price_t price,
                                       price_t risk, SystemPart from) {
    HKU_ERROR_IF_RETURN(!m_tm, 0.0,
                        "m_tm is null! Datetime({}) Stock({}) price({:<.4f}) risk({:<.2f})",
                        datetime, stock.market_code(), price, risk);

    if (PART_ENVIRONMENT == from) {
        // 强制全部卖出
        HKU_IF_RETURN(!getParam<bool>("disable_ev_force_clean_position"), MAX_DOUBLE);
    }

    if (PART_CONDITION == from) {
        HKU_IF_RETURN(!getParam<bool>("disable_cn_force_clean_position"), MAX_DOUBLE);
    }

    // 如果风险小于等于0，则忽略
    HKU_IF_RETURN(risk <= 0.0, 0.0);

    return _getSellNumber(datetime, stock, price, risk, from);
}

double MoneyManagerBase::getBuyNumber(const Datetime& datetime, const Stock& stock, price_t price,
                                      price_t risk, SystemPart from) {
    HKU_ERROR_IF_RETURN(!m_tm, 0.0,
                        "m_tm is null! Datetime({}) Stock({}) price({:<.3f}) risk({:<.2f})",
                        datetime, stock.market_code(), price, risk);
    HKU_ERROR_IF_RETURN(stock.isNull(), 0.0, "stock is Null!");

    HKU_INFO_IF_RETURN(risk <= 0.0, 0.0,
                       "risk less zero (Mayby single-line price board, can ignored)! "
                       "Datetime({}) Stock({} {}) price({:<.3f}) risk({:<.2f}) Part({})",
                       datetime, stock.market_code(), stock.name(), price, risk,
                       getSystemPartName(from));

    HKU_TRACE_IF_RETURN(m_tm->getStockNumber() >= getParam<int>("max-stock"), 0.0,
                        "Ignore! TM had max-stock number!");

    double n = _getBuyNumber(datetime, stock, price, risk, from);
    double min_trade = stock.minTradeNumber();
    HKU_TRACE_IF_RETURN(n < min_trade, 0.0,
                        "Ignore! Is less than the minimum number of transactions({:<.4f}<{}) {}", n,
                        min_trade, stock.market_code());

    // 转换为最小交易量的整数倍
    n = int64_t(n / min_trade) * min_trade;

    double max_trade = stock.maxTradeNumber();
    HKU_WARN_IF_RETURN(n > max_trade, max_trade,
                       "Over stock.maxTradeNumber({}), will use maxTradeNumber", max_trade);

    // 在现金不足时，自动补充存入现金
    if (getParam<bool>("auto-checkin")) {
        price_t cash = m_tm->cash(datetime, m_query.kType());
        CostRecord cost = m_tm->getBuyCost(datetime, stock, price, n);
        int precision = m_tm->precision();
        price_t money = roundUp(price * n * stock.unit() + cost.total, precision);
        if (money > cash) {
            m_tm->checkin(datetime, roundUp(money - cash, precision));
        }
    } else {
        CostRecord cost = m_tm->getBuyCost(datetime, stock, price, n);
        price_t need_cash = n * price + cost.total;
        price_t current_cash = m_tm->cash(datetime, m_query.kType());
        while (n > min_trade && need_cash > current_cash) {
            n = n - min_trade;
            cost = m_tm->getBuyCost(datetime, stock, price, n);
            need_cash = n * price + cost.total;
        }
        if (need_cash > current_cash) {
            n = 0.0;
        }
    }

    return n;
}

double MoneyManagerBase::getSellShortNumber(const Datetime& datetime, const Stock& stock,
                                            price_t price, price_t risk, SystemPart from) {
    HKU_ERROR_IF_RETURN(!m_tm, 0.0,
                        "m_tm is null! Datetime({}) Stock({}) price({:<.3f}) risk({:<.2f})",
                        datetime, stock.market_code(), price, risk);
    HKU_ERROR_IF_RETURN(risk >= 0.0, 0.0,
                        "risk is positive! Datetime({}) Stock({}) price({:<.3f}) risk({:<.2f})",
                        datetime, stock.market_code(), price, risk);
    return _getSellShortNumber(datetime, stock, price, risk, from);
}

double MoneyManagerBase ::getBuyShortNumber(const Datetime& datetime, const Stock& stock,
                                            price_t price, price_t risk, SystemPart from) {
    HKU_ERROR_IF_RETURN(!m_tm, 0.0,
                        "m_tm is null! Datetime({}) Stock({}) price({:<.3f}) risk({:<.2f})",
                        datetime, stock.market_code(), price, risk);
    HKU_ERROR_IF_RETURN(risk >= 0.0, 0.0,
                        "risk is positive! Datetime({}) Stock({}) price({:<.3f}) risk({:<.2f})",
                        datetime, stock.market_code(), price, risk);
    return _getBuyShortNumber(datetime, stock, price, risk, from);
}

double MoneyManagerBase::_getSellNumber(const Datetime& datetime, const Stock& stock, price_t price,
                                        price_t risk, SystemPart from) {
    // 默认卖出全部
    return MAX_DOUBLE;
}

double MoneyManagerBase::_getSellShortNumber(const Datetime& datetime, const Stock& stock,
                                             price_t price, price_t risk, SystemPart from) {
    return 0;
}

double MoneyManagerBase::_getBuyShortNumber(const Datetime& datetime, const Stock& stock,
                                            price_t price, price_t risk, SystemPart from) {
    // 默认全部平仓
    return MAX_DOUBLE;
}

size_t MoneyManagerBase::currentBuyCount(const Stock& stk) const {
    const auto iter = m_buy_sell_counts.find(stk);
    return iter == m_buy_sell_counts.cend() ? 0 : iter->second.first;
}

size_t MoneyManagerBase::currentSellCount(const Stock& stk) const {
    const auto iter = m_buy_sell_counts.find(stk);
    return iter == m_buy_sell_counts.cend() ? 0 : iter->second.second;
}

void MoneyManagerBase::buyNotify(const TradeRecord& tr) {
    auto iter = m_buy_sell_counts.find(tr.stock);
    if (iter == m_buy_sell_counts.end()) {
        m_buy_sell_counts[tr.stock] = std::make_pair<size_t, size_t>(1, 0);
    } else {
        iter->second.first++;
        iter->second.second = 0;
    }
    _buyNotify(tr);
}

void MoneyManagerBase::sellNotify(const TradeRecord& tr) {
    auto iter = m_buy_sell_counts.find(tr.stock);
    if (iter == m_buy_sell_counts.end()) {
        m_buy_sell_counts[tr.stock] = std::make_pair<size_t, size_t>(0, 1);
    } else {
        iter->second.first = 0;
        iter->second.second++;
    }
    _sellNotify(tr);
}

//============================================================================
// 组合级资金分配（MM L1/L2/L3），供聚合 System（MultiSystem）调用
//============================================================================

void MoneyManagerBase::allocate(const Datetime& date, const TradeManagerPtr& tm,
                                TradeSuggestionList& suggestions, SubSystemContextList& contexts,
                                const KQuery& query) {
    auto weights = _allocateSystemWeight(date, tm, contexts, query);
    _allocateSuggestions(date, tm, suggestions, weights, query);
    _checkRisk(date, tm, suggestions, query);
}

std::unordered_map<SYSPtr, double> MoneyManagerBase::_allocateSystemWeight(
    const Datetime& date, const TradeManagerPtr& tm, SubSystemContextList& contexts,
    const KQuery& query) {
    std::unordered_map<SYSPtr, double> weights;
    if (contexts.empty()) {
        return weights;
    }
    double eq = 1.0 / contexts.size();
    for (auto& ctx : contexts) {
        weights[ctx.sys] = eq;
        if (m_mode == "B" && tm) {
            // 模式 B：L1 产出「真实额度」= 等权 × 父总资产，写入 contexts[i].quota，
            // 由父在调仓日回写给子系统（供下期运行，额度滞后一期）。
            ctx.quota = eq * tm->getFunds(date, query.kType()).total_assets();
        }
    }
    return weights;
}

void MoneyManagerBase::_allocateSuggestions(const Datetime& date, const TradeManagerPtr& tm,
                                            TradeSuggestionList& suggestions,
                                            const std::unordered_map<SYSPtr, double>& sys_weight,
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
            // 模式 A（默认）：目标持仓市值 = 权重 × 父总资产，换算为目标股数，number 改写为（目标 - 当前）的净调仓量
            double target_value = weight * total_assets;
            double target_shares = target_value / s.plan_price;
            s.number = target_shares - current;
        } else {
            // SELL / CLEAR：退出该标的（卖出当前全部持仓）
            s.number = -current;
        }
    }
}

void MoneyManagerBase::_checkRisk(const Datetime& date, const TradeManagerPtr& tm,
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
