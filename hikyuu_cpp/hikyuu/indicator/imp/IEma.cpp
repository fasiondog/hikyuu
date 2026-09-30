/*
 * IEma.cpp
 *
 *  Created on: 2013-4-10
 *      Author: fasiondog
 */

#include "IEma.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IEma)
#endif

namespace hku {

IEma::IEma() : IndicatorImp("EMA", 1) {
    setParam<int>("n", 22);
}

IEma::~IEma() {}

void IEma::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") > 0);
    }
}

void IEma::_calculate(const Indicator& indicator) {
    size_t total = indicator.size();
    m_discard = indicator.discard();
    if (total <= m_discard) {
        m_discard = total;
        return;
    }

    auto const* src = indicator.data();
    auto* dst = this->data();

    int n = getParam<int>("n");
    size_t startPos = discard();
    dst[startPos] = src[startPos];

    value_t multiplier = 2.0 / (n + 1);
    for (size_t i = startPos + 1; i < total; ++i) {
        dst[i] = (src[i] - dst[i - 1]) * multiplier + dst[i - 1];
    }
}

void IEma::_increment_calculate(const Indicator& data, size_t start_pos) {
    int n = getParam<int>("n");
    value_t multiplier = 2.0 / (n + 1);

    auto const* src = data.data();
    auto* dst = this->data();

    size_t total = data.size();
    for (size_t i = start_pos + 1; i < total; ++i) {
        dst[i] = (src[i] - dst[i - 1]) * multiplier + dst[i - 1];
    }
}

void IEma::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
    // Intentional: replicate the static EMA recursion on the prefix [0, curPos] with raw scalars,
    // bitwise identical to EMA(SLICE(ind, 0, curPos + 1)) but without per-bar allocations.
    auto const* src = ind.data();
    value_t multiplier = 2.0 / (step + 1.0);
    value_t prev = src[ind.discard()];
    for (size_t i = ind.discard() + 1; i <= curPos; ++i) {
        prev = (src[i] - prev) * multiplier + prev;
    }
    _set(prev, curPos);
}

Indicator HKU_API EMA(int n) {
    IndicatorImpPtr p = make_shared<IEma>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API EMA(const IndParam& n) {
    IndicatorImpPtr p = make_shared<IEma>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hku */
