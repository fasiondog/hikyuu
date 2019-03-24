/*
 * Indicator.cpp
 *
 *  Created on: 2012-10-15
 *      Author: fasiondog
 */

#include "Indicator.h"
#include "crt/CVAL.h"

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

Indicator Indicator::clone() const {
    return m_imp ? Indicator(m_imp->clone()) : Indicator();
}

PriceList Indicator::getResultAsPriceList(size_t num) const {
    if (!m_imp) {
        HKU_WARN("indicator imptr is null! [Indicator::getResultAsPriceList]");
        return PriceList();
    }
    return m_imp->getResultAsPriceList(num);
}

Indicator Indicator::getResult(size_t num) const {
    if (!m_imp) {
        HKU_WARN("indicator imptr is null! [Indicator::getResult]");
        return Indicator();
    }
    return m_imp->getResult(num);
}

Indicator Indicator::operator()(const Indicator& ind) {
    if (!m_imp)
        return Indicator();

    if (!ind.getImp())
        return Indicator(m_imp);
    
    IndicatorImpPtr p = m_imp->clone();
    p->add(IndicatorImp::OP, IndicatorImpPtr(), ind.getImp());
    return p->calculate();
}

HKU_API Indicator operator+(const Indicator& ind1, const Indicator& ind2) {
    if (!ind1.getImp() || !ind2.getImp()) {
        return Indicator();
    }

    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::ADD, ind1.getImp(), ind2.getImp());
    return p->calculate();
}

HKU_API Indicator operator-(const Indicator& ind1, const Indicator& ind2) {
    if (!ind1.getImp() || !ind2.getImp()) {
        return Indicator();
    }

    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::SUB, ind1.getImp(), ind2.getImp());
    return p->calculate();
}

HKU_API Indicator operator*(const Indicator& ind1, const Indicator& ind2) {
    if (!ind1.getImp() || !ind2.getImp()) {
        return Indicator();
    }

    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::MUL, ind1.getImp(), ind2.getImp());
    return p->calculate();
}

HKU_API Indicator operator/(const Indicator& ind1, const Indicator& ind2) {
    if (!ind1.getImp() || !ind2.getImp()) {
        return Indicator();
    }

    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::DIV, ind1.getImp(), ind2.getImp());
    return p->calculate();
}

HKU_API Indicator operator==(const Indicator& ind1, const Indicator& ind2) {
    if (!ind1.getImp() || !ind2.getImp()) {
        return Indicator();
    }

    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::EQ, ind1.getImp(), ind2.getImp());
    return p->calculate();
}

HKU_API Indicator operator!=(const Indicator& ind1, const Indicator& ind2) {
    if (!ind1.getImp() || !ind2.getImp()) {
        return Indicator();
    }

    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::NE, ind1.getImp(), ind2.getImp());
    return p->calculate();
}

HKU_API Indicator operator>(const Indicator& ind1, const Indicator& ind2) {
    if (!ind1.getImp() || !ind2.getImp()) {
        return Indicator();
    }

    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::GT, ind1.getImp(), ind2.getImp());
    return p->calculate();
}

HKU_API Indicator operator<(const Indicator& ind1, const Indicator& ind2) {
    if (!ind1.getImp() || !ind2.getImp()) {
        return Indicator();
    }

    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::LT, ind1.getImp(), ind2.getImp());
    return p->calculate();
}

HKU_API Indicator operator>=(const Indicator& ind1, const Indicator& ind2) {
    if (!ind1.getImp() || !ind2.getImp()) {
        return Indicator();
    }

    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::GE, ind1.getImp(), ind2.getImp());
    return p->calculate();
}

HKU_API Indicator operator<=(const Indicator& ind1, const Indicator& ind2) {
    if (!ind1.getImp() || !ind2.getImp()) {
        return Indicator();
    }

    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::LE, ind1.getImp(), ind2.getImp());
    return p->calculate();
}

