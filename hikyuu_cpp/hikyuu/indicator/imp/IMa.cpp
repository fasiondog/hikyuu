/*
 * IMa.cpp
 *
 *  Created on: 2013-2-10
 *      Author: fasiondog
 */

#include "IMa.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IMa)
#endif

namespace hku {

IMa::IMa() : IndicatorImp("MA", 1) {
    setParam<int>("n", 22);
}

IMa::~IMa() {}

bool IMa::check() {
    return getParam<int>("n") >= 1;
}

void IMa::_calculate(const Indicator& indicator) {
    size_t total = indicator.size();
    m_discard = indicator.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    int n = getParam<int>("n");
    size_t startPos = m_discard;
    price_t sum = 0.0;
    size_t count = 1;
    size_t first_end = startPos + n >= total ? total : startPos + n;
    for (size_t i = startPos; i < first_end; ++i) {
        sum += indicator[i];
        _set(sum / count++, i);
    }

    for (size_t i = first_end; i < total; ++i) {
        sum = indicator[i] + sum - indicator[i - n];
        _set(sum / n, i);
    }
}

void IMa::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    size_t start = _get_step_start(curPos, step, ind.discard());
    price_t sum = 0.0;
    for (size_t i = start; i <= curPos; i++) {
        sum += ind[i];
    }
    _set(sum / (curPos - start + 1), curPos);
}

Indicator HKU_API MA(int n) {
    IndicatorImpPtr p = make_shared<IMa>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API MA(const IndParam& n) {
    IndicatorImpPtr p = make_shared<IMa>();
    p->setIndParam("n", n);
    return Indicator(p);
}

Indicator HKU_API MA(const Indicator& ind, int n) {
    return MA(n)(ind);
}

Indicator HKU_API MA(const Indicator& ind, const Indicator& n) {
    return MA(IndParam(n))(ind);
}

} /* namespace hku */
