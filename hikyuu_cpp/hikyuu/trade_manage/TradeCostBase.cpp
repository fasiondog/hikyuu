/*
 * TradeCostBase.cpp
 *
 *  Created on: 2013-2-13
 *      Author: fasiondog
 */

#include "TradeCostBase.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TradeCostBase)
#endif

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const TradeCostBase& tc) {
    os << "TradeCostFunc(" << tc.name() << ", " << tc.getParameter() << ")";
    return os;
}

HKU_API std::ostream& operator<<(std::ostream& os, const TradeCostPtr& tc) {
    if (tc) {
        os << *tc;
    } else {
        os << "TradeCostFunc(NULL)";
    }
    return os;
}

TradeCostBase::TradeCostBase(const string& name) : m_name(name) {}

TradeCostBase::~TradeCostBase() {}

TradeCostPtr TradeCostBase::clone() {
    TradeCostPtr result = _clone();
    TradeCostBase* p = result.get();
    p->m_params = m_params;
    p->m_name = m_name;
    return result;
}

} /* namespace hku */