HKU_API Indicator operator&(const Indicator& ind1, const Indicator& ind2) {
    if (!ind1.getImp() || !ind2.getImp()) {
        return Indicator();
    }

    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::AND, ind1.getImp(), ind2.getImp());
    return p->calculate();
}

HKU_API Indicator operator|(const Indicator& ind1, const Indicator& ind2) {
    if (!ind1.getImp() || !ind2.getImp()) {
        return Indicator();
    }

    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::OR, ind1.getImp(), ind2.getImp());
    return p->calculate();
}


HKU_API Indicator operator+(const Indicator& ind, price_t val) {
    return ind + CVAL(ind, val);
}

HKU_API Indicator operator+(price_t val, const Indicator& ind) {
    return CVAL(ind, val) + ind;
}

HKU_API Indicator operator-(const Indicator& ind, price_t val) {
    return ind - CVAL(ind, val);
}

HKU_API Indicator operator-(price_t val, const Indicator& ind) {
    return CVAL(ind, val) - ind;
}

HKU_API Indicator operator*(const Indicator& ind, price_t val) {
    return ind * CVAL(ind, val);
}

HKU_API Indicator operator*(price_t val, const Indicator& ind) {
    return CVAL(ind, val) * ind;
}

HKU_API Indicator operator/(const Indicator& ind, price_t val) {
    return ind / CVAL(ind, val);
}

HKU_API Indicator operator/(price_t val, const Indicator& ind) {
    return CVAL(ind, val) / ind;
}

HKU_API Indicator operator==(const Indicator& ind, price_t val) {
    return ind == CVAL(ind, val);
}

HKU_API Indicator operator==(price_t val, const Indicator& ind) {
    return CVAL(ind, val) == ind;
}

HKU_API Indicator operator!=(const Indicator& ind, price_t val) {
    return ind != CVAL(ind, val);
}

HKU_API Indicator operator!=(price_t val, const Indicator& ind) {
    return CVAL(ind, val) != ind;
}

HKU_API Indicator operator>(const Indicator& ind, price_t val) {
    return ind > CVAL(ind, val);
}

HKU_API Indicator operator>(price_t val, const Indicator& ind) {
    return CVAL(ind, val) > ind;
}

HKU_API Indicator operator<(const Indicator& ind, price_t val) {
    return ind < CVAL(ind, val);
}

HKU_API Indicator operator<(price_t val, const Indicator& ind) {
    return CVAL(ind, val) < ind;
}

HKU_API Indicator operator>=(const Indicator& ind, price_t val) {
    return ind >= CVAL(ind, val);
}

HKU_API Indicator operator>=(price_t val, const Indicator& ind) {
    return CVAL(ind, val) >= ind;
}

HKU_API Indicator operator<=(const Indicator& ind, price_t val) {
    return ind <= CVAL(ind, val);
}

HKU_API Indicator operator<=(price_t val, const Indicator& ind) {
    return CVAL(ind, val) <= ind;
}

HKU_API Indicator operator&(const Indicator& ind, price_t val) {
    return ind & CVAL(ind, val);
}

HKU_API Indicator operator&(price_t val, const Indicator& ind) {
    return CVAL(ind, val) & ind;
}

HKU_API Indicator operator|(const Indicator& ind, price_t val) {
    return ind | CVAL(ind, val);
}

HKU_API Indicator operator|(price_t val, const Indicator& ind) {
    return CVAL(ind, val) | ind;
}

Indicator HKU_API IF(const Indicator& ind1, 
        const Indicator& ind2, const Indicator& ind3) {
    if (!ind1.getImp()) {
        HKU_ERROR("condition indicator is None! [IF]");
        return Indicator();
    }

    Indicator new_ind2 = ind2.getImp() ? ind2: CVAL(Null<price_t>());
    Indicator new_ind3 = ind3.getImp() ? ind3: CVAL(Null<price_t>());

    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add_if(ind1.getImp(), new_ind2.getImp(), new_ind3.getImp());
    return p->calculate();
}
    

} /* namespace hku */
