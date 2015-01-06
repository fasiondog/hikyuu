/*
 * SlippageBase.cpp
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#include "SlippageBase.h"

namespace hku {

HKU_API std::ostream & operator<<(std::ostream& os, const SlippageBase& sp) {
    os << "Slippage(" << sp.name() << ", " << sp.getParameter() << ")";
    return os;
}

HKU_API std::ostream & operator<<(std::ostream& os, const SlippagePtr& sp) {
    if (sp) {
        os << *sp;
    } else {
        os << "Slippage(NULL)";
    }
    return os;
}

SlippagePtr SlippageBase::clone() {
    SlippagePtr p = _clone();
    p->m_params = m_params;
    p->m_name = m_name;
    p->m_kdata = m_kdata;
    return p;
}

void SlippageBase::setTO(const KData& kdata) {
    reset();
    m_kdata = kdata;
    if (!kdata.empty()) {
        _calculate();
    }
}

} /* namespace hku */
