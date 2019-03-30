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

void Indicator::setContext(const KData& k) {
    if (m_imp) m_imp->setContext(k);
}

KData Indicator::getContext() const {
    return m_imp ? m_imp->getContext() : KData();
}


Indicator& Indicator::operator=(const Indicator& indicator) {
    if (this == &indicator)
        return *this;

    if (m_imp != indicator.m_imp)
        m_imp = indicator.m_imp;

    return *this;
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

Indicator HKU_API WEAVE(const Indicator& ind1, const Indicator& ind2) {
    if (!ind1.getImp() || !ind2.getImp()) {
        HKU_ERROR("ind1 or ind2 is Null Indicator! [WEAVE]");
        return Indicator();
    }
    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add(IndicatorImp::WEAVE, ind1.getImp(), ind2.getImp());
    return p->calculate();
}

Indicator HKU_API IF(const Indicator& ind1, 
        const Indicator& ind2, const Indicator& ind3) {
    if (!ind1.getImp() || !ind2.getImp() || !ind3.getImp()) {
        HKU_ERROR("Exists null indicator! [IF]");
        return Indicator();
    }

    IndicatorImpPtr p = make_shared<IndicatorImp>();
    p->add_if(ind1.getImp(), ind2.getImp(), ind3.getImp());
    return p->calculate();
}
    

} /* namespace hku */
