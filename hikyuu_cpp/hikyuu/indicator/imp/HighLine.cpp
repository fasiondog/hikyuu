/*
 * HighLine.cpp
 *
 *  Created on: 2016年4月1日
 *      Author: fasiondog
 */

#include "HighLine.h"

namespace hku {

HighLine::HighLine() : IndicatorImp("HHV", 1) {
    setParam<int>("n", 20);
}

HighLine::~HighLine() {

}

bool HighLine::check() {
    int n = getParam<int>("n");
    if (n < 1) {
        HKU_ERROR("Invalid param[n] ! (n >= 1) " << m_params
                << " [HighLine::calculate]");
        return false;
    }

    return true;
}

void HighLine::_calculate(const Indicator& data) {
    size_t total = data.size();

    int n = getParam<int>("n");

    m_discard = data.discard() + n - 1;

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
    IndicatorImpPtr p = make_shared<HighLine>();
    p->setParam<int>("n", n);
    p->calculate(ind);
    return Indicator(p);
}

} /* namespace hku */
