/*
 * ILowLine.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2016年4月2日
 *      Author: fasiondog
 */

#include "ILowLine.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ILowLine)
#endif

namespace hku {

ILowLine::ILowLine() : IndicatorImp("LLV", 1) {
    setParam<int>("n", 20);
}

ILowLine::~ILowLine() {}

bool ILowLine::check() {
    return getParam<int>("n") >= 0;
}

void ILowLine::_calculate(const Indicator& ind) {
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

    price_t min = ind[startPos];
    size_t pre_pos = startPos;
    for (size_t i = startPos; i < first_end; i++) {
        if (ind[i] <= min) {
            min = ind[i];
            pre_pos = i;
        }
        _set(min, i);
    }

    for (size_t i = first_end; i < total; i++) {
        size_t j = i + 1 - n;
        if (pre_pos < j) {
            pre_pos = j;
            min = ind[j];
            for (size_t j = pre_pos + 1; j <= i; j++) {
                if (ind[j] <= min) {
                    min = ind[j];
                    pre_pos = j;
                }
            }
        } else {
            if (ind[i] <= min) {
                min = ind[i];
                pre_pos = i;
            }
        }
        _set(min, i);
    }
}

Indicator HKU_API LLV(int n = 20) {
    IndicatorImpPtr p = make_shared<ILowLine>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API LLV(const Indicator& ind, int n = 20) {
    return LLV(n)(ind);
}

} /* namespace hku */
