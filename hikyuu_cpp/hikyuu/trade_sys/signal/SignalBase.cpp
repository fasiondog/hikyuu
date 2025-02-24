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
    initParam();
}

SignalBase::SignalBase(const string& name) : m_name(name), m_hold_long(false), m_hold_short(false) {
    initParam();
}

SignalBase::~SignalBase() {}

void SignalBase::initParam() {
    setParam<bool>("cycle", false);                 // 仅在指定周期范围内计算
    setParam<bool>("alternate", true);              // 买入卖出信号交替出现
    setParam<bool>("support_borrow_stock", false);  // 支持发出空头信号
}

void SignalBase::baseCheckParam(const string& name) const {}
void SignalBase::paramChanged() {
    m_calculated = false;
}

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
    p->m_calculated = m_calculated;
    p->m_hold_long = m_hold_long;
    p->m_hold_short = m_hold_short;
    p->m_buySig = m_buySig;
    p->m_sellSig = m_sellSig;
    p->m_cycle_start = m_cycle_start;
    p->m_cycle_end = m_cycle_start;
    return p;
}

void SignalBase::setTO(const KData& kdata) {
    HKU_IF_RETURN(m_calculated && m_kdata == kdata, void());
    m_kdata = kdata;
    m_calculated = false;
    HKU_IF_RETURN(kdata.empty(), void());

    bool cycle = getParam<bool>("cycle");
    m_cycle_start = kdata[0].datetime;

    if (!cycle) {
        _calculate(kdata);
    }

    m_calculated = true;
}

void SignalBase::reset() {
    m_kdata = Null<KData>();
    m_buySig.clear();
    m_sellSig.clear();
    m_hold_long = false;
    m_hold_short = false;
    m_cycle_start = Null<Datetime>();
    m_cycle_end = Null<Datetime>();
    _reset();
}

void SignalBase::startCycle(const Datetime& start, const Datetime& close) {
    HKU_IF_RETURN(!getParam<bool>("cycle"), void());
    HKU_CHECK(start != Null<Datetime>() && close != Null<Datetime>() && start < close, "{}",
              m_name);
    HKU_CHECK(start >= m_cycle_end || m_cycle_end == Null<Datetime>(),
              "curretn start: {}, pre cycle end: {}", start, m_cycle_end);
    m_cycle_start = start;
    m_cycle_end = close;
    KData kdata = m_kdata.getKData(start, close);
    if (!kdata.empty()) {
        _calculate(kdata);
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
    HKU_IF_RETURN(iszero(value) || std::isnan(value), void());

    double new_value = value + getBuyValue(datetime) + getSellValue(datetime);
    HKU_IF_RETURN(iszero(new_value), void());

    if (new_value > 0.0) {
        auto iter = m_buySig.find(datetime);
        if (!getParam<bool>("alternate")) {
            if (iter != m_buySig.end()) {
                iter->second += new_value;
            } else {
                m_buySig.insert({datetime, new_value});
            }
            return;
        }

        if (!m_hold_long) {
            if (iter != m_buySig.end()) {
                iter->second += new_value;
            } else {
                m_buySig.insert({datetime, new_value});
            }
            if (getParam<bool>("support_borrow_stock") && m_hold_short) {
                m_hold_short = false;
            } else {
                m_hold_long = true;
            }
        }

    } else {
        auto iter = m_sellSig.find(datetime);
        if (!getParam<bool>("alternate")) {
            if (iter != m_sellSig.end()) {
                iter->second += new_value;
            } else {
                m_sellSig.insert({datetime, new_value});
            }
            return;
        }

        if (!m_hold_short) {
            if (m_hold_long) {
                if (iter != m_sellSig.end()) {
                    iter->second += new_value;
                } else {
                    m_sellSig.insert({datetime, new_value});
                }
                m_hold_long = false;
            } else if (getParam<bool>("support_borrow_stock")) {
                if (iter != m_sellSig.end()) {
                    iter->second += new_value;
                } else {
                    m_sellSig.insert({datetime, new_value});
                }
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
