/*
 * Environment.cpp
 *
 *  Created on: 2013-2-28
 *      Author: fasiondog
 */

#include "EnvironmentBase.h"

namespace hku {

HKU_API std::ostream& operator <<(std::ostream &os, const EnvironmentBase& en) {
    os << "Environment(" << en.name() << " " << en.getParameter() << ")";
    return os;
}

HKU_API std::ostream& operator <<(std::ostream &os, const EnvironmentPtr& en) {
    if (en) {
        os << *en;
    } else {
        os << "Environment(NULL)";
    }
    return os;
}

EnvironmentBase::EnvironmentBase(const string& name)
: m_name(name), m_ktype(KQuery::DAY) {

}

EnvironmentBase::~EnvironmentBase() {

}

EnvironmentPtr EnvironmentBase::clone() {
    EnvironmentPtr p = _clone();
    p->m_params = m_params;
    p->m_name = m_name;
    p->m_ktype = m_ktype;
    return p;
}

} /* namespace hku */
