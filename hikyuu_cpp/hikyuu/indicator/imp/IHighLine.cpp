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

void IHighLine::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") >= 0);
    }
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

    auto const* src = ind.data();
    auto* dst = this->data();

    price_t max = src[startPos];
    size_t pre_pos = startPos;
    for (size_t i = startPos; i < first_end; i++) {
        if (src[i] >= max) {
            max = src[i];
            pre_pos = i;
        }
        dst[i] = max;
    }

    for (size_t i = first_end; i < total; i++) {
        size_t j = i + 1 - n;
        if (pre_pos < j) {
            pre_pos = j;
            max = src[j];
            for (size_t k = pre_pos + 1; k <= i; k++) {
                if (src[k] >= max) {
                    max = src[k];
                    pre_pos = k;
                }
            }
        } else {
            if (src[i] >= max) {
                max = src[i];
                pre_pos = i;
            }
        }
        dst[i] = max;
    }
}

void IHighLine::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    size_t start = _get_step_start(curPos, step, ind.discard());
    price_t max_val = ind[start];
    for (size_t i = start + 1; i <= curPos; i++) {
        if (ind[i] > max_val) {
            max_val = ind[i];
        }
    }
    _set(max_val, curPos);
}

Indicator HKU_API HHV(int n = 20) {
    IndicatorImpPtr p = make_shared<IHighLine>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API HHV(const IndParam& n) {
    IndicatorImpPtr p = make_shared<IHighLine>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hku */
