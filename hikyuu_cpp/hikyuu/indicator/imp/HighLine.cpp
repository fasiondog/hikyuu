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

void HighLine::_calculate(const Indicator& data) {
    size_t total = data.size();
    if (0 == total) {
        m_discard = 0;
        return;
    }

    int n = getParam<int>("n");
    if (0 == n) {
        m_discard = data.discard();
        if (m_discard >= total) {
            return;
        }
        
        price_t max = data[0];
        _set(max, 0);
        for (size_t i = m_discard + 1; i < total; i++) {
            if (data[i] > max) {
                max = data[i];
            }
            _set(max, i);
        }
        return;
    }

    m_discard = data.discard() + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    size_t pos = m_discard + 1 - n;
    price_t max = 0;
    for (size_t i = m_discard; i < total; ++i) {
        size_t j = i + 1 - n;
        if (pos > j) {
            j = pos;
        } else {
            max = data[j];
        }
        for (; j <= i; ++j) {
            if (data[j] > max) {
                max = data[j];
                pos = j;
            }
        }
        _set(max, i);
    }

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
