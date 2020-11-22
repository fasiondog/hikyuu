/*
 * IHighLine.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2016年4月1日
 *      Author: fasiondog
 */

#include "IHighLine.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IHighLine)
#endif

namespace hku {

IHighLine::IHighLine() : IndicatorImp("HHV", 1) {
    setParam<int>("n", 20);
}

IHighLine::~IHighLine() {}

bool IHighLine::check() {
    return getParam<int>("n") >= 0;
}

void IHighLine::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    if (0 == total) {
        m_discard = 0;
        return;
    }

    if (ind.discard() >= total) {
        m_discard = total;
        return;
    }

    m_discard = ind.discard();
    if (1 == total) {
        if (0 == m_discard) {
            _set(ind[0], 0);
        }
        return;
    }

    int n = getParam<int>("n");
    if (n <= 0) {
        n = total - m_discard;
    } else if (n > total) {
        n = total;
    }

    size_t startPos = m_discard;
    size_t first_end = startPos + n >= total ? total : startPos + n;

    price_t max = ind[startPos];
    size_t pre_pos = startPos;
    for (size_t i = startPos; i < first_end; i++) {
        if (ind[i] >= max) {
            max = ind[i];
            pre_pos = i;
        }
        _set(max, i);
    }

    for (size_t i = first_end; i < total; i++) {
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

    startPos = total - n;
    max = ind[startPos];
    for (size_t i = startPos; i < total; i++) {
        if (ind[i] >= max) {
            max = ind[i];
        }
    }
    _set(max, total - 1);
}

Indicator HKU_API HHV(int n = 20) {
    IndicatorImpPtr p = make_shared<IHighLine>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API HHV(const Indicator& ind, int n = 20) {
    return HHV(n)(ind);
}

} /* namespace hku */
