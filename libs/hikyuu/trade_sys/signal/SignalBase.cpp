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


SignalBase::SignalBase(const string& name): m_name(name) {

}

SignalBase::~SignalBase() {

}


SignalPtr SignalBase::clone() {
    SignalPtr p = _clone();
    p->m_params = m_params;
    p->m_name = m_name;
    p->m_kdata = m_kdata;
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

} /* namespace hku */
