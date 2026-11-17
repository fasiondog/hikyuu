/*
 * MoneyManagerBase.cpp
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#include <cmath>
#include "MoneyManagerBase.h"
#include "hikyuu/StockManager.h"

namespace hku {

// Values above this cannot be cast to int64_t: INT64_MAX is 9.22e18 and the cast of anything
// beyond it (or of NaN/inf) is undefined behaviour
constexpr double MAX_INT64_CAST_VALUE{9.0e18};

namespace {

// The number of the trading days in (from, to] (weekends and exchange holidays excluded),
// stopping at the given limit
size_t tradingDaysIn(const Datetime& from, const Datetime& to, size_t limit) {
    size_t count = 0;
    for (Datetime d = from + TimeDelta(1); d <= to && count < limit; d = d + TimeDelta(1)) {
        if (!StockManager::instance().isHoliday(d)) {
            ++count;
        }
    }
    return count;
}

}  // namespace

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
    setParam<bool>("auto_checkin", false);
    setParam<int>("max_stock", 20000);
    setParam<bool>("disable_ev_force_clean_position", false);
    setParam<bool>("disable_cn_force_clean_position", false);

    // T+n: the shares bought within the last n trading days cannot be sold (1 = the T+1 rule,
    // 0 turns it off); the daily KType or above is unaffected
    setParam<int>("t_plus_n", 1);
}

MoneyManagerBase::MoneyManagerBase(const string& name) : m_name(name) {
    setParam<bool>("auto_checkin", false);
    setParam<int>("max_stock", 20000);
    setParam<bool>("disable_ev_force_clean_position", false);
    setParam<bool>("disable_cn_force_clean_position", false);

    // T+n: see the default constructor
    setParam<int>("t_plus_n", 1);
}

MoneyManagerBase::~MoneyManagerBase() {}

void MoneyManagerBase::baseCheckParam(const string& name) const {
    HKU_CHECK(name != "auto-checkin" && name != "max-stock",
              "Deprecated hyphenated parameter name '{}'! use '{}' instead", name,
              name == "auto-checkin" ? "auto_checkin" : "max_stock");
    if ("max_stock" == name) {
        HKU_ASSERT(getParam<int>("max_stock") >= 1);
    } else if ("t_plus_n" == name) {
        HKU_ASSERT(getParam<int>("t_plus_n") >= 0);
    }
}

void MoneyManagerBase::paramChanged() {}

void MoneyManagerBase::reset() {
    m_query = KQuery();
    m_tm.reset();
    // The EV/CN are re-assigned by the system on every run
    m_buy_sell_counts.clear();
    m_tplus_buys.clear();
    _reset();
}

MoneyManagerPtr MoneyManagerBase::clone() {
    MoneyManagerPtr p = _clone();
    HKU_CHECK(p && p.get() != this,
              "Failed clone! The subclass _clone of {} must return a new object!", m_name);

    p->m_params = m_params;
    p->m_name = m_name;
    p->m_is_python_object = m_is_python_object;
    p->m_support_mult_buy_sell = m_support_mult_buy_sell;
    p->m_tm = m_tm;
    p->m_ev = m_ev;
    p->m_cn = m_cn;
    p->m_query = m_query;
    p->m_buy_sell_counts = m_buy_sell_counts;
    p->m_tplus_buys = m_tplus_buys;
    return p;
}

double MoneyManagerBase::getSellNumber(const Datetime& datetime, const Stock& stock, price_t price,
                                       price_t risk, SystemPart from) {
    HKU_ERROR_IF_RETURN(!m_tm, 0.0, "m_tm is null! Datetime({}) price({:<.4f}) risk({:<.2f})",
                        datetime, price, risk);

    if (PART_ENVIRONMENT == from) {
        // Force selling everything
        HKU_IF_RETURN(!getParam<bool>("disable_ev_force_clean_position"), MAX_DOUBLE);
    }

    if (PART_CONDITION == from) {
        HKU_IF_RETURN(!getParam<bool>("disable_cn_force_clean_position"), MAX_DOUBLE);
    }

    HKU_ERROR_IF_RETURN(stock.isNull(), 0.0, "stock is Null!");
    HKU_ERROR_IF_RETURN(
      !(price > 0.0), 0.0, "Invalid price! Datetime({}) Stock({} {}) price({:<.3f}) Part({})",
      datetime, stock.market_code(), stock.name(), price, getSystemPartName(from));

    // When the risk is not positive (NaN included, i.e. the price has reached or fallen below
    // the stop loss price), the MMs which do not support multi-trading liquidate the whole
    // position directly, otherwise the subclass _getSellNumber is left to decide
    if (!(risk > 0.0) && !m_support_mult_buy_sell) {
        // The stop-loss liquidation cannot close the T+n locked shares either
        return _capSellNumber(datetime, stock, MAX_DOUBLE);
    }

    double n = _capSellNumber(datetime, stock, _getSellNumber(datetime, stock, price, risk, from));

    // The "sell everything" sentinel is resolved to the position by the trade manager
    HKU_IF_RETURN(n == MAX_DOUBLE, MAX_DOUBLE);
    HKU_WARN_IF_RETURN(!std::isfinite(n) || n <= 0.0, 0.0,
                       "Invalid sell number({})! Datetime({}) Stock({} {}) price({:<.3f})", n,
                       datetime, stock.market_code(), stock.name(), price);

    // Reject a dirty maxTradeNumber (NaN / +-Inf / negative) before the cap comparison, following
    // getBuyNumber
    double max_trade = stock.maxTradeNumber();
    HKU_WARN_IF_RETURN(!std::isfinite(max_trade) || !(max_trade > 0.0), 0.0,
                       "Invalid stock.maxTradeNumber({})! Stock({} {})", max_trade,
                       stock.market_code(), stock.name());
    if (n > max_trade) {
        HKU_WARN("Over stock.maxTradeNumber({}), will use maxTradeNumber", max_trade);
        n = max_trade;
    }
    // The size is neither clamped to the holding (the caller sells against the position it holds)
    // nor aligned to minTradeNumber: a dividend or a stock gift may leave an odd lot
    return n;
}

bool MoneyManagerBase::_isDailyOrAboveQuery() const {
    return KQuery::getKTypeInSeconds(m_query.kType()) >= 86400;
}

double MoneyManagerBase::_capSellNumber(const Datetime& datetime, const Stock& stock, double n) {
    HKU_IF_RETURN(getParam<int>("t_plus_n") <= 0 || _isDailyOrAboveQuery(), n);
    double locked = _getTplusLockedNumber(datetime, stock);
    HKU_IF_RETURN(locked <= 0.0, n);
    double sellable = m_tm->getHoldNumber(datetime, stock) - locked;
    // A part below minTradeNumber (e.g. the odd lot left by a dividend) would be rejected by the
    // trade manager, so treat it as nothing sellable
    HKU_IF_RETURN(sellable < stock.minTradeNumber(), 0.0);
    return n > sellable ? sellable : n;
}

double MoneyManagerBase::_getTplusLockedNumber(const Datetime& datetime, const Stock& stock) {
    auto iter = m_tplus_buys.find(stock);
    HKU_IF_RETURN(iter == m_tplus_buys.end(), 0.0);
    auto& entries = iter->second;
    HKU_IF_RETURN(entries.empty(), 0.0);

    _trimTplusCache(entries, datetime.startOfDay(), getParam<int>("t_plus_n"));

    double locked = 0.0;
    for (const auto& entry : entries) {
        locked += entry.second;
    }
    return locked;
}

void MoneyManagerBase::_trimTplusCache(std::vector<std::pair<Datetime, double>>& entries,
                                       const Datetime& today, int n) {
    // The entries are appended with non-decreasing dates, so the expired ones sit at the front
    while (!entries.empty()) {
        const Datetime& buy_date = entries.front().first;
        // T+1: any later trading day unlocks, no holiday lookup needed
        bool expired =
          n <= 1 ? today > buy_date : tradingDaysIn(buy_date, today, size_t(n)) >= size_t(n);
        if (!expired) {
            break;
        }
        entries.erase(entries.begin());
    }
}

double MoneyManagerBase::getBuyNumber(const Datetime& datetime, const Stock& stock, price_t price,
                                      price_t risk, SystemPart from) {
    HKU_ERROR_IF_RETURN(!m_tm, 0.0, "m_tm is null! Datetime({}) price({:<.3f}) risk({:<.2f})",
                        datetime, price, risk);
    HKU_ERROR_IF_RETURN(stock.isNull(), 0.0, "stock is Null!");

    // Protects all the money managers from a division by zero or an invalid price propagated by
    // dirty data, so that no inf/nan position count can be produced
    HKU_ERROR_IF_RETURN(
      !(price > 0.0), 0.0, "Invalid price! Datetime({}) Stock({} {}) price({:<.3f}) Part({})",
      datetime, stock.market_code(), stock.name(), price, getSystemPartName(from));

    // A NaN risk (e.g. from a NaN stop-loss price) fails the "risk <= 0.0" test and would be
    // propagated down to the int64_t cast of the trade quantity
    HKU_INFO_IF_RETURN(!(risk > 0.0), 0.0,
                       "risk not positive (Maybe single-line price board, can ignored)! "
                       "Datetime({}) Stock({} {}) price({:<.3f}) risk({:<.2f}) Part({})",
                       datetime, stock.market_code(), stock.name(), price, risk,
                       getSystemPartName(from));

    HKU_TRACE_IF_RETURN(m_tm->getStockNumber() >= getParam<int>("max_stock"), 0.0,
                        "Ignore! TM had max_stock number!");

    double n = _getBuyNumber(datetime, stock, price, risk, from);
    double min_trade = stock.minTradeNumber();
    HKU_TRACE_IF_RETURN(n < min_trade, 0.0,
                        "Ignore! Is less than the minimum number of transactions({:<.4f}<{}) {}", n,
                        min_trade, stock.market_code());

    // A non-finite quotient (NaN, or a division by a dirty minTradeNumber) and an out-of-range
    // one both make the int64_t cast below undefined behaviour
    double lot_count = n / min_trade;
    HKU_WARN_IF_RETURN(
      !std::isfinite(lot_count) || !(lot_count >= 0.0) || lot_count > MAX_INT64_CAST_VALUE, 0.0,
      "Invalid buy number! Datetime({}) Stock({} {}) price({:<.3f}) number({}) "
      "minTradeNumber({:<.4f})",
      datetime, stock.market_code(), stock.name(), price, n, min_trade);

    // Convert it into an integer multiple of the minimum trade quantity
    n = int64_t(lot_count) * min_trade;

    // Reject a dirty maxTradeNumber (NaN / +-Inf / negative) before the cap comparison: against
    // NaN or +Inf that comparison is always false, so a guard nested in the cap branch could never
    // fire
    double max_trade = stock.maxTradeNumber();
    HKU_WARN_IF_RETURN(!std::isfinite(max_trade) || !(max_trade > 0.0), 0.0,
                       "Invalid stock.maxTradeNumber({})! Stock({} {})", max_trade,
                       stock.market_code(), stock.name());
    if (n > max_trade) {
        HKU_WARN("Over stock.maxTradeNumber({}), will use maxTradeNumber", max_trade);
        n = max_trade;
    }

    // With auto_checkin System deposits the cash at the actual trade, so the number is not
    // constrained by the current cash here
    if (!getParam<bool>("auto_checkin")) {
        // Mirror the affordability check of TradeManager::buy: the amount and the cost are both
        // rounded to the account precision, otherwise a size a fraction of a cent above the cash
        // passes here and gets the whole order rejected there
        const int precision = m_tm->precision();
        auto needCash = [&](double num) {
            CostRecord cost = m_tm->getBuyCost(datetime, stock, price, num);
            return roundEx(roundEx(num * price * stock.unit(), precision) + cost.total, precision);
        };
        price_t current_cash = m_tm->cash(datetime, m_query.kType());
        if (needCash(n) > current_cash) {
            // needCash(k) is monotonically increasing in k, so binary search the largest
            // affordable multiple of min_trade — O(log(n/min_trade)) cost evaluations instead of
            // decrementing lot by lot
            double low = min_trade, high = n;
            while (high - low > min_trade) {
                double mid = int64_t((low + high) / (2.0 * min_trade)) * min_trade;
                if (mid <= low || mid >= high) {
                    break;
                }
                if (needCash(mid) <= current_cash) {
                    low = mid;
                } else {
                    high = mid;
                }
            }
            n = low;
            if (needCash(n) > current_cash) {
                n = 0.0;
            }
        }
    }

    return n;
}

double MoneyManagerBase::getSellShortNumber(const Datetime& datetime, const Stock& stock,
                                            price_t price, price_t risk, SystemPart from) {
    HKU_ERROR_IF_RETURN(!m_tm, 0.0, "m_tm is null! Datetime({}) price({:<.3f}) risk({:<.2f})",
                        datetime, price, risk);
    HKU_ERROR_IF_RETURN(stock.isNull(), 0.0, "stock is Null!");
    HKU_ERROR_IF_RETURN(
      !(price > 0.0), 0.0, "Invalid price! Datetime({}) Stock({} {}) price({:<.3f}) Part({})",
      datetime, stock.market_code(), stock.name(), price, getSystemPartName(from));
    // For a short sell the stop-loss is above the entry, so the per-share risk (stoploss - price)
    // must be positive; a non-positive risk (NaN included) means the price has already reached
    // the stop-loss
    HKU_INFO_IF_RETURN(!(risk > 0.0), 0.0,
                       "risk not positive (Maybe single-line price board, can ignored)! "
                       "Datetime({}) Stock({} {}) price({:<.3f}) risk({:<.2f}) Part({})",
                       datetime, stock.market_code(), stock.name(), price, risk,
                       getSystemPartName(from));

    double n = _getSellShortNumber(datetime, stock, price, risk, from);

    // Align to the lot size and clamp to maxTradeNumber, mirroring getBuyNumber: the trade
    // manager rejects a fractional or an oversized quantity
    double min_trade = stock.minTradeNumber();
    HKU_TRACE_IF_RETURN(n < min_trade, 0.0,
                        "Ignore! Is less than the minimum number of transactions({:<.4f}<{}) {}", n,
                        min_trade, stock.market_code());

    double lot_count = n / min_trade;
    HKU_WARN_IF_RETURN(
      !std::isfinite(lot_count) || !(lot_count >= 0.0) || lot_count > MAX_INT64_CAST_VALUE, 0.0,
      "Invalid sell short number! Datetime({}) Stock({} {}) price({:<.3f}) number({}) "
      "minTradeNumber({:<.4f})",
      datetime, stock.market_code(), stock.name(), price, n, min_trade);

    n = int64_t(lot_count) * min_trade;

    double max_trade = stock.maxTradeNumber();
    HKU_WARN_IF_RETURN(!std::isfinite(max_trade) || !(max_trade > 0.0), 0.0,
                       "Invalid stock.maxTradeNumber({})! Stock({} {})", max_trade,
                       stock.market_code(), stock.name());
    if (n > max_trade) {
        HKU_WARN("Over stock.maxTradeNumber({}), will use maxTradeNumber", max_trade);
        return max_trade;
    }

    return n;
}

double MoneyManagerBase::getBuyShortNumber(const Datetime& datetime, const Stock& stock,
                                           price_t price, price_t risk, SystemPart from) {
    HKU_ERROR_IF_RETURN(!m_tm, 0.0, "m_tm is null! Datetime({}) price({:<.3f}) risk({:<.2f})",
                        datetime, price, risk);

    if (PART_ENVIRONMENT == from) {
        // Force covering the whole short position
        HKU_IF_RETURN(!getParam<bool>("disable_ev_force_clean_position"), MAX_DOUBLE);
    }

    if (PART_CONDITION == from) {
        HKU_IF_RETURN(!getParam<bool>("disable_cn_force_clean_position"), MAX_DOUBLE);
    }

    HKU_ERROR_IF_RETURN(stock.isNull(), 0.0, "stock is Null!");
    HKU_ERROR_IF_RETURN(
      !(price > 0.0), 0.0, "Invalid price! Datetime({}) Stock({} {}) price({:<.3f}) Part({})",
      datetime, stock.market_code(), stock.name(), price, getSystemPartName(from));

    // When the risk is not positive (NaN included, i.e. the price has risen to or above the
    // stop-loss price), the MMs which do not support multi-trading cover the whole position
    HKU_IF_RETURN(!(risk > 0.0) && !m_support_mult_buy_sell, MAX_DOUBLE);

    return _getBuyShortNumber(datetime, stock, price, risk, from);
}

double MoneyManagerBase::_getSellNumber(const Datetime& datetime, const Stock& stock, price_t price,
                                        price_t risk, SystemPart from) {
    // Sell everything by default
    return MAX_DOUBLE;
}

double MoneyManagerBase::_getSellShortNumber(const Datetime& datetime, const Stock& stock,
                                             price_t price, price_t risk, SystemPart from) {
    return 0;
}

double MoneyManagerBase::_getBuyShortNumber(const Datetime& datetime, const Stock& stock,
                                            price_t price, price_t risk, SystemPart from) {
    // Liquidate everything by default
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
    // Only the long-side opening updates the consecutive trade counters; the short-side trades
    // (BUSINESS_BUY_SHORT etc.) must not pollute the long tranches
    if (tr.business == BUSINESS_BUY) {
        auto iter = m_buy_sell_counts.find(tr.stock);
        if (iter == m_buy_sell_counts.end()) {
            m_buy_sell_counts[tr.stock] = std::make_pair<size_t, size_t>(1, 0);
        } else {
            iter->second.first++;
            iter->second.second = 0;
        }

        // The T+n cache: aggregate the fills of the same buy date; skipped on the daily KType or
        // above, where the rule is never applied
        if (getParam<int>("t_plus_n") > 0 && !_isDailyOrAboveQuery()) {
            auto& entries = m_tplus_buys[tr.stock];
            Datetime date = tr.datetime.startOfDay();
            if (!entries.empty() && entries.back().first == date) {
                entries.back().second += tr.number;
            } else {
                entries.emplace_back(date, tr.number);
            }
            _trimTplusCache(entries, date, getParam<int>("t_plus_n"));
        }
    }
    _buyNotify(tr);
}

void MoneyManagerBase::sellNotify(const TradeRecord& tr) {
    // Only the long-side closing updates the consecutive trade counters; the short-side trades
    // (BUSINESS_SELL_SHORT etc.) must not pollute the long tranches
    if (tr.business == BUSINESS_SELL) {
        auto iter = m_buy_sell_counts.find(tr.stock);
        if (iter == m_buy_sell_counts.end()) {
            m_buy_sell_counts[tr.stock] = std::make_pair<size_t, size_t>(0, 1);
        } else {
            iter->second.first = 0;
            iter->second.second++;
        }
    }
    _sellNotify(tr);
}

} /* namespace hku */
