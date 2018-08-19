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

EnvironmentBase::EnvironmentBase()
: m_name("EnvironmentBase") {

}

EnvironmentBase::EnvironmentBase(const string& name)
: m_name(name) {

}

EnvironmentBase::~EnvironmentBase() {

}

void EnvironmentBase::reset() {
    m_valid.clear();
    _reset();
}

EnvironmentPtr EnvironmentBase::clone() {
    EnvironmentPtr p;
    try {
        p = _clone();
    } catch(...) {
        HKU_ERROR("Subclass _clone failed! [EnvironmentBase::clone]");
        p = EnvironmentPtr();
    }

    if (!p || p.get() == this) {
        HKU_ERROR("Failed clone! Will use self-ptr! [EnvironmentBase::clone]" );
        return shared_from_this();
    }

    p->m_params = m_params;
    p->m_name = m_name;
    p->m_query = m_query;
    p->m_valid = m_valid;
    return p;
}

void EnvironmentBase::setQuery(const KQuery& query) {
    m_query = query;
    _calculate();
}

void EnvironmentBase::_addValid(const Datetime& datetime) {
    m_valid.insert(datetime);
}

bool EnvironmentBase::isValid(const Datetime& datetime) {
    if (m_valid.count(datetime) != 0) {
        return true;
    }
    return false;
}

} /* namespace hku */
