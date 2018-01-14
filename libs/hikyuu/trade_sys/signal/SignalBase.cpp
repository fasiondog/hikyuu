/*
 * SignalBase.cpp
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#include "SignalBase.h"

namespace hku {

HKU_API std::ostream & operator<<(std::ostream& os, const SignalBase& sg) {
    os << "Signal(" << sg.name() << ", " << sg.getParameter() << ")";
    return os;
}

HKU_API std::ostream & operator<<(std::ostream& os, const SignalPtr& sg) {
    if (sg) {
        os << *sg;
    } else {
        os << "Signal(NULL)";
    }

    return os;
}


SignalBase::SignalBase() : m_name("SignalBase"), m_hold(false) {
    setParam<bool>("alternate", true); //买入卖出信号交替出现
}

SignalBase::SignalBase(const string& name): m_name(name), m_hold(false) {
    setParam<bool>("alternate", true);
}

SignalBase::~SignalBase() {

}


SignalPtr SignalBase::clone() {
    SignalPtr p;
    try {
        p = _clone();
    } catch(...) {
        HKU_ERROR("Subclass _clone failed! [SignalBase::clone]");
        p = SignalPtr();
    }

    if (!p || p.get() == this) {
        HKU_ERROR("Failed clone! Will use self-ptr! [SignalBase::clone]" );
        return shared_from_this();
    }

    p->m_name = m_name;
    p->m_params = m_params;
    p->m_kdata = m_kdata;
    p->m_hold = m_hold;
    p->m_buySig = m_buySig;
    p->m_sellSig = m_sellSig;
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
    m_hold = false;
    _reset();
}


DatetimeList SignalBase::getBuySignal() const {
    DatetimeList result;
    result.reserve(m_buySig.size());
    std::set<Datetime>::const_iterator iter = m_buySig.begin();
    for (; iter != m_buySig.end(); ++iter) {
        result.push_back(*iter);
    }
    return result;
}


DatetimeList SignalBase::getSellSignal() const {
    DatetimeList result;
    result.reserve(m_sellSig.size());
    std::set<Datetime>::const_iterator iter = m_sellSig.begin();
    for (; iter != m_sellSig.end(); ++iter) {
        result.push_back(*iter);
    }
    return result;
}

void SignalBase::_addBuySignal(const Datetime& datetime) {
    if (!getParam<bool>("alternate")) {
        m_buySig.insert(datetime);
    } else {
        if (!m_hold) {
            m_buySig.insert(datetime);
            m_hold = true;
        }
    }
}

void SignalBase::_addSellSignal(const Datetime& datetime) {
    if (!getParam<bool>("alternate")) {
        m_sellSig.insert(datetime);
    } else {
        if (m_hold) {
            m_sellSig.insert(datetime);
            m_hold = false;
        }
    }
}


} /* namespace hku */
