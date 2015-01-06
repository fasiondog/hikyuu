/*
 * StoplossBase.cpp
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#include "StoplossBase.h"

namespace hku {

HKU_API std::ostream& operator <<(std::ostream& os, const StoplossBase& sl) {
    os << "Stoploss(" << sl.name() << ", " << sl.getParameter() << ")";
    return os;
}

HKU_API std::ostream& operator <<(std::ostream& os, const StoplossPtr& sl) {
    if (sl) {
        os << *sl;
    } else {
        os << "Stoploss(NULL)";
    }
    return os;
}


StoplossPtr StoplossBase::clone() {
    StoplossPtr p = _clone();
    p->m_params = m_params;
    p->m_name = m_name;
    p->m_tm = m_tm;
    p->m_kdata = m_kdata;
    return p;
}


void StoplossBase::setTO(const KData& kdata) {
    reset();
    m_kdata = kdata;
    if (!kdata.empty()) {
        _calculate();
    }
}

} /* namespace hku */
