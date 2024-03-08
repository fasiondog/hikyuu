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

SignalBase::SignalBase() : m_name("SignalBase"), m_hold_long(false), m_hold_short(false) {
    setParam<bool>("alternate", true);              // 买入卖出信号交替出现
    setParam<bool>("support_borrow_stock", false);  // 支持发出空头信号
}

SignalBase::SignalBase(const string& name) : m_name(name), m_hold_long(false), m_hold_short(false) {
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
    p->m_values = m_values;
    return p;
}

void SignalBase::setTO(const KData& kdata) {
    reset();
    m_kdata = kdata;
    if (!kdata.empty()) {
        size_t total = kdata.size();
        m_values.resize(total);
        memset(m_values.data(), 0, sizeof(price_t) * total);
        auto const* ks = m_kdata.data();
        for (size_t i = 0; i < total; i++) {
            m_date_index[ks[i].datetime] = i;
        }
        _calculate();
    }
}

void SignalBase::reset() {
    m_buySig.clear();
    m_sellSig.clear();
    m_hold_long = false;
    m_hold_short = false;
    m_date_index.clear();
    m_values.clear();
    _reset();
}

DatetimeList SignalBase::getBuySignal() const {
    DatetimeList result;
    result.reserve(m_buySig.size());
    for (auto iter = m_buySig.begin(), end = m_buySig.end(); iter != end; ++iter) {
        result.push_back(iter->first);
    }
    return result;
}

DatetimeList SignalBase::getSellSignal() const {
    DatetimeList result(m_sellSig.size());
    result.reserve(m_sellSig.size());
    for (auto iter = m_sellSig.begin(), end = m_sellSig.end(); iter != end; ++iter) {
        result.push_back(iter->first);
    }
    return result;
}

bool SignalBase::shouldBuy(const Datetime& datetime) const {
    return m_buySig.count(datetime) ? true : false;
}

bool SignalBase::shouldSell(const Datetime& datetime) const {
    return m_sellSig.count(datetime) ? true : false;
}

void SignalBase::_addSignal(const Datetime& datetime, price_t value) {
    if (value > 0.0) {
        _addBuySignal(datetime, value);
    } else if (value < 0.0) {
        _addSellSignal(datetime, value);
    } else {
        // 已经初始化为0
    }
}

void SignalBase::_addBuySignal(const Datetime& datetime, price_t value) {
    auto iter = m_date_index.find(datetime);
    HKU_IF_RETURN(iter == m_date_index.end(), void());

    size_t pos = iter->second;
    m_values[pos] += value;

    if (!getParam<bool>("alternate")) {
        m_buySig[datetime] = pos;

    } else {
        if (!m_hold_long) {
            m_buySig[datetime] = pos;
            if (getParam<bool>("support_borrow_stock") && m_hold_short) {
                m_hold_short = false;
            } else {
                m_hold_long = true;
            }
        }
    }
}

void SignalBase::_addSellSignal(const Datetime& datetime, price_t value) {
    auto iter = m_date_index.find(datetime);
    HKU_IF_RETURN(iter == m_date_index.end(), void());

    size_t pos = iter->second;
    m_values[pos] += value;

    if (!getParam<bool>("alternate")) {
        m_sellSig[datetime] = pos;
    } else {
        if (!m_hold_short) {
            if (m_hold_long) {
                m_sellSig[datetime] = pos;
                m_hold_long = false;
            } else if (getParam<bool>("support_borrow_stock")) {
                m_sellSig[datetime] = pos;
                m_hold_short = true;
            }
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
