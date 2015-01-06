/*
 * Indicator.cpp
 *
 *  Created on: 2012-10-15
 *      Author: fasiondog
 */

#include "Indicator.h"

namespace hku {

HKU_API std::ostream & operator<<(std::ostream& os, const Indicator& indicator) {
    os << indicator.m_imp;
    return os;
}


Indicator::Indicator(const IndicatorImpPtr& imp): m_imp(imp) {

}


Indicator::Indicator(const Indicator& indicator) {
    m_imp = indicator.m_imp;
}


Indicator::~Indicator() {

}


Indicator& Indicator::operator=(const Indicator& indicator) {
    if (this == &indicator)
        return *this;

    if (m_imp != indicator.m_imp)
        m_imp = indicator.m_imp;

    return *this;
}


string Indicator::name() const {
    return m_imp ? m_imp->name() : "IndicatorImp";
}

string Indicator::long_name() const {
    return m_imp ? m_imp->long_name() : "IndicatorImp()";
}

size_t Indicator::discard() const {
    return m_imp ? m_imp->discard() : 0 ;
}

size_t Indicator::getResultNumber() const {
    return m_imp ? m_imp->getResultNumber() : 0;
}

bool Indicator::empty() const {
    return (!m_imp || m_imp->size() == 0) ? true : false;
}

size_t Indicator::size() const {
    return m_imp ? m_imp->size() : 0;
}

HKU_API Indicator operator+(const Indicator& ind1, const Indicator& ind2) {
    if (ind1.size() != ind2.size() || ind1.size() == 0) {
        return Indicator();
    }

    size_t result_number = std::min(ind1.getResultNumber(), ind2.getResultNumber());
    size_t total = ind1.size();
    int discard = std::max(ind1.discard(), ind2.discard());
    IndicatorImpPtr imp;
    imp = IndicatorImpPtr(new IndicatorImp(ind1, discard, result_number));
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            imp->_set(ind1.get(i, r) + ind2.get(i, r), i, r);
        }
    }

    return Indicator(imp);
}


HKU_API Indicator operator-(const Indicator& ind1, const Indicator& ind2) {
    if (ind1.size() != ind2.size() || ind1.size() == 0) {
        return Indicator();
    }

    size_t result_number = std::min(ind1.getResultNumber(), ind2.getResultNumber());
    size_t total = ind1.size();
    int discard = std::max(ind1.discard(), ind2.discard());
    IndicatorImpPtr imp;
    imp = IndicatorImpPtr(new IndicatorImp(ind1, discard, result_number));
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            imp->_set(ind1.get(i, r) - ind2.get(i, r), i, r);
        }
    }

    return Indicator(imp);
}


HKU_API Indicator operator*(const Indicator& ind1, const Indicator& ind2) {
    if (ind1.size() != ind2.size() || ind1.size() == 0) {
        return Indicator();
    }

    size_t result_number = std::min(ind1.getResultNumber(), ind2.getResultNumber());
    size_t total = ind1.size();
    int discard = std::max(ind1.discard(), ind2.discard());
    IndicatorImpPtr imp;
    imp = IndicatorImpPtr(new IndicatorImp(ind1, discard, result_number));
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            imp->_set(ind1.get(i, r) * ind2.get(i, r), i, r);
        }
    }

    return Indicator(imp);
}


HKU_API Indicator operator/(const Indicator& ind1, const Indicator& ind2) {
    if (ind1.size() != ind2.size() || ind1.size() == 0) {
        return Indicator();
    }

    size_t result_number = std::min(ind1.getResultNumber(), ind2.getResultNumber());
    size_t total = ind1.size();
    int discard = std::max(ind1.discard(), ind2.discard());
    IndicatorImpPtr imp;
    imp = IndicatorImpPtr(new IndicatorImp(ind1, discard, result_number));
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            if (ind2.get(i, r) == 0.0) {
                imp->_set(Null<price_t>(), i, r);
            } else {
                imp->_set(ind1.get(i, r) / ind2.get(i, r), i, r);
            }
        }
    }

    return Indicator(imp);
}

} /* namespace hku */
