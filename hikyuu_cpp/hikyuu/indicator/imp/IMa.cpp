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
    auto const* src = indicator.data();
    auto* dst = this->data();

    int n = getParam<int>("n");
    if (n <= 0) {
        m_discard = indicator.discard();
        if (m_discard >= total) {
            m_discard = total;
            return;
        }

        price_t sum = 0.0;
        for (size_t i = m_discard; i < total; i++) {
            if (!std::isnan(src[i])) {
                sum += src[i];
                dst[i] = sum / (i - m_discard + 1);
            }
        }
        return;
    }

    m_discard = indicator.discard() + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    if (n == 1) {
        memcpy(dst + m_discard, src + m_discard, (total - m_discard) * sizeof(value_t));
        return;
    }

    size_t startPos = indicator.discard();
    price_t sum = 0.0;
    for (size_t i = startPos; i <= m_discard; ++i) {
        if (!std::isnan(src[i])) {
            sum += src[i];
        }
    }

    dst[m_discard] = sum / n;

    for (size_t i = m_discard + 1; i < total; ++i) {
        if (!std::isnan(src[i]) && !std::isnan(src[i - n])) {
            sum = src[i] + sum - src[i - n];
            dst[i] = sum / n;
        }
    }
}

bool IMa::supportIncrementCalculate() const {
    int n = getParam<int>("n");
    return n > 1;
}

size_t IMa::min_increment_start() const {
    int n = getParam<int>("n");
    return n;
}

void IMa::_increment_calculate(const Indicator& indicator, size_t startPos) {
    size_t total = indicator.size();
    auto const* src = indicator.data();
    auto* dst = this->data();

    int n = getParam<int>("n");
    HKU_ASSERT(startPos + 1 >= n);
    size_t start = startPos + 1 - n;
    value_t sum = 0.0;
    for (size_t i = start; i <= startPos; ++i) {
        if (!std::isnan(src[i])) {
            sum += src[i];
        }
    }

    dst[startPos] = sum / n;

    for (size_t i = startPos + 1; i < total; ++i) {
        if (!std::isnan(src[i]) && !std::isnan(src[i - n])) {
            sum = src[i] + sum - src[i - n];
            dst[i] = sum / n;
        }
    }
}

void IMa::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    size_t start = _get_step_start(curPos, step, ind.discard());
    if (curPos + 1 < ind.discard() + step) {
        return;
    }
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
