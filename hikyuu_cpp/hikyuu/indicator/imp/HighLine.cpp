/*
 * HighLine.cpp
 *
 *  Created on: 2016年4月1日
 *      Author: fasiondog
 */

#include "HighLine.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::HighLine)
#endif


namespace hku {

HighLine::HighLine() : IndicatorImp("HHV", 1) {
    setParam<int>("n", 20);
}

HighLine::~HighLine() {

}

bool HighLine::check() {
    int n = getParam<int>("n");
    if (n < 0) {
        HKU_ERROR("Invalid param[n] ! (n >= 0) " << m_params
                << " [HighLine::calculate]");
        return false;
    }

    return true;
}

void HighLine::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    if (0 == total) {
        m_discard = 0;
        return;
    }

    if (ind.discard() >= total) {
        m_discard = total;
        return;
    }

    int n = getParam<int>("n");
    if (0 == n) {
        n = total - ind.discard();
        m_discard = ind.discard();
    } else {
        m_discard = ind.discard() + n - 1;
        if (n + m_discard > total) {
            n = total - m_discard;
        }
    }

    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    if (1 == total) {
        if (0 == m_discard) {
            _set(ind[0], 0);
        }
        return;
    }

    price_t max = ind[m_discard];
    size_t pre_pos = m_discard;
    size_t start_pos = m_discard + n <= total ? m_discard + n : m_discard;
    for (size_t i = m_discard; i < start_pos; i++) {
         if (ind[i] >= max) {
            max = ind[i];
            pre_pos = i;
        }
        _set(max, i);
    }

    for (size_t i = start_pos; i < total-1; i++) {
        size_t j = i + 1 - n;
        if (pre_pos < j) {
            pre_pos = j;
            max = ind[j];
        }
        if (ind[i] >= max) {
            max = ind[i];
            pre_pos = i;
        }
        _set(max, i);
    }

    start_pos = total - n;
    max = ind[start_pos];
    for (size_t i = start_pos; i < total; i++) {
        if (ind[i] >= max) {
            max = ind[i];
        }
    }
    _set(max, total-1);
}


Indicator HKU_API HHV(int n =20) {
    IndicatorImpPtr p = make_shared<HighLine>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API HHV(const Indicator& ind, int n =20) {
    return HHV(n)(ind);
}

} /* namespace hku */
