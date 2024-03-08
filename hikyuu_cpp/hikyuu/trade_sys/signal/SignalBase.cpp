/*
 * SignalBase.cpp
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#include "SignalBase.h"

namespace hku {

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

SignalBase::SignalBase()
: m_name("SignalBase"),
  m_hold_long(false),
  m_hold_short(false),
  m_last_buy_pos(Null<size_t>()),
  m_last_sell_pos(Null<size_t>()) {
    setParam<bool>("alternate", true);              // 买入卖出信号交替出现
    setParam<bool>("support_borrow_stock", false);  // 支持发出空头信号
}

SignalBase::SignalBase(const string& name)
: m_name(name),
  m_hold_long(false),
  m_hold_short(false),
  m_last_buy_pos(Null<size_t>()),
  m_last_sell_pos(Null<size_t>()) {
    setParam<bool>("alternate", true);
    setParam<bool>("support_borrow_stock", false);
}

SignalBase::~SignalBase() {}

SignalPtr SignalBase::clone() {
    SignalPtr p;
    try {
        p = _clone();
    } catch (...) {
        HKU_ERROR("Subclass _clone failed!");
        p = SignalPtr();
    }

    if (!p || p.get() == this) {
        HKU_ERROR("Failed clone! Will use self-ptr!");
        return shared_from_this();
    }

    p->m_name = m_name;
    p->m_params = m_params;
    p->m_kdata = m_kdata;
    p->m_hold_long = m_hold_long;
    p->m_hold_short = m_hold_short;
    p->m_buySig = m_buySig;
    p->m_sellSig = m_sellSig;

    p->m_date_index = m_date_index;
    p->m_dates = m_dates;
    p->m_values = m_values;
    p->m_last_buy_pos = m_last_buy_pos;
    p->m_last_sell_pos = m_last_sell_pos;
    return p;
}

void SignalBase::setTO(const KData& kdata) {
    reset();
    m_kdata = kdata;
    if (!kdata.empty()) {
        _calculate();
    }
}

void SignalBase::reset() {
    m_buySig.clear();
    m_sellSig.clear();
    m_hold_long = false;
    m_hold_short = false;
    m_date_index.clear();
    m_dates.clear();
    m_values.clear();
    _reset();
}

DatetimeList SignalBase::getBuySignal() const {
    DatetimeList result(m_buySig.size());
    std::copy(m_buySig.begin(), m_buySig.end(), result.begin());
    return result;
}

DatetimeList SignalBase::getSellSignal() const {
    DatetimeList result(m_sellSig.size());
    std::copy(m_sellSig.begin(), m_sellSig.end(), result.begin());
    return result;
}

bool SignalBase::shouldBuy(const Datetime& datetime) const {
    return m_buySig.count(datetime) ? true : false;
}

bool SignalBase::shouldSell(const Datetime& datetime) const {
    return m_sellSig.count(datetime) ? true : false;
}

void SignalBase::_addBuySignal(const Datetime& datetime, price_t value) {
    if (!getParam<bool>("alternate")) {
        m_buySig.insert(datetime);
    } else {
        if (!m_hold_long) {
            m_buySig.insert(datetime);
            if (getParam<bool>("support_borrow_stock") && m_hold_short) {
                m_hold_short = false;
            } else {
                m_hold_long = true;
            }
        }
    }

    HKU_WARN_IF(!m_dates.empty() && datetime > m_dates.back(), "Please join in ascending order!");
    if (!getParam<bool>("alternate")) {
        m_date_index[datetime] = m_values.size();
        m_dates.emplace_back(datetime);
        m_values.push_back(value);
        m_last_buy_pos = m_values.size();

    } else {
        if (m_last_sell_pos > m_last_buy_pos || m_last_sell_pos == Null<size_t>()) {
            m_values.push_back(value);
            m_last_buy_pos = m_values.size();
            m_date_index[datetime] = m_last_buy_pos;
        }
    }
}

void SignalBase::_addSellSignal(const Datetime& datetime, price_t value) {
    if (!getParam<bool>("alternate")) {
        m_sellSig.insert(datetime);
    } else {
        if (!m_hold_short) {
            if (m_hold_long) {
                m_sellSig.insert(datetime);
                m_hold_long = false;
            } else if (getParam<bool>("support_borrow_stock")) {
                m_sellSig.insert(datetime);
                m_hold_short = true;
            }
        }
    }

    if (!getParam<bool>("alternate")) {
        m_date_index[datetime] = m_values.size();
        m_dates.emplace_back(datetime);
        m_values.push_back(value);
        m_last_sell_pos = m_values.size();
    } else {
        if (m_last_buy_pos > m_last_sell_pos || m_last_buy_pos == Null<size_t>()) {
            m_values.push_back(value);
            m_last_buy_pos = m_values.size();
            m_date_index[datetime] = m_last_buy_pos;
        }
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
