/*
 * ISum.cpp
 *
 *  Created on: 2019-4-1
 *      Author: fasiondog
 */

#include "ISum.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ISum)
#endif

namespace hku {

ISum::ISum() : IndicatorImp("SUM", 1) {
    setParam<int>("n", 20);
}

ISum::~ISum() {}

bool ISum::check() {
    return getParam<int>("n") >= 0;
}

void ISum::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    if (0 == total || ind.discard() >= total) {
        m_discard = total;
        return;
    }

    int n = getParam<int>("n");
    if (n <= 0) {
        m_discard = ind.discard();
        price_t sum = 0;
        for (size_t i = m_discard; i < total; i++) {
            sum += ind[i];
            _set(sum, i);
        }
        return;
    }

    m_discard = ind.discard();
    price_t sum = 0.0;
    for (size_t i = m_discard; i < m_discard + n; i++) {
        sum += ind[i];
        _set(sum, i);
    }

    for (size_t i = m_discard + n; i < total; i++) {
        sum = sum - ind[i - n] + ind[i];
        _set(sum, i);
    }

    return;
}

void ISum::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    size_t start = _get_step_start(curPos, step, ind.discard());
    price_t sum = 0.0;
    for (size_t i = start; i <= curPos; i++) {
        sum += ind[i];
    }
    _set(sum, curPos);
}

Indicator HKU_API SUM(int n) {
    IndicatorImpPtr p = make_shared<ISum>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API SUM(const IndParam& n) {
    IndicatorImpPtr p = make_shared<ISum>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hku */
