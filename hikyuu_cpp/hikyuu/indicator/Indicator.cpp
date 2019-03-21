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

string Indicator::formula() const {
    return m_imp ? m_imp->formula() : "Indicator";
}

void Indicator::setContext(const Stock& stock, const KQuery& query) {
    if (m_imp) m_imp->setContext(stock, query);
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

void Indicator::name(const string& name) {
    if (m_imp) {
        m_imp->name(name);
    }
}

string Indicator::long_name() const {
    return m_imp ? m_imp->long_name() : "IndicatorImp()";
}

size_t Indicator::discard() const {
    return m_imp ? m_imp->discard() : 0 ;
}

void Indicator::setDiscard(size_t discard) {
    if (m_imp)
        m_imp->setDiscard(discard);
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

Indicator Indicator::operator()(const Indicator& ind) {
    if (!m_imp)
        return Indicator();

    if (!ind.getImp())
        return Indicator(m_imp);
    
    IndicatorImpPtr p = m_imp->clone();
    p->add(IndicatorImp::OP, IndicatorImpPtr(), ind.getImp()->clone());
    return p->calculate();
    //m_imp->add(IndicatorImp::OP, IndicatorImpPtr(), ind.getImp());
    //return m_imp->calculate();
}

Indicator Indicator::operator+(const Indicator& ind) {
    if (!m_imp) {
        return Indicator();
    }

    if (!ind.getImp()) {
        return Indicator(m_imp->clone());
    }

    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::ADD, m_imp->clone(), ind.getImp()->clone());
    return p->calculate();
}

Indicator Indicator::operator-(const Indicator& ind) {
    if (!m_imp) {
        return Indicator();
    }

    if (!ind.getImp()) {
        return Indicator(m_imp->clone());
    }

    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::SUB, m_imp->clone(), ind.getImp()->clone());
    return p->calculate();
}

Indicator Indicator::operator*(const Indicator& ind) {
    if (!m_imp) {
        return Indicator();
    }

    if (!ind.getImp()) {
        return Indicator(m_imp->clone());
    }

    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::MUL, m_imp->clone(), ind.getImp()->clone());
    return p->calculate();
}

Indicator Indicator::operator/(const Indicator& ind) {
    if (!m_imp) {
        return Indicator();
    }

    if (!ind.getImp()) {
        return Indicator(m_imp->clone());
    }

    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::DIV, m_imp->clone(), ind.getImp()->clone());
    return p->calculate();
}

Indicator Indicator::operator==(const Indicator& ind) {
    if (!m_imp) {
        return Indicator();
    }

    if (!ind.getImp()) {
        return Indicator(m_imp->clone());
    }

    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::EQ, m_imp->clone(), ind.getImp()->clone());
    return p->calculate();
}

Indicator Indicator::operator!=(const Indicator& ind) {
    if (!m_imp) {
        return Indicator();
    }

    if (!ind.getImp()) {
        return Indicator(m_imp->clone());
    }

    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::NE, m_imp->clone(), ind.getImp()->clone());
    return p->calculate();
}

Indicator Indicator::operator>(const Indicator& ind) {
    if (!m_imp) {
        return Indicator();
    }

    if (!ind.getImp()) {
        return Indicator(m_imp->clone());
    }

    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::GT, m_imp->clone(), ind.getImp()->clone());
    return p->calculate();
}

Indicator Indicator::operator<(const Indicator& ind) {
    if (!m_imp) {
        return Indicator();
    }

    if (!ind.getImp()) {
        return Indicator(m_imp->clone());
    }

    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::LT, m_imp->clone(), ind.getImp()->clone());
    return p->calculate();
}

Indicator Indicator::operator>=(const Indicator& ind) {
    if (!m_imp) {
        return Indicator();
    }

    if (!ind.getImp()) {
        return Indicator(m_imp->clone());
    }

    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::GE, m_imp->clone(), ind.getImp()->clone());
    return p->calculate();
}

Indicator Indicator::operator<=(const Indicator& ind) {
    if (!m_imp) {
        return Indicator();
    }

    if (!ind.getImp()) {
        return Indicator(m_imp->clone());
    }

    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::LE, m_imp->clone(), ind.getImp()->clone());
    return p->calculate();
}

Indicator Indicator::operator&(const Indicator& ind) {
    if (!m_imp) {
        return Indicator();
    }

    if (!ind.getImp()) {
        return Indicator(m_imp->clone());
    }

    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::AND, m_imp->clone(), ind.getImp()->clone());
    return p->calculate();
}

Indicator Indicator::operator|(const Indicator& ind) {
    if (!m_imp) {
        return Indicator();
    }

    if (!ind.getImp()) {
        return Indicator(m_imp->clone());
    }

    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::OR, m_imp->clone(), ind.getImp()->clone());
    return p->calculate();
}

} /* namespace hku */
