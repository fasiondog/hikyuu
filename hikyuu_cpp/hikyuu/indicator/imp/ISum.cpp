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

void ISum::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") >= 0);
    }
}

void ISum::_calculate(const Indicator& ind) {
    size_t total = ind.size();
    if (0 == total || ind.discard() >= total) {
        m_discard = total;
        return;
    }

    auto const* src = ind.data();
    auto* dst = this->data();

    int n = getParam<int>("n");
    if (n <= 0) {
        m_discard = ind.discard();
        price_t sum = 0;
        for (size_t i = m_discard; i < total; i++) {
            sum += src[i];
            dst[i] = sum;
        }
        return;
    }

    m_discard = ind.discard() + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    if (n == 1) {
        memcpy(dst + m_discard, src + m_discard, (total - m_discard) * sizeof(value_t));
        return;
    }

    _increment_calculate(ind, m_discard);
    return;
}

bool ISum::supportIncrementCalculate() const {
    return getParam<int>("n") > 1;
}

size_t ISum::min_increment_start() const {
    return getParam<int>("n") - 1;
}

void ISum::_increment_calculate(const Indicator& ind, size_t start_pos) {
    size_t total = ind.size();
    auto const* src = ind.data();
    auto* dst = this->data();

    int n = getParam<int>("n");
    price_t sum = 0.0;
    for (size_t i = start_pos + 1 - n; i <= start_pos; i++) {
        sum += src[i];
    }
    dst[start_pos] = sum;

    for (size_t i = start_pos + 1; i < total; i++) {
        sum = sum - src[i - n] + src[i];
        dst[i] = sum;
    }
}

void ISum::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    size_t start = _get_step_start(curPos, step, ind.discard());
    if (curPos + 1 < ind.discard() + step) {
        return;
    }
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
