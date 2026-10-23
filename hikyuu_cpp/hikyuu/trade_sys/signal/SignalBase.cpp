/*
 * SignalBase.cpp
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#include <algorithm>
#include <cmath>
#include "SignalBase.h"

namespace hku {

namespace {
// Signals below this magnitude are float residuals (e.g. sg*0.5 - sg/2), not real intents
constexpr double ZERO_SIGNAL_THRESHOLD = 1e-10;
}  // namespace

HKU_API std::ostream& operator<<(std::ostream& os, const SignalBase& sg) {
    os << "Signal(" << sg.name() << ", " << sg.getParameter() << ")";
    return os;
}

HKU_API std::ostream& operator<<(std::ostream& os, const SignalPtr& sg) {
    if (sg) {
        os << *sg;
    } else {
        os << "Signal(NULL)";
    }

    return os;
}

SignalBase::SignalBase() : m_name("SignalBase"), m_hold_long(false), m_hold_short(false) {
    initParam();
}

SignalBase::SignalBase(const string& name) : m_name(name), m_hold_long(false), m_hold_short(false) {
    initParam();
}

SignalBase::~SignalBase() {}

void SignalBase::initParam() {
    setParam<bool>("cycle", false);  // A marker of the cycle signals: they consume the rebalancing
                                     // days declared by a MultiSystem (it no longer limits the
                                     // calculation range)
    setParam<bool>("alternate", true);              // The buy and sell signals appear alternately
    setParam<bool>("support_borrow_stock", false);  // Support issuing a short signal
}

void SignalBase::baseCheckParam(const string& name) const {}
void SignalBase::paramChanged() {
    m_calculated = false;
    _normalizeCycleAlternate();
}

void SignalBase::_normalizeCycleAlternate() {
    HKU_IF_RETURN(!tryGetParam<bool>("cycle", false), void());
    HKU_IF_RETURN(!tryGetParam<bool>("alternate", true), void());
    HKU_WARN("The cycle signal {} ignores alternate=true: a scheduled signal must not alternate!",
             m_name);
    m_params.set<bool>("alternate", false);
}

SignalPtr SignalBase::clone() {
    SignalPtr p = _clone();
    HKU_CHECK(p && p.get() != this,
              "Failed clone! The subclass _clone of {} must return a new object!", m_name);

    p->m_name = m_name;
    p->m_params = m_params;
    p->m_is_python_object = m_is_python_object;
    p->m_keepSignals = m_keepSignals;
    p->m_kdata = m_kdata;
    // The clone recalculates lazily so the subclass runtime state (e.g. the Python subclass
    // cache) is rebuilt on the next setTO instead of keeping the stale snapshot
    p->m_calculated = false;
    p->m_hold_long = m_hold_long;
    p->m_hold_short = m_hold_short;
    p->m_buySig = m_buySig;
    p->m_sellSig = m_sellSig;
    p->m_cycle_start = m_cycle_start;
    p->m_cycle_end = m_cycle_end;
    p->m_cycle_dates = m_cycle_dates;
    return p;
}

void SignalBase::setTO(const KData& kdata) {
    HKU_IF_RETURN(m_calculated && m_kdata == kdata, void());
    m_kdata = kdata;
    m_calculated = false;

    if (!m_keepSignals) {
        m_buySig.clear();
        m_sellSig.clear();
        m_hold_long = false;
        m_hold_short = false;
    }

    HKU_IF_RETURN(kdata.empty(), void());

    _calculate(kdata);

    m_calculated = true;
}

void SignalBase::reset() {
    m_kdata = Null<KData>();
    if (!m_keepSignals) {
        m_buySig.clear();
        m_sellSig.clear();
        m_hold_long = false;
        m_hold_short = false;
    }
    m_cycle_start = Null<Datetime>();
    m_cycle_end = Null<Datetime>();
    m_cycle_dates.clear();
    _reset();
}

void SignalBase::startCycle(const Datetime& start, const Datetime& close) {
    // Only a cycle signal consumes the declared cycle days; the other signals keep their full-range
    // calculation untouched
    HKU_IF_RETURN(!getParam<bool>("cycle"), void());
    HKU_CHECK(!start.isNull() && !close.isNull() && start < close, "{}", m_name);
    HKU_CHECK(start >= m_cycle_end || m_cycle_end.isNull(), "curretn start: {}, pre cycle end: {}",
              start, m_cycle_end);
    m_cycle_start = start;
    m_cycle_end = close;

    DatetimeList dates = m_cycle_dates;
    dates.push_back(start);
    setCycleDates(dates);
}

void SignalBase::setCycleDates(const DatetimeList& dates) {
    HKU_IF_RETURN(dates == m_cycle_dates, void());

    DatetimeList sorted;
    sorted.reserve(dates.size());
    for (const auto& date : dates) {
        if (!date.isNull()) {
            sorted.push_back(date);
        }
    }
    std::sort(sorted.begin(), sorted.end());
    sorted.erase(std::unique(sorted.begin(), sorted.end()), sorted.end());
    HKU_IF_RETURN(sorted == m_cycle_dates, void());

    m_cycle_dates = std::move(sorted);
    m_calculated = false;
    if (!m_kdata.empty()) {
        setTO(m_kdata);
    }
}

DatetimeList SignalBase::getBuySignal() const {
    DatetimeList result;
    result.reserve(m_buySig.size());
    for (auto iter = m_buySig.begin(); iter != m_buySig.end(); ++iter) {
        result.emplace_back(iter->first);
    }
    return result;
}

DatetimeList SignalBase::getSellSignal() const {
    DatetimeList result;
    result.reserve(m_sellSig.size());
    for (auto iter = m_sellSig.begin(); iter != m_sellSig.end(); ++iter) {
        result.emplace_back(iter->first);
    }
    return result;
}

double SignalBase::getBuyValue(const Datetime& datetime) const {
    auto iter = m_buySig.find(datetime);
    return iter != m_buySig.end() ? iter->second : 0.0;
}
double SignalBase::getSellValue(const Datetime& datetime) const {
    auto iter = m_sellSig.find(datetime);
    return iter != m_sellSig.end() ? iter->second : 0.0;
}

void SignalBase::_addSignal(const Datetime& datetime, double value) {
    HKU_IF_RETURN(!std::isfinite(value) || std::abs(value) < ZERO_SIGNAL_THRESHOLD, void());

    auto buy_iter = m_buySig.find(datetime);
    auto sell_iter = m_sellSig.find(datetime);
    double buy_value = buy_iter != m_buySig.end() ? buy_iter->second : 0.0;
    double sell_value = sell_iter != m_sellSig.end() ? sell_iter->second : 0.0;
    double new_value = value + buy_value + sell_value;
    HKU_IF_RETURN(!std::isfinite(new_value), void());

    if (std::abs(new_value) < ZERO_SIGNAL_THRESHOLD) {
        // A zero net leaves no direction: drop the records already written for the day. The hold
        // flags keep the alternation sequence and are not derived from the tables
        m_buySig.erase(datetime);
        m_sellSig.erase(datetime);
        return;
    }

    // One day carries one direction only: the written table receives the merged net value and the
    // opposite record of the same day is dropped, otherwise both shouldBuy and shouldSell hold.
    // When the alternation state rejects a new entry, a record already flagged for the day is
    // still refreshed with the net value (it is not a new entry, only a value update)
    bool alternate = getParam<bool>("alternate");
    if (new_value > 0.0) {
        if (!alternate || !m_hold_long) {
            m_sellSig.erase(datetime);
            m_buySig[datetime] = new_value;
            if (alternate) {
                if (getParam<bool>("support_borrow_stock") && m_hold_short) {
                    m_hold_short = false;
                } else {
                    m_hold_long = true;
                }
            }
        } else if (buy_iter != m_buySig.end()) {
            m_sellSig.erase(datetime);
            m_buySig[datetime] = new_value;
        }
        return;
    }

    bool allow_sell =
      !alternate || (!m_hold_short && (m_hold_long || getParam<bool>("support_borrow_stock")));
    if (allow_sell) {
        m_buySig.erase(datetime);
        m_sellSig[datetime] = new_value;
        if (alternate) {
            if (m_hold_long) {
                m_hold_long = false;
            } else {
                m_hold_short = true;
            }
        }
    } else if (sell_iter != m_sellSig.end()) {
        m_buySig.erase(datetime);
        m_sellSig[datetime] = new_value;
    }
}

bool SignalBase::nextTimeShouldBuy() const {
    size_t total = m_kdata.size();
    HKU_IF_RETURN(total == 0, false);
    return shouldBuy(m_kdata[total - 1].datetime);
}

bool SignalBase::nextTimeShouldSell() const {
    size_t total = m_kdata.size();
    HKU_IF_RETURN(total == 0, false);
    return shouldSell(m_kdata[total - 1].datetime);
}

} /* namespace hku */
