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

/*
HKU_API Indicator operator+(const Indicator& ind, price_t val) {
    if (ind.size() == 0) {
        return Indicator();
    }

    size_t result_number = ind.getResultNumber();
    size_t total = ind.size();
    size_t discard = ind.discard();
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            imp->_set(ind.get(i, r) + val, i, r);
        }
    }

    return Indicator(imp);
}


HKU_API Indicator operator+(price_t val, const Indicator& ind) {
    if (ind.size() == 0) {
        return Indicator();
    }

    size_t result_number = ind.getResultNumber();
    size_t total = ind.size();
    size_t discard = ind.discard();
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            imp->_set(ind.get(i, r) + val, i, r);
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
    size_t discard = std::max(ind1.discard(), ind2.discard());
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            imp->_set(ind1.get(i, r) - ind2.get(i, r), i, r);
        }
    }

    return Indicator(imp);
}

HKU_API Indicator operator-(const Indicator& ind, price_t val) {
    if (ind.size() == 0) {
        return Indicator();
    }

    size_t result_number = ind.getResultNumber();
    size_t total = ind.size();
    size_t discard = ind.discard();
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            imp->_set(ind.get(i, r) - val, i, r);
        }
    }

    return Indicator(imp);
}

HKU_API Indicator operator-(price_t val, const Indicator& ind) {
    if (ind.size() == 0) {
        return Indicator();
    }

    size_t result_number = ind.getResultNumber();
    size_t total = ind.size();
    size_t discard = ind.discard();
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            imp->_set(val - ind.get(i, r), i, r);
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
    size_t discard = std::max(ind1.discard(), ind2.discard());
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            imp->_set(ind1.get(i, r) * ind2.get(i, r), i, r);
        }
    }

    return Indicator(imp);
}

HKU_API Indicator operator*(const Indicator& ind, price_t val) {
    if (ind.size() == 0) {
        return Indicator();
    }

    size_t result_number = ind.getResultNumber();
    size_t total = ind.size();
    size_t discard = ind.discard();
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            imp->_set(ind.get(i, r) * val, i, r);
        }
    }

    return Indicator(imp);
}

HKU_API Indicator operator*(price_t val, const Indicator& ind) {
    if (ind.size() == 0) {
        return Indicator();
    }

    size_t result_number = ind.getResultNumber();
    size_t total = ind.size();
    size_t discard = ind.discard();
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            imp->_set(ind.get(i, r) * val, i, r);
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
    size_t discard = std::max(ind1.discard(), ind2.discard());
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
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

HKU_API Indicator operator/(const Indicator& ind, price_t val) {
    if (ind.size() == 0) {
        return Indicator();
    }

    size_t result_number = ind.getResultNumber();
    size_t total = ind.size();
    size_t discard = ind.discard();
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            if (val == 0.0) {
                imp->_set(Null<price_t>(), i, r);
            } else {
                imp->_set(ind.get(i, r) / val, i, r);
            }
        }
    }

    return Indicator(imp);
}

HKU_API Indicator operator/(price_t val, const Indicator& ind) {
    if (ind.size() == 0) {
        return Indicator();
    }

    size_t result_number = ind.getResultNumber();
    size_t total = ind.size();
    size_t discard = ind.discard();
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            if (ind.get(i, r) == 0.0) {
                imp->_set(Null<price_t>(), i, r);
            } else {
                imp->_set(val / ind.get(i, r), i, r);
            }
        }
    }

    return Indicator(imp);
}

HKU_API Indicator operator==(const Indicator& ind1, const Indicator& ind2) {
    if (ind1.size() != ind2.size() || ind1.size() == 0) {
        return Indicator();
    }

    size_t result_number = std::min(ind1.getResultNumber(), ind2.getResultNumber());
    size_t total = ind1.size();
    size_t discard = std::max(ind1.discard(), ind2.discard());
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            if (std::fabs(ind1.get(i, r) - ind2.get(i, r)) < IND_EQ_THRESHOLD) {
                imp->_set(1, i, r);
            } else {
                imp->_set(0, i, r);
            }
        }
    }

    return Indicator(imp);
}

HKU_API Indicator operator==(const Indicator& ind, price_t val) {
    if (ind.size() == 0) {
        return Indicator();
    }

    size_t result_number = ind.getResultNumber();
    size_t total = ind.size();
    size_t discard = ind.discard();
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            if (std::fabs(ind.get(i, r) - val) < IND_EQ_THRESHOLD) {
                imp->_set(1, i, r);
            } else {
                imp->_set(0, i, r);
            }
        }
    }

    return Indicator(imp);
}

HKU_API Indicator operator==(price_t val, const Indicator& ind) {
    if (ind.size() == 0) {
        return Indicator();
    }

    size_t result_number = ind.getResultNumber();
    size_t total = ind.size();
    size_t discard = ind.discard();
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            if (std::fabs(ind.get(i, r) - val) < IND_EQ_THRESHOLD) {
                imp->_set(1, i, r);
            } else {
                imp->_set(0, i, r);
            }
        }
    }

    return Indicator(imp);
}


HKU_API Indicator operator!=(const Indicator& ind1, const Indicator& ind2) {
    if (ind1.size() != ind2.size() || ind1.size() == 0) {
        return Indicator();
    }

    size_t result_number = std::min(ind1.getResultNumber(), ind2.getResultNumber());
    size_t total = ind1.size();
    size_t discard = std::max(ind1.discard(), ind2.discard());
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            if (std::fabs(ind1.get(i, r) - ind2.get(i, r)) >= IND_EQ_THRESHOLD) {
                imp->_set(1, i, r);
            } else {
                imp->_set(0, i, r);
            }
        }
    }

    return Indicator(imp);
}

HKU_API Indicator operator!=(const Indicator& ind, price_t val) {
    if (ind.size() == 0) {
        return Indicator();
    }

    size_t result_number = ind.getResultNumber();
    size_t total = ind.size();
    size_t discard = ind.discard();
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            if (std::fabs(ind.get(i, r) - val) >= IND_EQ_THRESHOLD) {
                imp->_set(1, i, r);
            } else {
                imp->_set(0, i, r);
            }
        }
    }

    return Indicator(imp);
}

HKU_API Indicator operator!=(price_t val, const Indicator& ind) {
    if (ind.size() == 0) {
        return Indicator();
    }

    size_t result_number = ind.getResultNumber();
    size_t total = ind.size();
    size_t discard = ind.discard();
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            if (std::fabs(ind.get(i, r) - val) >= IND_EQ_THRESHOLD) {
                imp->_set(1, i, r);
            } else {
                imp->_set(0, i, r);
            }
        }
    }

    return Indicator(imp);
}


HKU_API Indicator operator>(const Indicator& ind1, const Indicator& ind2) {
    if (ind1.size() != ind2.size() || ind1.size() == 0) {
        return Indicator();
    }

    size_t result_number = std::min(ind1.getResultNumber(), ind2.getResultNumber());
    size_t total = ind1.size();
    size_t discard = std::max(ind1.discard(), ind2.discard());
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            if ((ind1.get(i, r) - ind2.get(i, r)) >= IND_EQ_THRESHOLD) {
                imp->_set(1, i, r);
            } else {
                imp->_set(0, i, r);
            }
        }
    }

    return Indicator(imp);
}

HKU_API Indicator operator>(const Indicator& ind, price_t val) {
    if (ind.size() == 0) {
        return Indicator();
    }

    size_t result_number = ind.getResultNumber();
    size_t total = ind.size();
    size_t discard = ind.discard();
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            if ((ind.get(i, r) - val) >= IND_EQ_THRESHOLD) {
                imp->_set(1, i, r);
            } else {
                imp->_set(0, i, r);
            }
        }
    }

    return Indicator(imp);
}

HKU_API Indicator operator>(price_t val, const Indicator& ind) {
    if (ind.size() == 0) {
        return Indicator();
    }

    size_t result_number = ind.getResultNumber();
    size_t total = ind.size();
    size_t discard = ind.discard();
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            if ((val - ind.get(i, r)) >= IND_EQ_THRESHOLD) {
                imp->_set(1, i, r);
            } else {
                imp->_set(0, i, r);
            }
        }
    }

    return Indicator(imp);
}

HKU_API Indicator operator<(const Indicator& ind1, const Indicator& ind2) {
    if (ind1.size() != ind2.size() || ind1.size() == 0) {
        return Indicator();
    }

    size_t result_number = std::min(ind1.getResultNumber(), ind2.getResultNumber());
    size_t total = ind1.size();
    size_t discard = std::max(ind1.discard(), ind2.discard());
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            if ((ind2.get(i, r) - ind1.get(i, r)) >= IND_EQ_THRESHOLD) {
                imp->_set(1, i, r);
            } else {
                imp->_set(0, i, r);
            }
        }
    }

    return Indicator(imp);
}

HKU_API Indicator operator<(const Indicator& ind, price_t val) {
    if (ind.size() == 0) {
        return Indicator();
    }

    size_t result_number = ind.getResultNumber();
    size_t total = ind.size();
    size_t discard = ind.discard();
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            if ((val - ind.get(i, r)) >= IND_EQ_THRESHOLD) {
                imp->_set(1, i, r);
            } else {
                imp->_set(0, i, r);
            }
        }
    }

    return Indicator(imp);
}

HKU_API Indicator operator<(price_t val, const Indicator& ind) {
    if (ind.size() == 0) {
        return Indicator();
    }

    size_t result_number = ind.getResultNumber();
    size_t total = ind.size();
    size_t discard = ind.discard();
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            if ((ind.get(i, r) - val) >= IND_EQ_THRESHOLD) {
                imp->_set(1, i, r);
            } else {
                imp->_set(0, i, r);
            }
        }
    }

    return Indicator(imp);
}


HKU_API Indicator operator>=(const Indicator& ind1, const Indicator& ind2) {
    if (ind1.size() != ind2.size() || ind1.size() == 0) {
        return Indicator();
    }

    size_t result_number = std::min(ind1.getResultNumber(), ind2.getResultNumber());
    size_t total = ind1.size();
    size_t discard = std::max(ind1.discard(), ind2.discard());
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            if (ind1.get(i, r) > ind2.get(i,r) - IND_EQ_THRESHOLD) {
                imp->_set(1, i, r);
            } else {
                imp->_set(0, i, r);
            }
        }
    }

    return Indicator(imp);
}

HKU_API Indicator operator>=(const Indicator& ind, price_t val) {
    if (ind.size() == 0) {
        return Indicator();
    }

    size_t result_number = ind.getResultNumber();
    size_t total = ind.size();
    size_t discard = ind.discard();
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            if (ind.get(i, r) > val - IND_EQ_THRESHOLD) {
                imp->_set(1, i, r);
            } else {
                imp->_set(0, i, r);
            }
        }
    }

    return Indicator(imp);
}

HKU_API Indicator operator>=(price_t val, const Indicator& ind) {
    if (ind.size() == 0) {
        return Indicator();
    }

    size_t result_number = ind.getResultNumber();
    size_t total = ind.size();
    size_t discard = ind.discard();
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            if (val > ind.get(i,r) - IND_EQ_THRESHOLD) {
                imp->_set(1, i, r);
            } else {
                imp->_set(0, i, r);
            }
        }
    }

    return Indicator(imp);
}

HKU_API Indicator operator<=(const Indicator& ind1, const Indicator& ind2) {
    if (ind1.size() != ind2.size() || ind1.size() == 0) {
        return Indicator();
    }

    size_t result_number = std::min(ind1.getResultNumber(), ind2.getResultNumber());
    size_t total = ind1.size();
    size_t discard = std::max(ind1.discard(), ind2.discard());
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            if (ind1.get(i, r) < ind2.get(i,r) + IND_EQ_THRESHOLD) {
                imp->_set(1, i, r);
            } else {
                imp->_set(0, i, r);
            }
        }
    }

    return Indicator(imp);
}

HKU_API Indicator operator<=(const Indicator& ind, price_t val) {
    if (ind.size() == 0) {
        return Indicator();
    }

    size_t result_number = ind.getResultNumber();
    size_t total = ind.size();
    size_t discard = ind.discard();
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            if (ind.get(i, r) < val + IND_EQ_THRESHOLD) {
                imp->_set(1, i, r);
            } else {
                imp->_set(0, i, r);
            }
        }
    }

    return Indicator(imp);
}

HKU_API Indicator operator<=(price_t val, const Indicator& ind) {
    if (ind.size() == 0) {
        return Indicator();
    }

    size_t result_number = ind.getResultNumber();
    size_t total = ind.size();
    size_t discard = ind.discard();
    IndicatorImpPtr imp(new IndicatorImp());
    imp->_readyBuffer(total, result_number);
    imp->setDiscard(discard);
    for (size_t i = discard; i < total; ++i) {
        for (size_t r = 0; r < result_number; ++r) {
            if (val < ind.get(i, r) + IND_EQ_THRESHOLD) {
                imp->_set(1, i, r);
            } else {
                imp->_set(0, i, r);
            }
        }
    }

    return Indicator(imp);
}
*/

} /* namespace hku */
