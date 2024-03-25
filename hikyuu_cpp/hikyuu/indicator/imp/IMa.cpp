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

void IMa::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") >= 0);
    }
}

void IMa::_calculate(const Indicator& indicator) {
    size_t total = indicator.size();
    m_discard = indicator.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const* src = indicator.data();
    auto* dst = this->data();

    int n = getParam<int>("n");
    if (n <= 0) {
        price_t sum = 0.0;
        for (size_t i = m_discard; i < total; i++) {
            if (!std::isnan(src[i])) {
                sum += src[i];
                dst[i] = sum / (i - m_discard + 1);
            }
        }
        return;
    }

    size_t startPos = m_discard;
    price_t sum = 0.0;
    size_t count = 1;
    size_t first_end = startPos + n >= total ? total : startPos + n;
    for (size_t i = startPos; i < first_end; ++i) {
        if (!std::isnan(src[i])) {
            sum += src[i];
            dst[i] = sum / count++;
        }
    }

    for (size_t i = first_end; i < total; ++i) {
        if (!std::isnan(src[i]) && !std::isnan(src[i - n])) {
            sum = src[i] + sum - src[i - n];
            dst[i] = sum / n;
        }
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

} /* namespace hku */
