/*
 * IRocr100.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */

#include "IRocr100.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IRocr100)
#endif

namespace hku {

IRocr100::IRocr100() : IndicatorImp("ROCR100", 1) {
    setParam<int>("n", 10);
}

IRocr100::~IRocr100() {}

bool IRocr100::check() {
    return getParam<int>("n") >= 0;
}

void IRocr100::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    int n = getParam<int>("n");

    m_discard = ind.discard() + n;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    if (0 == n) {
        price_t pre_price = ind[m_discard];
        if (pre_price != 0.0) {
            _set(100.0, m_discard);
            for (size_t i = m_discard + 1; i < total; i++) {
                _set(ind[i] / pre_price * 100.0, i);
            }
        } else {
            for (size_t i = m_discard; i < total; i++) {
                _set(0.0, i);
            }
        }
        return;
    }

    for (size_t i = m_discard; i < total; i++) {
        price_t pre_price = ind[i - n];
        if (pre_price != 0.0) {
            _set((ind[i] / pre_price) * 100.0, i);
        } else {
            _set(0.0, i);
        }
    }
}

void IRocr100::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    size_t start = 0;
    if (0 == step) {
        start = ind.discard();
    } else if (curPos < ind.discard() + step) {
        return;
    } else {
        start = curPos - step;
    }

    _set(ind[start] != 0.0 ? ind[curPos] / ind[start] * 100.0 : 0.0, curPos);
}

void IRocr100::_after_dyn_calculate(const Indicator& ind) {
    size_t total = ind.size();
    HKU_IF_RETURN(m_discard == total, void());

    size_t discard = m_discard;
    for (size_t i = total - 1; i > discard; i--) {
        if (std::isnan(get(i))) {
            m_discard = i + 1;
            break;
        }
    }
    if (m_discard == discard && std::isnan(get(discard))) {
        m_discard = discard + 1;
    }
}

Indicator HKU_API ROCR100(int n) {
    IndicatorImpPtr p = make_shared<IRocr100>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API ROCR100(const IndParam& n) {
    IndicatorImpPtr p = make_shared<IRocr100>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hku */
